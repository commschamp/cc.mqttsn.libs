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

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

void Connect::tickImpl()
{
    doNextStep();
}

Connect::Type Connect::typeImpl() const
{
    return Type::Connect;
}

void Connect::handle(ConnectMsg_SN& msg)
{
    m_status = ConnectionStatus::Connecting;
    typedef ConnectMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& keepAliveField = std::get<MsgType::FieldIdx_duration>(fields);
    auto& clientIdField = std::get<MsgType::FieldIdx_clientId>(fields);

    if (m_state.m_hasClientId) {
        m_state = State();
    }

    m_state.m_hasClientId = true;

    m_info.m_clientId = clientIdField.value();
    m_info.m_keepAlive = keepAliveField.value();
    m_info.m_clean = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_cleanSession);
    m_info.m_will = WillInfo();

    if (!midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_will)) {
        m_state.m_hasWillTopic = true;
        m_state.m_hasWillMsg = true;
    }

    doNextStep();
}

void Connect::handle(WilltopicMsg_SN& msg)
{
    assert(m_status == ConnectionStatus::Connecting);
    if ((!m_state.m_hasClientId) || (m_state.m_hasWillMsg)) {
        return;
    }

    m_state.m_hasWillTopic = true;
    m_state.m_attempt = 0;

    typedef WilltopicMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& willTopicField = std::get<MsgType::FieldIdx_willTopic>(fields);

    m_info.m_will.m_topic = willTopicField.value();
    m_info.m_will.m_qos = translateQos(qosField.value());
    m_info.m_will.m_retain = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_retain);
    doNextStep();
}

void Connect::handle(WillmsgMsg_SN& msg)
{
    assert(m_status == ConnectionStatus::Connecting);
    if (!m_state.m_hasWillMsg) {
        return;
    }

    assert(m_state.m_hasClientId);

    m_state.m_hasWillMsg = true;
    m_state.m_attempt = 0;

    typedef WillmsgMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& willMsgField = std::get<MsgType::FieldIdx_willMsg>(fields);

    m_info.m_will.m_msg = willMsgField.value();
    doNextStep();
}

void Connect::handle(ConnackMsg& msg)
{
    assert(m_status == ConnectionStatus::Connecting);
    if (!m_state.m_hasWillMsg) {
        return;
    }

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

    typedef ConnackMsg MsgType;
    auto& fields = msg.fields();
    auto& responseField = std::get<MsgType::FieldIdx_Response>(fields);
    auto responseVal = responseField.value();

    auto retCode = mqttsn::protocol::field::ReturnCodeVal_NotSupported;
    if (static_cast<std::size_t>(responseVal) < RetCodeMapSize) {
        retCode = RetCodeMap[static_cast<std::size_t>(responseVal)];
    }

    ConnackMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& respRetCodeField = std::get<decltype(respMsg)::FieldIdx_returnCode>(respFields);
    respRetCodeField.value() = retCode;
    sendToClient(respMsg);

    m_clientId = m_info.m_clientId;
    m_status = ConnectionStatus::Connected;
    setComplete();
}

void Connect::doNextStep()
{
    cancelTick();

    if (m_state.m_attempt <= retryCount()) {
        m_clientId.clear();
        m_status = ConnectionStatus::Disconnected;
        return;
    }

    ++m_state.m_attempt;

    if (m_state.m_hasWillMsg) {
        assert(m_state.m_hasWillTopic);
        assert(m_state.m_hasWillMsg);
        assert(m_state.m_hasClientId);
        forwardConnectionReq();
        nextTickReq(retryPeriod());
        return;
    }

    if (m_state.m_hasWillTopic) {
        assert(m_state.m_hasClientId);
        sendToClient(WillmsgreqMsg_SN());
        nextTickReq(retryPeriod());
        return;
    }

    assert(m_state.m_hasClientId);
    sendToClient(WillmsgreqMsg_SN());
    nextTickReq(retryPeriod());
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

    clientIdField.value() = m_info.m_clientId;
    keepAliveField.value() = m_info.m_keepAlive;
    lowFlagsField.setBitValue(mqtt::message::ConnectFlagsLowBitIdx_CleanSession, m_info.m_clean);

    if (!m_info.m_will.m_topic.empty()) {
        auto& willTopicField = std::get<decltype(msg)::FieldIdx_WillTopic>(fields);
        auto& willMsgField = std::get<decltype(msg)::FieldIdx_WillMessage>(fields);
        auto& willQosField = std::get<mqtt::message::ConnectFlagsMemberIdx_WillQos>(flagsMembers);

        lowFlagsField.setBitValue(mqtt::message::ConnectFlagsLowBitIdx_WillFlag, true);
        willTopicField.field().value() = m_info.m_will.m_topic;
        willMsgField.field().value() = m_info.m_will.m_msg;
        willQosField.value() = translateQosForBroker(m_info.m_will.m_qos);
        highFlagsField.setBitValue(mqtt::message::ConnectFlagsHighBitIdx_WillRetain, m_info.m_will.m_retain);
    }

    if (!m_username.empty()) {
        auto& usernameField = std::get<decltype(msg)::FieldIdx_UserName>(fields);
        usernameField.field().value() = m_username;

        if (!m_password.empty()) {
            auto& passwordField = std::get<decltype(msg)::FieldIdx_Password>(fields);
            passwordField.field().value() = m_password;
        }
    }

    msg.refresh();
    sendToBroker(msg);
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



