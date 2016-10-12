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

#include "Forward.h"

#include <cassert>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

Forward::Forward(SessionState& sessionState)
  : Base(sessionState)
{
}

Forward::~Forward() = default;

void Forward::handle(PublishMsg_SN& msg)
{
    typedef PublishMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
    auto& topicIdField = std::get<MsgType::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    auto& dataField = std::get<MsgType::FieldIdx_data>(fields);

    auto& st = state();
    auto& topic = st.m_regMgr.mapTopicId(topicIdField.value());
    if (topic.empty()) {
        PubackMsg_SN respMsg;
        auto& respFields = respMsg.fields();
        auto& respTopicIdField = std::get<decltype(respMsg)::FieldIdx_topicId>(respFields);
        auto& respMsgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);
        auto& respRetCodeField = std::get<decltype(respMsg)::FieldIdx_returnCode>(respFields);

        respTopicIdField.value() = topicIdField.value();
        respMsgIdField.value() = msgIdField.value();
        respRetCodeField.value() = mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId;
        sendToClient(respMsg);
        return;
    }

    bool retain = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_retain);
    bool dup = dupFlagsField.getBitValue(mqttsn::protocol::field::DupFlagsBits_dup);
    m_lastPubTopicId = topicIdField.value();

    PublishMsg fwdMsg;
    auto& fwdFields =fwdMsg.fields();
    auto& fwdFlagsField = std::get<decltype(fwdMsg)::FieldIdx_PublishFlags>(fwdFields);
    auto& fwdFlagsMembers = fwdFlagsField.value();
    auto& fwdRetainFlagsField = std::get<mqtt::message::PublishActualFlagIdx_Retain>(fwdFlagsMembers);
    auto& fwdQosField = std::get<mqtt::message::PublishActualFlagIdx_QoS>(fwdFlagsMembers);
    auto& fwdDupFlagsField = std::get<mqtt::message::PublishActualFlagIdx_Dup>(fwdFlagsMembers);
    auto& fwdTopicField = std::get<decltype(fwdMsg)::FieldIdx_Topic>(fwdFields);
    auto& fwdPacketIdField = std::get<decltype(fwdMsg)::FieldIdx_PacketId>(fwdFields);
    auto& fwdPayloadField = std::get<decltype(fwdMsg)::FieldIdx_Payload>(fwdFields);

    fwdRetainFlagsField.setBitValue(0, retain);
    fwdQosField.value() = translateQosForBroker(translateQos(qosField.value()));
    fwdDupFlagsField.setBitValue(0, dup);
    fwdTopicField.value() = topic;
    fwdPacketIdField.field().value() = msgIdField.value();
    fwdPayloadField.value() = dataField.value();
    fwdMsg.refresh();
    sendToBroker(fwdMsg);
}

void Forward::handle(PubackMsg& msg)
{
    typedef PubackMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    PubackMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& respTopicIdField = std::get<decltype(respMsg)::FieldIdx_topicId>(respFields);
    auto& respMsgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);
    auto& respRetCodeField = std::get<decltype(respMsg)::FieldIdx_returnCode>(respFields);

    respTopicIdField.value() = m_lastPubTopicId;
    respMsgIdField.value() = packetIdField.value();
    respRetCodeField.value() = mqttsn::protocol::field::ReturnCodeVal_Accepted;
    sendToClient(respMsg);
}

void Forward::handle(PubrecMsg& msg)
{
    typedef PubrecMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    PubrecMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& msgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);

    msgIdField.value() = packetIdField.value();
    sendToClient(respMsg);
}

void Forward::handle(PubcompMsg& msg)
{
    typedef PubcompMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    PubcompMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& msgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);

    msgIdField.value() = packetIdField.value();
    sendToClient(respMsg);
}


}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



