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
    // TODO
}

Connect::Type Connect::typeImpl() const
{
    return Type::Connect;
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

    if (m_state.m_hasClientId) {
        m_state = State();
    }

    m_state.m_hasClientId = true;

    m_info.m_clientId = clientIdField.value();
    m_info.m_keepAlive = keepAliveField.value();
    m_info.m_clean = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_cleanSession);
    m_info.m_will = WillInfo();

    if (!midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_will)) {
        forwardConnectionReq();
        return;
    }

    sendToClient(WilltopicreqMsg_SN());
    // TODO: req tick
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

    if (!m_info.m_username.empty()) {
        auto& usernameField = std::get<decltype(msg)::FieldIdx_UserName>(fields);
        usernameField.field().value() = m_info.m_username;

        if (!m_info.m_password.empty()) {
            auto& passwordField = std::get<decltype(msg)::FieldIdx_Password>(fields);
            passwordField.field().value() = m_info.m_password;
        }
    }

    msg.refresh();
    sendToBroker(msg);
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



