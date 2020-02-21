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
    if ((!m_currPub) ||
        (msg.field_topicId().value() != m_currTopicInfo.m_topicId) ||
        (msg.field_msgId().value() != m_currMsgId)) {
        return;
    }

    cancelTick();
    if (msg.field_returnCode().value() != mqttsn::protocol::field::ReturnCodeVal_Accepted) {
        m_currPub.reset();
        checkSend();
        return;
    }

    m_attempt = 0;
    m_registered = true;
    ++m_registerCount;
    m_currMsgId = allocMsgId(); 
    doSend();
}

void PubSend::handle(PubackMsg_SN& msg)
{
    if (msg.field_returnCode().value() == mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId) {
        state().m_regMgr.discardRegistration(msg.field_topicId().value());
    }

    if ((!m_currPub) ||
        (msg.field_topicId().value() != m_currTopicInfo.m_topicId) ||
        (msg.field_msgId().value() != m_currMsgId)) {
        return;
    }

    if ((msg.field_returnCode().value() == mqttsn::protocol::field::ReturnCodeVal_Accepted) &&
        (m_currPub->m_qos == QoS_ExactlyOnceDelivery)) {
        return; // "PUBREC" is expected instead of "PUBACK"
    }

    cancelTick();
    if (msg.field_returnCode().value() == mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId) {
        sendCurrent();
        return;
    }

    m_currPub.reset();
    checkSend();
}

void PubSend::handle(PubrecMsg_SN& msg)
{
    if ((!m_currPub) || (msg.field_msgId().value() != m_currMsgId)) {
        return;
    }

    cancelTick();
    m_acked = true;
    m_attempt = 0;
    sendPubrel();
}

void PubSend::handle(PubcompMsg_SN& msg)
{
    if ((!m_currPub) || (msg.field_msgId().value() != m_currMsgId)) {
        return;
    }

    cancelTick();
    m_currPub.reset();
    checkSend();
}

void PubSend::handle(PingreqMsg_SN& msg)
{
    if (state().m_connStatus != ConnectionStatus::Asleep) {
        return;
    }

    if (msg.field_clientId().value() != state().m_clientId) {
        return;
    }

    m_ping = true;
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

        m_registerCount = 0U;
        sendCurrent();
    }

    if (!st.m_brokerPubs.empty()) {
        return;
    }

    if (st.m_pendingClientDisconnect) {
        sendDisconnect();
        return;
    }

    if (m_ping) {
        m_ping = false;
        sendToClient(PingrespMsg_SN());
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
        (m_currTopicInfo.m_topicIdType == RegMgr::TopicIdType::Normal) &&
        (!m_registered)) {

        if (st.m_retryCount <= m_registerCount) {
            m_currPub.reset();
            checkSend();
            return;
        }

        RegisterMsg_SN msg;
        m_currMsgId = allocMsgId();
        msg.field_topicId().value() = m_currTopicInfo.m_topicId;
        msg.field_msgId().value() = m_currMsgId;

        auto& topicStorage = msg.field_topicName().value();
        using TopicStorage = typename std::decay<decltype(topicStorage)>::type;
        msg.field_topicName().value() = TopicStorage(m_currPub->m_topic.c_str(), m_currPub->m_topic.size());
        sendToClient(msg);
        nextTickReq(st.m_retryPeriod);
        return;
    }

    PublishMsg_SN msg;
    auto& midFlagsField = msg.field_flags().field_midFlags();
    auto& dupFlagsField = msg.field_flags().field_dupFlags();

    typedef typename std::decay<decltype(midFlagsField)>::type MidFlags;
    typedef typename std::decay<decltype(dupFlagsField)>::type DupFlags;

    bool dup = m_currPub->m_dup || (1U < m_attempt);

    msg.field_flags().field_topicId().value() = m_currTopicInfo.m_topicIdType;
    midFlagsField.setBitValue(MidFlags::BitIdx_retain, m_currPub->m_retain);
    msg.field_flags().field_qos().value() = translateQosForClient(m_currPub->m_qos);
    dupFlagsField.setBitValue(DupFlags::BitIdx_bit, dup);
    msg.field_topicId().value() = m_currTopicInfo.m_topicId;

    if (m_currPub->m_qos >= QoS_AtLeastOnceDelivery) {
        if (m_currMsgId == 0U) {
            m_currMsgId = allocMsgId();
        }

        msg.field_msgId().value() = m_currMsgId;
    }

    auto& dataStorage = msg.field_data().value();
    using DataStorage = typename std::decay<decltype(dataStorage)>::type;
    msg.field_data().value() = DataStorage(&(*m_currPub->m_msg.begin()), m_currPub->m_msg.size());
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
    sendDisconnectToClient();
    termRequest();
}

void PubSend::sendPubrel()
{
    PubrelMsg_SN msg;
    msg.field_msgId().value() = m_currMsgId;
    sendToClient(msg);
    nextTickReq(state().m_retryPeriod);
}

void PubSend::checkSend()
{
    auto& st = state();
    if ((m_currPub) ||
        (st.m_connStatus == ConnectionStatus::Disconnected)) {
        return;
    }

    if ((st.m_connStatus == ConnectionStatus::Asleep) && (m_ping)) {
        newSends();
        return;
    }

    if (st.m_connStatus != ConnectionStatus::Connected) {
        return;
    }

    if ((!state().m_brokerPubs.empty()) || (state().m_pendingClientDisconnect)) {
        newSends();
    }
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



