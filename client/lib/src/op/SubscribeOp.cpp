//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/SubscribeOp.h"
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

inline SubscribeOp* asSubscribeOp(void* data)
{
    return reinterpret_cast<SubscribeOp*>(data);
}

inline CC_MqttsnSubscribeHandle asHandle(SubscribeOp* op)
{
    return reinterpret_cast<CC_MqttsnSubscribeHandle>(op);
}

} // namespace 
    

SubscribeOp::SubscribeOp(ClientImpl& client) : 
    Base(client),
    m_timer(client.timerMgr().allocTimer())
{
}   

SubscribeOp::~SubscribeOp()
{
    releasePacketId(m_subscribeMsg.field_msgId().value());
}

CC_MqttsnErrorCode SubscribeOp::config(const CC_MqttsnSubscribeConfig* config)
{
    if (config == nullptr) {
        errorLog("Subscribe configuration is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    bool emptyTopic = 
        (config->m_topic == nullptr) ||
        (config->m_topic[0] == '\0');

    if (emptyTopic && (!isValidTopicId(config->m_topicId))) {
        errorLog("Neither topic nor pre-defined topic ID are provided in SUBSCRIBE configuration.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (static_cast<decltype(config->m_qos)>(Config::MaxQos) < config->m_qos) {
        errorLog("Bad subscription qos value.");
        return CC_MqttsnErrorCode_BadParam;        
    }    

    if ((!emptyTopic) && (!verifySubFilter(config->m_topic))) {
        errorLog("Bad topic filter format in subscribe.");
        return CC_MqttsnErrorCode_BadParam;
    }    

    m_subscribeMsg.field_flags().field_qos().setValue(config->m_qos);

    using TopicIdType = SubscribeMsg::Field_flags::Field_topicIdType::ValueType;
    if (emptyTopic) {
        m_subscribeMsg.field_topicId().field().setValue(config->m_topicId);
        m_subscribeMsg.field_flags().field_topicIdType().value() = TopicIdType::PredefinedTopicId;
        return CC_MqttsnErrorCode_Success;
    }

    if (isShortTopic(config->m_topic)) {
        auto topicId = 
            (static_cast<std::uint16_t>(config->m_topic[0]) << 8U) | 
            (static_cast<std::uint8_t>(config->m_topic[1]));
        m_subscribeMsg.field_topicId().field().setValue(topicId);
        m_subscribeMsg.field_flags().field_topicIdType().value() = TopicIdType::ShortTopicName;
        return CC_MqttsnErrorCode_Success;
    }

    m_subscribeMsg.field_topicName().field().value() = config->m_topic;
    m_subscribeMsg.field_flags().field_topicIdType().value() = TopicIdType::Normal;
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode SubscribeOp::send(CC_MqttsnSubscribeCompleteCb cb, void* cbData) 
{
    client().allowNextPrepare();
    auto completeOnError = 
        comms::util::makeScopeGuard(
            [this]()
            {
                opComplete();
            });

    if (cb == nullptr) {
        errorLog("Subscribe completion callback is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (!m_timer.isValid()) {
        errorLog("The library cannot allocate required number of timers.");
        return CC_MqttsnErrorCode_InternalError;
    }    

    auto guard = client().apiEnter();
    m_cb = cb;
    m_cbData = cbData;

    m_subscribeMsg.field_msgId().setValue(allocPacketId());
    m_subscribeMsg.doRefresh(); // Update optionals

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    completeOnError.release();
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode SubscribeOp::cancel()
{
    if (m_cb == nullptr) {
        // hasn't been sent yet
        client().allowNextPrepare();
    }

    opComplete();
    return CC_MqttsnErrorCode_Success;
}

void SubscribeOp::handle(SubackMsg& msg)
{
    if ((m_suspended) || (msg.field_msgId().value() != m_subscribeMsg.field_msgId().value())) {
        return;
    }

    m_timer.cancel();

    auto info = CC_MqttsnSubscribeInfo();
    info.m_returnCode = static_cast<decltype(info.m_returnCode)>(msg.field_returnCode().value());
    info.m_qos = static_cast<decltype(info.m_qos)>(msg.field_flags().field_qos().value());

    auto topicId = static_cast<CC_MqttsnTopicId>(msg.field_topicId().value());
    auto& topicStr = m_subscribeMsg.field_topicName().field().value();
    char shortTopic[sizeof(std::uint16_t) + 1] = {0};

    auto* topicPtr = topicStr.c_str();
    do {
        if (!topicStr.empty()) {
            break;
        }

        using TopicIdType = SubscribeMsg::Field_flags::Field_topicIdType::ValueType;
        if (m_subscribeMsg.field_flags().field_topicIdType().value() == TopicIdType::ShortTopicName) {
            shortTopic[0] = static_cast<char>(m_subscribeMsg.field_topicId().field().value() >> 8U);
            shortTopic[1] = static_cast<char>(m_subscribeMsg.field_topicId().field().value() & 0xff);
            topicPtr = &shortTopic[0];
            break;
        }

        topicPtr = nullptr;
    } while (false);

    if ((topicId != 0U) && (topicPtr != nullptr)) {
        client().storeInRegTopic(topicPtr, topicId);
    }

    auto& filtersMap = client().reuseState().m_subFilters;
    do {
        if (topicPtr != nullptr) {
            auto iter = 
                std::lower_bound(
                    filtersMap.begin(), filtersMap.end(), topicPtr,
                    [](auto& elem, const char* topicParam)
                    {
                        return elem.m_topic < topicParam;
                    });

            if ((iter == filtersMap.end()) || (iter->m_topic != topicPtr)) {
                filtersMap.emplace(iter, topicPtr);    
            }

            break;
        }

        COMMS_ASSERT(m_subscribeMsg.field_topicId().doesExist());    
        COMMS_ASSERT(m_subscribeMsg.field_topicId().field().value() != 0U);

        auto iter = 
            std::find_if(
                filtersMap.begin(), filtersMap.end(),
                [](auto& elem)
                {
                    return !elem.m_topic.empty();
                });

        filtersMap.emplace(iter, static_cast<CC_MqttsnTopicId>(m_subscribeMsg.field_topicId().field().value()));
    } while (false);

    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &info);
}

void SubscribeOp::resume()
{
    if (!m_suspended) {
        return;
    }

    m_suspended = false;
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        errorLog("Failed to send SUBSCRIBE, after prev SUBSCRIBE completion");
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }
}

Op::Type SubscribeOp::typeImpl() const
{
    return Type_Subscribe;
}

void SubscribeOp::terminateOpImpl(CC_MqttsnAsyncOpStatus status)
{
    completeOpInternal(status);
}

void SubscribeOp::completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info)
{
    auto handle = asHandle(this);
    auto cb = m_cb;
    auto* cbData = m_cbData;
    opComplete(); // mustn't access data members after destruction
    if (cb != nullptr) {
        cb(cbData, handle, status, info);    
    }
}

void SubscribeOp::restartTimer()
{
    m_timer.wait(getRetryPeriod(), &SubscribeOp::opTimeoutCb, this);
}

CC_MqttsnErrorCode SubscribeOp::sendInternal()
{
    if (m_suspended) {
        return CC_MqttsnErrorCode_Success; // Send after resume
    }

    auto ec = sendMessage(m_subscribeMsg);
    if (ec == CC_MqttsnErrorCode_Success) {
        restartTimer();
    }
    
    return ec;
}

void SubscribeOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the subscribe operation have been exhausted.");
        completeOpInternal(CC_MqttsnAsyncOpStatus_Timeout);
        return;
    }  

    decRetryCount();
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }  
}

void SubscribeOp::opTimeoutCb(void* data)
{
    asSubscribeOp(data)->timeoutInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
