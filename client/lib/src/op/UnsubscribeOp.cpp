//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/UnsubscribeOp.h"
#include "ClientImpl.h"

#include "comms/util/assign.h"
#include "comms/util/ScopeGuard.h"
#include "comms/units.h"

#include <algorithm>
#include <limits>

namespace cc_mqttsn_client
{

namespace op
{

namespace 
{

inline UnsubscribeOp* asUnsubscribeOp(void* data)
{
    return reinterpret_cast<UnsubscribeOp*>(data);
}

inline CC_MqttsnUnsubscribeHandle asHandle(UnsubscribeOp* op)
{
    return reinterpret_cast<CC_MqttsnUnsubscribeHandle>(op);
}

} // namespace 
    

UnsubscribeOp::UnsubscribeOp(ClientImpl& client) : 
    Base(client),
    m_timer(client.timerMgr().allocTimer())
{
}   

CC_MqttsnErrorCode UnsubscribeOp::config(const CC_MqttsnUnsubscribeConfig* config)
{
    if (config == nullptr) {
        errorLog("Unsubscribe configuration is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    bool emptyTopic = 
        (config->m_topic == nullptr) ||
        (config->m_topic[0] == '\0');

    if (emptyTopic && (!isValidTopicId(config->m_topicId))) {
        errorLog("Neither topic nor pre-defined topic ID are provided in UNSUBSCRIBE configuration.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (static_cast<decltype(config->m_qos)>(Config::MaxQos) < config->m_qos) {
        errorLog("Bad subscription qos value.");
        return CC_MqttsnErrorCode_BadParam;        
    }    

    if ((!emptyTopic) && (!verifySubFilter(config->m_topic))) {
        errorLog("Bad topic filter format in unsubscribe.");
        return CC_MqttsnErrorCode_BadParam;
    }    

    m_unsubscribeMsg.field_flags().field_qos().setValue(config->m_qos);

    using TopicIdType = UnsubscribeMsg::Field_flags::Field_topicIdType::ValueType;
    if (emptyTopic) {
        m_unsubscribeMsg.field_topicId().field().setValue(config->m_topicId);
        m_unsubscribeMsg.field_flags().field_topicIdType().value() = TopicIdType::PredefinedTopicId;
        return CC_MqttsnErrorCode_Success;
    }

    if (isShortTopic(config->m_topic)) {
        auto topicId = 
            (static_cast<std::uint16_t>(config->m_topic[0]) << 8U) | 
            (static_cast<std::uint8_t>(config->m_topic[1]));
        m_unsubscribeMsg.field_topicId().field().setValue(topicId);
        m_unsubscribeMsg.field_flags().field_topicIdType().value() = TopicIdType::ShortTopicName;
        return CC_MqttsnErrorCode_Success;
    }

    m_unsubscribeMsg.field_topicName().field().value() = config->m_topic;
    m_unsubscribeMsg.field_flags().field_topicIdType().value() = TopicIdType::Normal;
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode UnsubscribeOp::send(CC_MqttsnUnsubscribeCompleteCb cb, void* cbData) 
{
    client().allowNextPrepare();
    auto completeOnError = 
        comms::util::makeScopeGuard(
            [this]()
            {
                opComplete();
            });

    if (cb == nullptr) {
        errorLog("Unsubscribe completion callback is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (!m_timer.isValid()) {
        errorLog("The library cannot allocate required number of timers.");
        return CC_MqttsnErrorCode_InternalError;
    }    

    auto guard = client().apiEnter();
    m_cb = cb;
    m_cbData = cbData;

    m_unsubscribeMsg.field_msgId().setValue(allocPacketId());
    m_unsubscribeMsg.doRefresh(); // Update optionals

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    // TODO: remove the record

    completeOnError.release();
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode UnsubscribeOp::cancel()
{
    if (m_cb == nullptr) {
        // hasn't been sent yet
        client().allowNextPrepare();
    }

    opComplete();
    return CC_MqttsnErrorCode_Success;
}

void UnsubscribeOp::handle(UnsubackMsg& msg)
{
    if ((m_suspended) || (msg.field_msgId().value() != m_unsubscribeMsg.field_msgId().value())) {
        return;
    }

    m_timer.cancel();
    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete);
}

void UnsubscribeOp::resume()
{
    COMMS_ASSERT(m_suspended);
    m_suspended = false;
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        errorLog("Failed to send UNSUBSCRIBE, after prev UNSUBSCRIBE completion");
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }
}

Op::Type UnsubscribeOp::typeImpl() const
{
    return Type_Unsubscribe;
}

void UnsubscribeOp::terminateOpImpl(CC_MqttsnAsyncOpStatus status)
{
    completeOpInternal(status);
}

void UnsubscribeOp::completeOpInternal(CC_MqttsnAsyncOpStatus status)
{
    auto handle = asHandle(this);
    auto cb = m_cb;
    auto* cbData = m_cbData;
    opComplete(); // mustn't access data members after destruction
    if (cb != nullptr) {
        cb(cbData, handle, status);    
    }
}

void UnsubscribeOp::restartTimer()
{
    m_timer.wait(getRetryPeriod(), &UnsubscribeOp::opTimeoutCb, this);
}

CC_MqttsnErrorCode UnsubscribeOp::sendInternal()
{
    if (m_suspended) {
        return CC_MqttsnErrorCode_Success; // Send after resume
    }

    auto ec = sendMessage(m_unsubscribeMsg);
    if (ec == CC_MqttsnErrorCode_Success) {
        restartTimer();
    }
    
    return ec;
}

void UnsubscribeOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the unsubscribe operation have been exhausted.");
        completeOpInternal(CC_MqttsnAsyncOpStatus_Timeout);
        return;
    }  

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }  

    decRetryCount();
}

void UnsubscribeOp::opTimeoutCb(void* data)
{
    asUnsubscribeOp(data)->timeoutInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
