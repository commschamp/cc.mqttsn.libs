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

#include "PubSend.h"

#include <cassert>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

PubSend::PubSend(SessionState& sessionState)
  : Base(sessionState)
{
}

PubSend::~PubSend() = default;

void PubSend::tickImpl()
{
    if (!m_currPub) {
        checkSend();
        return;
    }

    if (!m_acked) {
        doSend();
        return;
    }

    auto& st = state();
    if (st.m_retryCount <= m_attempt) {
        m_currPub.reset();
        checkSend();
        return;
    }

    ++m_attempt;
    sendPubrel();
}

void PubSend::handle(RegackMsg_SN& msg)
{
    typedef RegackMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& topicIdField = std::get<MsgType::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    auto& retCodeField = std::get<MsgType::FieldIdx_returnCode>(fields);

    if ((!m_currPub) ||
        (topicIdField.value() != m_currTopicInfo.m_topicId) ||
        (msgIdField.value() != m_currMsgId)) {
        return;
    }

    if (retCodeField.value() != mqttsn::protocol::field::ReturnCodeVal_Accepted) {
        m_currPub.reset();
        checkSend();
        return;
    }

    m_attempt = 0;
    m_registered = true;
    m_currMsgId = allocMsgId();
    doSend();
}

void PubSend::handle(PubackMsg_SN& msg)
{
    typedef PubackMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& topicIdField = std::get<MsgType::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    auto& retCodeField = std::get<MsgType::FieldIdx_returnCode>(fields);

    if ((!m_currPub) ||
        (topicIdField.value() != m_currTopicInfo.m_topicId) ||
        (msgIdField.value() != m_currMsgId)) {
        return;
    }

    if (retCodeField.value() == mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId) {
        state().m_regMgr.discardRegistration(m_currTopicInfo.m_topicId);
        sendCurrent();
        return;
    }

    if ((retCodeField.value() == mqttsn::protocol::field::ReturnCodeVal_Accepted) &&
        (m_currPub->m_qos == QoS_ExactlyOnceDelivery)) {
        return; // "PUBREC" is expected instead of "PUBACK"
    }

    m_currPub.reset();
    checkSend();
}

void PubSend::handle(PubrecMsg_SN& msg)
{
    typedef PubrecMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    if ((!m_currPub) || (msgIdField.value() != m_currMsgId)) {
        return;
    }

    cancelTick();
    m_acked = true;
    m_attempt = 0;
    sendPubrel();
}

void PubSend::handle(PubcompMsg_SN& msg)
{
    typedef PubcompMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    if ((!m_currPub) || (msgIdField.value() != m_currMsgId)) {
        return;
    }

    cancelTick();
    m_currPub.reset();
    checkSend();
}

void PubSend::handle(MqttsnMessage& msg)
{
    static_cast<void>(msg);
    checkSend();
}

void PubSend::handle(MqttMessage& msg)
{
    static_cast<void>(msg);
    checkSend();
}

void PubSend::newSends()
{
    cancelTick();
    assert(!m_currPub);
    auto& st = state();
    while ((!st.m_brokerPubs.empty()) && (!m_currPub)) {
        m_currPub = std::move(st.m_brokerPubs.front());
        st.m_brokerPubs.pop_front();

        sendCurrent();
    }

    if (st.m_brokerPubs.empty() && st.m_pendingClientDisconnect) {
        sendDisconnect();
    }
}

void PubSend::sendCurrent()
{
    assert(m_currPub);
    m_attempt = 0;

    m_currTopicInfo = state().m_regMgr.mapTopic(m_currPub->m_topic);
    assert(0 < m_currTopicInfo.m_topicId);
    m_currMsgId = allocMsgId();
    m_registered = false;
    m_acked = false;

    doSend();
}

void PubSend::doSend()
{
    auto& st = state();
    if (st.m_retryCount <= m_attempt) {
        m_currPub.reset();
        checkSend();
        return;
    }

    ++m_attempt;

    assert(m_currTopicInfo.m_topicId != 0);
    if ((m_currTopicInfo.m_newInsersion) &&
        (!m_currTopicInfo.m_predefined) &&
        (!m_registered)) {
        RegisterMsg_SN msg;
        auto& fields = msg.fields();
        auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
        auto& topicNameField = std::get<decltype(msg)::FieldIdx_topicName>(fields);

        m_currMsgId = allocMsgId();
        topicIdField.value() = m_currTopicInfo.m_topicId;
        msgIdField.value() = m_currMsgId;
        topicNameField.value() = m_currPub->m_topic;
        sendToClient(msg);
        nextTickReq(st.m_retryPeriod);
        return;
    }

    PublishMsg_SN msg;
    auto& fields = msg.fields();
    auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& topicTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
    auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
    auto& dataField = std::get<decltype(msg)::FieldIdx_data>(fields);

    auto topicType = mqttsn::protocol::field::TopicIdTypeVal::Normal;
    if (m_currTopicInfo.m_predefined) {
        topicType = mqttsn::protocol::field::TopicIdTypeVal::PreDefined;
    }

    bool dup = m_currPub->m_dup || (1U < m_attempt);

    topicTypeField.value() = topicType;
    midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_retain, m_currPub->m_retain);
    qosField.value() = translateQosForClient(m_currPub->m_qos);
    dupFlagsField.setBitValue(mqttsn::protocol::field::DupFlagsBits_dup, dup);
    topicIdField.value() = m_currTopicInfo.m_topicId;
    msgIdField.value() = m_currMsgId;
    dataField.value() = m_currPub->m_msg;
    sendToClient(msg);

    if (m_currPub->m_qos == QoS_AtMostOnceDelivery) {
        m_currPub.reset();
        return;
    }

    nextTickReq(st.m_retryPeriod);
}

unsigned PubSend::allocMsgId()
{
    return ++m_nextMsgId;
}

void PubSend::sendDisconnect()
{
    DisconnectMsg_SN msg;
    auto& fields = msg.fields();
    auto& durationField = std::get<decltype(msg)::FieldIdx_duration>(fields);
    durationField.setMode(comms::field::OptionalMode::Missing);
    sendToClient(msg);
    termRequest();
}

void PubSend::sendPubrel()
{
    PubrelMsg_SN msg;
    auto& fields = msg.fields();
    auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
    msgIdField.value() = m_currMsgId;
    sendToClient(msg);
    nextTickReq(state().m_retryPeriod);
}

void PubSend::checkSend()
{
    if ((m_currPub) ||
        (state().m_connStatus != ConnectionStatus::Connected)) {
        return;
    }

    if ((!state().m_brokerPubs.empty()) || (state().m_pendingClientDisconnect)) {
        newSends();
    }
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



