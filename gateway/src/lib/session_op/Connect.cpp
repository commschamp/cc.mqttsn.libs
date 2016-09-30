//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
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
    typedef ConnectMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& keepAliveField = std::get<MsgType::FieldIdx_duration>(fields);
    auto& clientIdField = std::get<MsgType::FieldIdx_clientId>(fields);

    if ((state().m_connStatus != ConnectionStatus::Disconnected) &&
        (clientIdField.value() != state().m_clientId)) {
        assert(!state().m_clientId.empty());
        DisconnectMsg_SN respMsg;
        auto& respFields = respMsg.fields();
        auto& durationField = std::get<decltype(respMsg)::FieldIdx_duration>(respFields);
        durationField.setMode(comms::field::OptionalMode::Missing);
        sendToClient(respMsg);
        state().m_connStatus = ConnectionStatus::Disconnected;
        termRequest();
        return;
    }

    assert(state().m_clientId.empty() || state().m_clientId == clientIdField.value());
    if (m_internalState.m_hasClientId) {
        m_internalState = State();
    }

    m_internalState.m_hasClientId = true;

    m_clientId = clientIdField.value();
    m_keepAlive = keepAliveField.value();
    m_clean = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_cleanSession);

    do {
        if (midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_will)) {
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

    doNextStep();
}

void Connect::handle(WilltopicMsg_SN& msg)
{
    if ((!m_internalState.m_hasClientId) || (m_internalState.m_hasWillMsg)) {
        return;
    }

    m_internalState.m_hasWillTopic = true;
    m_internalState.m_attempt = 0;

    typedef WilltopicMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& willTopicField = std::get<MsgType::FieldIdx_willTopic>(fields);

    m_will.m_topic = willTopicField.value();
    m_will.m_qos = translateQos(qosField.value());
    m_will.m_retain = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_retain);
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

    typedef WillmsgMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& willMsgField = std::get<MsgType::FieldIdx_willMsg>(fields);

    m_will.m_msg = willMsgField.value();
    doNextStep();
}

void Connect::handle(ConnackMsg& msg)
{
    if (!m_internalState.m_hasWillMsg) {
        return;
    }

    typedef ConnackMsg MsgType;
    auto& fields = msg.fields();
    auto& responseField = std::get<MsgType::FieldIdx_Response>(fields);
    processAck(responseField.value());
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
        processAck(mqtt::message::ConnackResponseCode::ServerUnavailable);
        termRequest();
        return;
    }

    if (m_internalState.m_hasWillMsg) {
        assert(m_internalState.m_hasWillTopic);
        assert(m_internalState.m_hasWillMsg);
        assert(m_internalState.m_hasClientId);

        auto& st = state();
        do {

            if ((st.m_clientId != m_clientId) ||
                (st.m_keepAlive != m_keepAlive)) {
                break;
            }

            if (st.m_will.m_topic != m_will.m_topic) {
                break;
            }

            if ((!st.m_will.m_topic.empty()) &&
                (!m_will.m_topic.empty()) &&
                (st.m_will != m_will)) {
                break;
            }

            if (st.m_connStatus == ConnectionStatus::Asleep) {
                sendToBroker(PingreqMsg());
            }

            // TODO: if clean session and have subscriptions reconnect
            processAck(mqtt::message::ConnackResponseCode::Accepted);
            return;
        } while (false);

        if (st.m_connStatus == ConnectionStatus::Connected) {
            sendToBroker(DisconnectMsg());
            m_internalState.m_waitingForReconnect = true;
            nextTickReq(state().m_retryPeriod);
            brokerReconnectRequest();
            return;
        }

        assert(st.m_connStatus == ConnectionStatus::Disconnected);
        forwardConnectionReq();
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
    auto& fields = msg.fields();
    auto& flagsField = std::get<decltype(msg)::FieldIdx_Flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& lowFlagsField = std::get<mqtt::message::ConnectFlagsMemberIdx_FlagsLow>(flagsMembers);
    auto& highFlagsField = std::get<mqtt::message::ConnectFlagsMemberIdx_FlagsHigh>(flagsMembers);
    auto& keepAliveField = std::get<decltype(msg)::FieldIdx_KeepAlive>(fields);
    auto& clientIdField = std::get<decltype(msg)::FieldIdx_ClientId>(fields);

    clientIdField.value() = m_clientId;
    keepAliveField.value() = m_keepAlive;
    lowFlagsField.setBitValue(mqtt::message::ConnectFlagsLowBitIdx_CleanSession, m_clean);

    if (!m_will.m_topic.empty()) {
        auto& willTopicField = std::get<decltype(msg)::FieldIdx_WillTopic>(fields);
        auto& willMsgField = std::get<decltype(msg)::FieldIdx_WillMessage>(fields);
        auto& willQosField = std::get<mqtt::message::ConnectFlagsMemberIdx_WillQos>(flagsMembers);

        lowFlagsField.setBitValue(mqtt::message::ConnectFlagsLowBitIdx_WillFlag, true);
        willTopicField.field().value() = m_will.m_topic;
        willMsgField.field().value() = m_will.m_msg;
        willQosField.value() = translateQosForBroker(m_will.m_qos);
        highFlagsField.setBitValue(mqtt::message::ConnectFlagsHighBitIdx_WillRetain, m_will.m_retain);
    }

    if (!state().m_username.empty()) {
        auto& usernameField = std::get<decltype(msg)::FieldIdx_UserName>(fields);
        usernameField.field().value() = state().m_username;
        highFlagsField.setBitValue(mqtt::message::ConnectFlagsHighBitIdx_UserNameFlag, true);

        if (!state().m_password.empty()) {
            auto& passwordField = std::get<decltype(msg)::FieldIdx_Password>(fields);
            passwordField.field().value() = state().m_password;
            highFlagsField.setBitValue(mqtt::message::ConnectFlagsHighBitIdx_PasswordFlag, true);
        }
    }

    msg.refresh();
    sendToBroker(msg);
}

void Connect::processAck(mqtt::message::ConnackResponseCode respCode)
{
    static const mqttsn::protocol::field::ReturnCodeVal RetCodeMap[] = {
        /* Accepted */ mqttsn::protocol::field::ReturnCodeVal_Accepted,
        /* WrongProtocolVersion */ mqttsn::protocol::field::ReturnCodeVal_NotSupported,
        /* IdentifierRejected */ mqttsn::protocol::field::ReturnCodeVal_NotSupported,
        /* ServerUnavailable */ mqttsn::protocol::field::ReturnCodeVal_Conjestion,
        /* BadUsernameOrPassword */ mqttsn::protocol::field::ReturnCodeVal_NotSupported,
        /* NotAuthorized */ mqttsn::protocol::field::ReturnCodeVal_NotSupported
    };

    static const std::size_t RetCodeMapSize =
                        std::extent<decltype(RetCodeMap)>::value;

    static_assert(RetCodeMapSize == (std::size_t)mqtt::message::ConnackResponseCode::NumOfValues,
        "Incorrect map");

    auto retCode = mqttsn::protocol::field::ReturnCodeVal_NotSupported;
    if (static_cast<std::size_t>(respCode) < RetCodeMapSize) {
        retCode = RetCodeMap[static_cast<std::size_t>(respCode)];
    }

    ConnackMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& respRetCodeField = std::get<decltype(respMsg)::FieldIdx_returnCode>(respFields);
    respRetCodeField.value() = retCode;
    sendToClient(respMsg);



    // TODO: if clean session delete registrations and pending pubs.
    // TODO: if pending pubs, initiate republish

    if (retCode != mqttsn::protocol::field::ReturnCodeVal_Accepted) {
        clearConnectionInfo();
        clearInternalState();
        return;
    }

    auto& sessionState = state();
    sessionState.m_clientId = m_clientId;
    sessionState.m_connStatus = ConnectionStatus::Connected;
    sessionState.m_keepAlive = m_keepAlive;
    sessionState.m_will = m_will;
    clearInternalState();
}

void Connect::clearConnectionInfo()
{
    auto& sessionState = state();
    sessionState.m_clientId.clear();
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
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



