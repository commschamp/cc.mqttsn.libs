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

#include "WillUpdate.h"

#include <cassert>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

WillUpdate::WillUpdate(SessionState& sessionState)
  : Base(sessionState)
{
}

WillUpdate::~WillUpdate() = default;

void WillUpdate::tickImpl()
{
    if (m_op == Op::None) {
        return;
    }

    sendFailureAndTerm();
}

void WillUpdate::brokerConnectionUpdatedImpl()
{
    if (m_op == Op::None) {
        return;
    }

    auto& st = state();
    if ((st.m_brokerConnected) &&
        (m_reconnectRequested) &&
        (!m_brokerConnectSent)) {
        cancelTick();
        doNextStage();
        return;
    }
}

void WillUpdate::handle(ConnectMsg_SN& msg)
{
    static_cast<void>(msg);
    if (m_op != Op::None) {
        cancelOp();
    }
}

void WillUpdate::handle(DisconnectMsg_SN& msg)
{
    static_cast<void>(msg);
    if (m_op != Op::None) {
        cancelOp();
    }
}

void WillUpdate::handle(WilltopicupdMsg_SN& msg)
{
    if (state().m_connStatus != ConnectionStatus::Connected) {
        sendTopicResp(mqttsn::protocol::field::ReturnCodeVal_NotSupported);
        return;
    }

    if (m_op == Op::MsgUpd) {
        sendTopicResp(mqttsn::protocol::field::ReturnCodeVal_Congestion);
        sendToBroker(PingreqMsg());
        return;
    }

    if (m_op == Op::TopicUpd) {
        sendToBroker(PingreqMsg());
        return;
    }

    typedef WilltopicupdMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.field().value();
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& topicField = std::get<MsgType::FieldIdx_willTopic>(fields);

    auto qos = translateQos(qosField.value());
    bool retain =  midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_retain);

    auto& st = state();
    if ((st.m_will.m_topic == topicField.value()) &&
        (st.m_will.m_qos == qos) &&
        (st.m_will.m_retain == retain)) {
        sendTopicResp(mqttsn::protocol::field::ReturnCodeVal_Accepted);
        sendToBroker(PingreqMsg());
        return;
    }


    m_will.m_topic = topicField.value();
    m_will.m_msg = st.m_will.m_msg;
    m_will.m_qos = qos;
    m_will.m_retain = retain;
    startOp(Op::TopicUpd);
}

void WillUpdate::handle(WillmsgupdMsg_SN& msg)
{
    if (state().m_connStatus != ConnectionStatus::Connected) {
        sendMsgResp(mqttsn::protocol::field::ReturnCodeVal_NotSupported);
        return;
    }

    if (m_op == Op::TopicUpd) {
        sendMsgResp(mqttsn::protocol::field::ReturnCodeVal_Congestion);
        sendToBroker(PingreqMsg());
        return;
    }

    if (m_op == Op::MsgUpd) {
        sendToBroker(PingreqMsg());
        return;
    }

    typedef WillmsgupdMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& msgField = std::get<MsgType::FieldIdx_willMsg>(fields);

    auto& st = state();
    if (st.m_will.m_msg == msgField.value()) {
        sendMsgResp(mqttsn::protocol::field::ReturnCodeVal_Accepted);
        sendToBroker(PingreqMsg());
        return;
    }

    m_will.m_topic = st.m_will.m_topic;
    m_will.m_msg = msgField.value();
    m_will.m_qos = st.m_will.m_qos;
    m_will.m_retain = st.m_will.m_retain;
    startOp(Op::MsgUpd);
}

void WillUpdate::handle(ConnackMsg& msg)
{
    if (m_op == Op::None) {
        return;
    }

    auto fields = msg.fieldsAsStruct();
    typedef typename std::decay<decltype(fields.response)>::type ResponseFieldType;
    if ((fields.response.value() != ResponseFieldType::ValueType::Accepted) ||
        (!fields.flags.getBitValue(0))) {
        sendFailureAndTerm();
        return;
    }

    state().m_will = m_will;
    sendResp(mqttsn::protocol::field::ReturnCodeVal_Accepted);
    cancelOp();
}

void WillUpdate::startOp(Op op)
{
    cancelTick();
    m_op = op;
    m_reconnectRequested = false;
    m_brokerConnectSent = false;
    doNextStage();
}

void WillUpdate::doNextStage()
{
    if (m_reconnectRequested) {
        assert(state().m_brokerConnected);
        sendConnectMsg();
        m_brokerConnectSent = true;
        nextTickReq(state().m_retryPeriod);
        return;
    }

    sendToBroker(DisconnectMsg());
    brokerReconnectRequest();
    m_reconnectRequested = true;
    nextTickReq(state().m_retryPeriod);
}

void WillUpdate::cancelOp()
{
    cancelTick();
    m_op = Op::None;
}

void WillUpdate::sendTopicResp(mqttsn::protocol::field::ReturnCodeVal rc)
{
    WilltopicrespMsg_SN msg;
    auto& fields = msg.fields();
    auto& retCodeField = std::get<decltype(msg)::FieldIdx_returnCode>(fields);

    retCodeField.value() = rc;
    sendToClient(msg);
}

void WillUpdate::sendMsgResp(mqttsn::protocol::field::ReturnCodeVal rc)
{
    WillmsgrespMsg_SN msg;
    auto& fields = msg.fields();
    auto& retCodeField = std::get<decltype(msg)::FieldIdx_returnCode>(fields);

    retCodeField.value() = rc;
    sendToClient(msg);
}

void WillUpdate::sendResp(mqttsn::protocol::field::ReturnCodeVal rc)
{
    assert(m_op != Op::None);
    if (m_op == Op::TopicUpd) {
        sendTopicResp(rc);
        return;
    }

    if (m_op == Op::MsgUpd) {
        sendMsgResp(rc);
        return;
    }
}

void WillUpdate::sendConnectMsg()
{
    ConnectMsg msg;
    auto fields = msg.fieldsAsStruct();
    auto flags = fields.flags.fieldsAsStruct();

    auto& st = state();
    fields.clientId.value() = st.m_clientId;
    fields.keepAlive.value() = st.m_keepAlive;

    typedef typename std::decay<decltype(flags.flagsLow)>::type FlagsLowFieldType;
    typedef typename std::decay<decltype(flags.flagsHigh)>::type FlagsHighFieldType;

    if (!m_will.m_topic.empty()) {
        flags.flagsLow.setBitValue(FlagsLowFieldType::BitIdx_willFlag, true);
        fields.willTopic.field().value() = m_will.m_topic;
        fields.willMessage.field().value() = m_will.m_msg;
        flags.willQos.value() = translateQosForBroker(m_will.m_qos);
        flags.flagsHigh.setBitValue(FlagsHighFieldType::BitIdx_willRetain, m_will.m_retain);
    }

    if (!st.m_username.empty()) {
        fields.userName.field().value() = state().m_username;
        flags.flagsHigh.setBitValue(FlagsHighFieldType::BitIdx_username, true);

        if (!state().m_password.empty()) {
            fields.password.field().value() = state().m_password;
            flags.flagsHigh.setBitValue(FlagsHighFieldType::BitIdx_password, true);
        }
    }

    msg.doRefresh();
    sendToBroker(msg);
}

void WillUpdate::sendFailureAndTerm()
{
    sendResp(mqttsn::protocol::field::ReturnCodeVal_NotSupported);
    cancelOp();
    sendDisconnectToClient();
    termRequest();
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



