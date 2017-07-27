//
// Copyright 2016 - 2017 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Connect.h"
#include <cassert>

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

Connect::Connect(SessionState& sessionState)
  : Base(sessionState)
{
}

Connect::~Connect() = default;

void Connect::tickImpl()
{
    doNextStep();
}

void Connect::brokerConnectionUpdatedImpl()
{
    if (!m_internalState.m_waitingForReconnect) {
        return;
    }

    if (!state().m_brokerConnected) {
        clearConnectionInfo();
        return;
    }

    m_internalState.m_waitingForReconnect = false;
    doNextStep();
}

void Connect::handle(ConnectMsg_SN& msg)
{
    auto& midFlagsField = msg.field_flags().field_midFlags();
    typedef typename std::decay<decltype(midFlagsField)>::type MidFlags;

    auto& st = state();
    auto reqClientId = msg.field_clientId().value().substr();
    if (reqClientId.empty()) {
        reqClientId = st.m_defaultClientId;
    }

    if ((st.m_connStatus != ConnectionStatus::Disconnected) &&
        (reqClientId != st.m_clientId)) {
        sendDisconnectToClient();
        state().m_connStatus = ConnectionStatus::Disconnected;
        termRequest();
        return;
    }


    assert(st.m_clientId.empty() || (st.m_clientId == reqClientId));
    if (m_internalState.m_hasClientId) {
        m_internalState = State();
    }

    m_internalState.m_hasClientId = true;

    m_clientId = std::move(reqClientId);
    m_keepAlive = msg.field_duration().value();
    m_clean = midFlagsField.getBitValue(MidFlags::BitIdx_cleanSession);

    do {
        if (midFlagsField.getBitValue(MidFlags::BitIdx_will)) {
            break;
        }

        m_internalState.m_hasWillTopic = true;
        m_internalState.m_hasWillMsg = true;

        if (m_clean) {
            m_will = WillInfo();
            break;
        }

        m_will = state().m_will;
    } while (false);

    if ((st.m_connStatus == ConnectionStatus::Disconnected) &&
        (!st.m_brokerConnected) &&
        (!m_internalState.m_waitingForReconnect) &&
        (!st.m_pendingClientDisconnect)) {
        m_internalState.m_waitingForReconnect = true;
        nextTickReq(st.m_retryPeriod);
        return;
    }

    doNextStep();
}

void Connect::handle(WilltopicMsg_SN& msg)
{
    if ((!m_internalState.m_hasClientId) || (m_internalState.m_hasWillMsg)) {
        return;
    }

    m_internalState.m_hasWillTopic = true;
    m_internalState.m_attempt = 0;

    auto& midFlagsField = msg.field_flags().field().field_midFlags();
    typedef typename std::decay<decltype(midFlagsField)>::type MidFlags;

    auto& topicView = msg.field_willTopic().value();
    m_will.m_topic.assign(topicView.begin(), topicView.end());
    m_will.m_qos = translateQos(msg.field_flags().field().field_qos().value());
    m_will.m_retain = midFlagsField.getBitValue(MidFlags::BitIdx_retain);

    if (m_will.m_topic.empty()) {
        m_internalState.m_hasWillMsg = true;
    }
    else if (state().m_connStatus == ConnectionStatus::Connected) {
        sendToBroker(PingreqMsg());
    }

    doNextStep();
}

void Connect::handle(WillmsgMsg_SN& msg)
{
    if (!m_internalState.m_hasWillTopic) {
        return;
    }

    assert(m_internalState.m_hasClientId);

    m_internalState.m_hasWillMsg = true;
    m_internalState.m_attempt = 0;

    auto& data = msg.field_willMsg().value();
    m_will.m_msg.assign(data.begin(), data.end());
    doNextStep();
}

void Connect::handle(PublishMsg_SN& msg)
{
    auto& st = state();
    if ((st.m_connStatus != ConnectionStatus::Disconnected) ||
        (st.m_pendingClientDisconnect) ||
        (!st.m_clientId.empty()) ||
        (!m_clientId.empty())) {
        return;
    }

    if (msg.field_flags().field_qos().value() != mqttsn::protocol::field::QosType::NoGwPublish) {
        return;
    }

    clearInternalState();
    m_clientId = st.m_defaultClientId;
    m_keepAlive = st.m_pubOnlyKeepAlive;
    m_clean = true;

    m_internalState.m_hasClientId = true;
    m_internalState.m_hasWillTopic = true;
    m_internalState.m_hasWillMsg = true;
    m_internalState.m_pubOnlyClient = true;

    if (st.m_brokerConnected) {
        doNextStep();
        return;
    }

    m_internalState.m_waitingForReconnect = true;
    nextTickReq(st.m_retryPeriod);
}

void Connect::handle(ConnackMsg& msg)
{
    if (!m_internalState.m_hasWillMsg) {
        return;
    }

    processAck(msg.field_responseCode().value());
}

void Connect::doNextStep()
{
    cancelTick();

    if (state().m_retryCount <= m_internalState.m_attempt) {
        m_clientId.clear();
        state().m_connStatus = ConnectionStatus::Disconnected;
        return;
    }

    ++m_internalState.m_attempt;

    if (m_internalState.m_waitingForReconnect) {
        processAck(mqtt::protocol::v311::field::ConnackResponseCodeVal::ServerUnavailable);
        termRequest();
        return;
    }

    if (m_internalState.m_hasWillMsg) {
        assert(m_internalState.m_hasWillTopic);
        assert(m_internalState.m_hasWillMsg);
        assert(m_internalState.m_hasClientId);

        auto& st = state();
        do {
            if ((!st.m_clientId.empty()) && (st.m_clientId != m_clientId)) {
                processAck(mqtt::protocol::v311::field::ConnackResponseCodeVal::IdentifierRejected);
                return;
            }

            if (st.m_will.m_topic != m_will.m_topic) {
                break;
            }

            if ((!st.m_will.m_topic.empty()) &&
                (!m_will.m_topic.empty()) &&
                (st.m_will != m_will)) {
                break;
            }

            if (st.m_clientId != m_clientId) {
                break;
            }

            if (st.m_pendingClientDisconnect) {
                // Emulate successful connection, Disconnect will be sent from PubSend op
                processAck(mqtt::protocol::v311::field::ConnackResponseCodeVal::Accepted);
                return;
            }

            if (st.m_keepAlive != m_keepAlive) {
                break;
            }

            if ((st.m_connStatus == ConnectionStatus::Asleep) &&
                (st.m_brokerConnected)) {
                sendToBroker(PingreqMsg());
            }

            if (m_clean) {
                st.m_regMgr.clearRegistrations();
            }

            processAck(mqtt::protocol::v311::field::ConnackResponseCodeVal::Accepted);
            return;
        } while (false);

        if (!st.m_brokerConnected) {
            processAck(mqtt::protocol::v311::field::ConnackResponseCodeVal::ServerUnavailable);
            return;
        }

        if (st.m_connStatus != ConnectionStatus::Disconnected) {
            sendToBroker(DisconnectMsg());
            m_internalState.m_waitingForReconnect = true;
            nextTickReq(state().m_retryPeriod);
            brokerReconnectRequest();
            return;
        }

        if ((m_clientId != st.m_clientId) ||
            (st.m_clientId.empty() && (!st.m_clientConnectReported))) {
            assert(m_authInfoReqCb);
            m_authInfo = m_authInfoReqCb(m_clientId);
        }
        else {
            m_authInfo = std::make_pair(st.m_username, st.m_password);
        }

        forwardConnectionReq();
        if (m_internalState.m_pubOnlyClient) {
            nextTickReq(state().m_retryPeriod);
        }
        return;
    }

    if (m_internalState.m_hasWillTopic) {
        assert(m_internalState.m_hasClientId);
        sendToClient(WillmsgreqMsg_SN());
        nextTickReq(state().m_retryPeriod);
        return;
    }

    assert(m_internalState.m_hasClientId);
    sendToClient(WilltopicreqMsg_SN());
    nextTickReq(state().m_retryPeriod);
}

void Connect::forwardConnectionReq()
{
    ConnectMsg msg;
    typedef typename std::decay<decltype(msg.field_flags().field_flagsLow())>::type LowFlagsFieldType;
    typedef typename std::decay<decltype(msg.field_flags().field_flagsHigh())>::type HighFlagsFieldType;

    msg.field_clientId().value() = m_clientId;
    msg.field_keepAlive().value() = m_keepAlive;
    msg.field_flags().field_flagsLow().setBitValue(LowFlagsFieldType::BitIdx_cleanSession, m_clean);

    if (!m_will.m_topic.empty()) {
        msg.field_flags().field_flagsLow().setBitValue(LowFlagsFieldType::BitIdx_willFlag, true);
        msg.field_willTopic().field().value() = m_will.m_topic;
        msg.field_willMessage().field().value() = m_will.m_msg;
        msg.field_flags().field_willQos().value() = translateQosForBroker(m_will.m_qos);
        msg.field_flags().field_flagsHigh().setBitValue(HighFlagsFieldType::BitIdx_willRetain, m_will.m_retain);
    }

    auto& username = m_authInfo.first;
    if (!username.empty()) {
        msg.field_userName().field().value() = username;
        msg.field_flags().field_flagsHigh().setBitValue(HighFlagsFieldType::BitIdx_username, true);

        auto& password = m_authInfo.second;
        if (!password.empty()) {
            msg.field_password().field().value() = password;
            msg.field_flags().field_flagsHigh().setBitValue(HighFlagsFieldType::BitIdx_password, true);
        }
    }

    msg.doRefresh();
    sendToBroker(msg);
}

void Connect::processAck(mqtt::protocol::v311::field::ConnackResponseCodeVal respCode)
{
    static const mqttsn::protocol::field::ReturnCodeVal RetCodeMap[] = {
        /* Accepted */ mqttsn::protocol::field::ReturnCodeVal_Accepted,
        /* WrongProtocolVersion */ mqttsn::protocol::field::ReturnCodeVal_NotSupported,
        /* IdentifierRejected */ mqttsn::protocol::field::ReturnCodeVal_NotSupported,
        /* ServerUnavailable */ mqttsn::protocol::field::ReturnCodeVal_Congestion,
        /* BadUsernameOrPassword */ mqttsn::protocol::field::ReturnCodeVal_NotSupported,
        /* NotAuthorized */ mqttsn::protocol::field::ReturnCodeVal_NotSupported
    };

    static const std::size_t RetCodeMapSize =
                        std::extent<decltype(RetCodeMap)>::value;

    static_assert(RetCodeMapSize == (std::size_t)mqtt::protocol::v311::field::ConnackResponseCodeVal::NumOfValues,
        "Incorrect map");

    auto retCode = mqttsn::protocol::field::ReturnCodeVal_NotSupported;
    if (static_cast<std::size_t>(respCode) < RetCodeMapSize) {
        retCode = RetCodeMap[static_cast<std::size_t>(respCode)];
    }

    if (!m_internalState.m_pubOnlyClient) {
        ConnackMsg_SN respMsg;
        respMsg.field_returnCode().value() = retCode;
        sendToClient(respMsg);
    }
    else {
        cancelTick();
    }

    if (retCode != mqttsn::protocol::field::ReturnCodeVal_Accepted) {
        clearConnectionInfo(true);
        clearInternalState();
        return;
    }

    auto& sessionState = state();
    if (!sessionState.m_clientConnectReported) {
        sessionState.m_clientConnectReported = true;
        assert(m_clientConnectedCb);
        m_clientConnectedCb(m_clientId);
    }
    sessionState.m_clientId = std::move(m_clientId);
    sessionState.m_connStatus = ConnectionStatus::Connected;
    sessionState.m_keepAlive = m_keepAlive;
    sessionState.m_will = m_will;
    sessionState.m_username = std::move(m_authInfo.first);
    sessionState.m_password = std::move(m_authInfo.second);
    clearInternalState();

    if (m_clean) {
        sessionState.m_regMgr.clearRegistrations();
    }
}

void Connect::clearConnectionInfo(bool clearClientId)
{
    auto& sessionState = state();
    if (clearClientId) {
        sessionState.m_clientId.clear();
        sessionState.m_username.clear();
        sessionState.m_password.clear();
    }
    sessionState.m_connStatus = ConnectionStatus::Disconnected;
    sessionState.m_keepAlive = 0;
    sessionState.m_will = WillInfo();
}

void Connect::clearInternalState()
{
    m_internalState = State();
    m_clientId.clear();
    m_will = WillInfo();
    m_keepAlive = 0;
    m_clean = false;
    m_authInfo = AuthInfo();
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



