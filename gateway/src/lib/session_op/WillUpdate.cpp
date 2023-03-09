//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "WillUpdate.h"

#include <cassert>
#include <algorithm>

namespace cc_mqttsn_gateway
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
        sendTopicResp(ReturnCodeVal::NotSupported);
        return;
    }

    if (m_op == Op::MsgUpd) {
        sendTopicResp(ReturnCodeVal::Congestion);
        sendToBroker(PingreqMsg());
        return;
    }

    if (m_op == Op::TopicUpd) {
        sendToBroker(PingreqMsg());
        return;
    }

    auto qos = translateQos(msg.field_flags().field().field_qos().value());
    bool retain = msg.field_flags().field().field_mid().getBitValue_Retain();

    auto& st = state();
    auto& willTopic = msg.field_willTopic().value();
    if ((st.m_will.m_topic == willTopic) &&
        (st.m_will.m_qos == qos) &&
        (st.m_will.m_retain == retain)) {
        sendTopicResp(ReturnCodeVal::Accepted);
        sendToBroker(PingreqMsg());
        return;
    }

    m_will.m_topic.assign(willTopic.begin(), willTopic.end());
    m_will.m_msg = st.m_will.m_msg;
    m_will.m_qos = qos;
    m_will.m_retain = retain;
    startOp(Op::TopicUpd);
}

void WillUpdate::handle(WillmsgupdMsg_SN& msg)
{
    if (state().m_connStatus != ConnectionStatus::Connected) {
        sendMsgResp(ReturnCodeVal::NotSupported);
        return;
    }

    if (m_op == Op::TopicUpd) {
        sendMsgResp(ReturnCodeVal::Congestion);
        sendToBroker(PingreqMsg());
        return;
    }

    if (m_op == Op::MsgUpd) {
        sendToBroker(PingreqMsg());
        return;
    }

    auto& st = state();
    auto& willData = msg.field_willMsg().value();
    if ((willData.size() == st.m_will.m_msg.size()) &&
        (std::equal(willData.begin(), willData.end(), st.m_will.m_msg.begin()))) {
        sendMsgResp(ReturnCodeVal::Accepted);
        sendToBroker(PingreqMsg());
        return;
    }

    m_will.m_topic = st.m_will.m_topic;
    m_will.m_msg.assign(willData.begin(), willData.end());
    m_will.m_qos = st.m_will.m_qos;
    m_will.m_retain = st.m_will.m_retain;
    startOp(Op::MsgUpd);
}

void WillUpdate::handle(ConnackMsg& msg)
{
    if (m_op == Op::None) {
        return;
    }

    if ((msg.field_returnCode().value() != ConnackMsg::Field_returnCode::ValueType::Accepted) ||
        (!msg.field_flags().getBitValue_sp())) {
        sendFailureAndTerm();
        return;
    }

    state().m_will = m_will;
    sendResp(ReturnCodeVal::Accepted);
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

void WillUpdate::sendTopicResp(ReturnCodeVal rc)
{
    WilltopicrespMsg_SN msg;
    msg.field_returnCode().value() = rc;
    sendToClient(msg);
}

void WillUpdate::sendMsgResp(ReturnCodeVal rc)
{
    WillmsgrespMsg_SN msg;
    msg.field_returnCode().value() = rc;
    sendToClient(msg);
}

void WillUpdate::sendResp(ReturnCodeVal rc)
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

    auto& st = state();
    msg.field_clientId().value() = st.m_clientId;
    msg.field_keepAlive().value() = st.m_keepAlive;

    auto& flagsField = msg.field_flags();


    if (!m_will.m_topic.empty()) {
        flagsField.field_low().setBitValue_willFlag(true);
        msg.field_willTopic().field().value() = m_will.m_topic;
        msg.field_willMessage().field().value() = m_will.m_msg;
        flagsField.field_willQos().value() = translateQosForBroker(m_will.m_qos);
        flagsField.field_high().setBitValue_willRetain(m_will.m_retain);
    }

    if (!st.m_username.empty()) {
        msg.field_userName().field().value() = state().m_username;
        flagsField.field_high().setBitValue_userNameFlag(true);

        if (!state().m_password.empty()) {
            msg.field_password().field().value() = state().m_password;
            flagsField.field_high().setBitValue_passwordFlag(true);
        }
    }

    msg.doRefresh();
    sendToBroker(msg);
}

void WillUpdate::sendFailureAndTerm()
{
    sendResp(ReturnCodeVal::NotSupported);
    cancelOp();
    sendDisconnectToClient();
    termRequest();
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway



