//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/SendOp.h"
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

inline SendOp* asSendOp(void* data)
{
    return reinterpret_cast<SendOp*>(data);
}

inline CC_MqttsnPublishHandle asHandle(SendOp* op)
{
    return reinterpret_cast<CC_MqttsnPublishHandle>(op);
}

} // namespace 
    

SendOp::SendOp(ClientImpl& client) : 
    Base(client),
    m_timer(client.timerMgr().allocTimer())
{
}   

SendOp::~SendOp()
{
    releasePacketId(m_registerMsg.field_msgId().value());
    releasePacketId(m_publishMsg.field_msgId().value());
}

CC_MqttsnErrorCode SendOp::config(const CC_MqttsnPublishConfig* config)
{
    if (config == nullptr) {
        errorLog("Publish configuration is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    bool emptyTopic = 
        (config->m_topic == nullptr) ||
        (config->m_topic[0] == '\0');

    if (emptyTopic && (!isValidTopicId(config->m_topicId))) {
        errorLog("Neither topic nor pre-defined topic ID are provided in PUBLISH configuration.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (static_cast<decltype(config->m_qos)>(Config::MaxQos) < config->m_qos) {
        errorLog("Bad publish qos value.");
        return CC_MqttsnErrorCode_BadParam;        
    }    

    if ((0U < config->m_dataLen) && (config->m_data == nullptr)) {
        errorLog("Bad publish message data.");
        return CC_MqttsnErrorCode_BadParam;          
    }

    if ((!emptyTopic) && (!verifyPubTopic(config->m_topic, true))) {
        errorLog("Bad topic filter format in publish.");
        return CC_MqttsnErrorCode_BadParam;
    }    

    m_publishMsg.field_flags().field_qos().setValue(config->m_qos);
    m_publishMsg.field_flags().field_mid().setBitValue_Retain(config->m_retain);
    
    if (0U < config->m_dataLen) {
        comms::util::assign(m_publishMsg.field_data().value(), config->m_data, config->m_data + config->m_dataLen);
    }
    else {
        m_publishMsg.field_data().value().clear();
    }

    using TopicIdType = PublishMsg::Field_flags::Field_topicIdType::ValueType;
    do {
        if (emptyTopic) {
            m_publishMsg.field_topicId().setValue(config->m_topicId);
            m_publishMsg.field_flags().field_topicIdType().value() = TopicIdType::PredefinedTopicId;
            m_stage = Stage_Publish;
            break;
        }

        if (isShortTopic(config->m_topic)) {
            auto topicId = 
                (static_cast<std::uint16_t>(config->m_topic[0]) << 8U) | 
                (static_cast<std::uint8_t>(config->m_topic[1]));
            m_publishMsg.field_topicId().setValue(topicId);
            m_publishMsg.field_flags().field_topicIdType().value() = TopicIdType::ShortTopicName;
            m_stage = Stage_Publish;
            break;
        }

        auto& regMap = client().reuseState().m_outRegTopics;
        auto iter = 
            std::lower_bound(
                regMap.begin(), regMap.end(), config->m_topic,
                [](auto& elem, const char* topicParam)
                {
                    return elem.m_topic < topicParam;
                });   

        if ((iter != regMap.end()) && (iter->m_topic == config->m_topic)) {
            m_publishMsg.field_topicId().setValue(iter->m_topicId);
            m_publishMsg.field_flags().field_topicIdType().value() = TopicIdType::Normal;
            m_stage = Stage_Publish;
            break;
        }

        m_registerMsg.field_topicName().value() = config->m_topic;
        m_stage = Stage_Register;
    } while (false);

    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode SendOp::send(CC_MqttsnPublishCompleteCb cb, void* cbData) 
{
    client().allowNextPrepare();
    auto completeOnError = 
        comms::util::makeScopeGuard(
            [this]()
            {
                opComplete();
            });

    if (cb == nullptr) {
        errorLog("Publish completion callback is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (!m_timer.isValid()) {
        errorLog("The library cannot allocate required number of timers.");
        return CC_MqttsnErrorCode_InternalError;
    }    

    auto guard = client().apiEnter();
    m_cb = cb;
    m_cbData = cbData;

    if (m_stage == Stage_Register) {
        m_registerMsg.field_msgId().setValue(allocPacketId());    
    }

    using QoS = PublishMsg::Field_flags::Field_qos::ValueType;
    if (QoS::AtMostOnceDelivery < m_publishMsg.field_flags().field_qos().value()) {
        m_publishMsg.field_msgId().setValue(allocPacketId());
    }

    m_origRetryCount = getRetryCount();

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    completeOnError.release();
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode SendOp::cancel()
{
    if (m_cb == nullptr) {
        // hasn't been sent yet
        client().allowNextPrepare();
    }

    opComplete();
    return CC_MqttsnErrorCode_Success;
}

void SendOp::resume()
{
    COMMS_ASSERT(m_suspended);
    m_suspended = false;
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        errorLog("Failed to send SUBSCRIBE, after prev SUBSCRIBE completion");
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }
}

void SendOp::handle(RegackMsg& msg)
{
    if (m_suspended) {
        return;
    }

    if ((Stage_Register < m_stage) || (msg.field_msgId().value() != m_registerMsg.field_msgId().value())) {
        errorLog("Unexpected REGACK message, ignoring");
        return;
    }    

    if (msg.field_returnCode().value() != RegackMsg::Field_returnCode::ValueType::Accepted) {
        auto info = CC_MqttsnPublishInfo();
        comms::cast_assign(info.m_returnCode) = msg.field_returnCode().value();
        completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &info);
        return;
    }    

    auto topicId = msg.field_topicId().value();
    if (!isValidTopicId(topicId)) {
        errorLog("Unexpected topic ID in REGACK message, ignoring");
        return;
    }

    auto& regMap = client().reuseState().m_outRegTopics;
    auto& topicStr = m_registerMsg.field_topicName().value();
    COMMS_ASSERT(!topicStr.empty());

    auto iter = 
        std::lower_bound(
            regMap.begin(), regMap.end(), topicStr,
            [](auto& elem, auto& topicParam)
            {
                return elem.m_topic < topicParam;
            });   

    do {
        if ((iter == regMap.end()) || (iter->m_topic != topicStr)) {
            regMap.emplace(iter, topicStr.c_str(), topicId);
            break;
        }

        iter->m_topicId = topicId;
    } while (false);

    m_stage = Stage_Publish; 
    m_publishMsg.field_topicId().setValue(topicId);
    setRetryCount(m_origRetryCount);
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }
}

#if CC_MQTTSN_CLIENT_MAX_QOS > 0
void SendOp::handle(PubackMsg& msg)
{
    if ((m_suspended) ||
        (m_stage < Stage_Publish) || 
        (msg.field_msgId().value() != m_publishMsg.field_msgId().value()) ||
        (msg.field_topicId().value() != m_publishMsg.field_topicId().value()))  {
        return;
    }

    auto info = CC_MqttsnPublishInfo();
    info.m_returnCode = static_cast<decltype(info.m_returnCode)>(msg.field_returnCode().value());    

    using Qos = PublishMsg::Field_flags::Field_qos::ValueType;
    if ((info.m_returnCode == CC_MqttsnReturnCode_Accepted) && 
        (m_publishMsg.field_flags().field_qos().value() != Qos::AtLeastOnceDelivery)) {
        errorLog("Received PUBACK instead of PUBREC, ignoring...");
        return;
    }

    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &info);
}

#if CC_MQTTSN_CLIENT_MAX_QOS > 1
void SendOp::handle(PubrecMsg& msg)
{
    if ((m_suspended) || 
        (m_stage < Stage_Publish) || 
        (msg.field_msgId().value() != m_publishMsg.field_msgId().value()))  {
        return;
    }

    using Qos = PublishMsg::Field_flags::Field_qos::ValueType;
    if (m_publishMsg.field_flags().field_qos().value() != Qos::ExactlyOnceDelivery) {
        errorLog("Received PUBREC instead of PUBACK, ignoring...");
        return;
    }    

    m_stage = Stage_Acked;
    setRetryCount(m_origRetryCount);
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }    
}

void SendOp::handle(PubcompMsg& msg)
{
    if ((m_suspended) || 
        (m_stage < Stage_Acked) || 
        (msg.field_msgId().value() != m_publishMsg.field_msgId().value()))  {
        return;
    }

    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete);
}

#endif // #if CC_MQTTSN_CLIENT_MAX_QOS > 1
#endif // #if CC_MQTTSN_CLIENT_MAX_QOS > 0

Op::Type SendOp::typeImpl() const
{
    return Type_Send;
}

void SendOp::terminateOpImpl(CC_MqttsnAsyncOpStatus status)
{
    completeOpInternal(status);
}

void SendOp::completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info)
{
    auto handle = asHandle(this);
    auto cb = m_cb;
    auto* cbData = m_cbData;
    opComplete(); // mustn't access data members after destruction
    if (cb != nullptr) {
        cb(cbData, handle, status, info);    
    }
}

void SendOp::restartTimer()
{
    m_timer.wait(getRetryPeriod(), &SendOp::opTimeoutCb, this);
}

CC_MqttsnErrorCode SendOp::sendInternal()
{
    if (m_suspended) {
        return CC_MqttsnErrorCode_Success;
    }

    using SendFunc = CC_MqttsnErrorCode (SendOp::*)();
    static const SendFunc Map[] = {
        /* Stage_Register */ &SendOp::sendInternal_Register,
        /* Stage_Publish */ &SendOp::sendInternal_Publish,
        /* Stage_Acked */ &SendOp::sendInternal_Pubrel,
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == Stage_ValuesLimit);

    auto idx = static_cast<unsigned>(m_stage);
    COMMS_ASSERT(idx < MapSize);
    auto func = Map[idx];
    return (this->*func)();
}

CC_MqttsnErrorCode SendOp::sendInternal_Register()
{
    auto ec = sendMessage(m_registerMsg);
    if (ec == CC_MqttsnErrorCode_Success) {
        restartTimer();
    }
    
    return ec;     
}

CC_MqttsnErrorCode SendOp::sendInternal_Publish()
{
    COMMS_ASSERT(isValidTopicId(m_publishMsg.field_topicId().value()));

    if constexpr (0 < Config::MaxQos) {
        if (getRetryCount() < m_origRetryCount) {
            m_publishMsg.field_flags().field_high().setBitValue_Dup(true);
        }
    }
    
    auto ec = sendMessage(m_publishMsg);
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    using QoS = PublishMsg::Field_flags::Field_qos::ValueType;
    if (QoS::AtMostOnceDelivery == m_publishMsg.field_flags().field_qos().value()) {
        completeOpInternal(CC_MqttsnAsyncOpStatus_Complete);
        return CC_MqttsnErrorCode_Success;
    }

    COMMS_ASSERT(m_publishMsg.field_msgId().value() > 0U);
    restartTimer();
    return CC_MqttsnErrorCode_Success; 
}

CC_MqttsnErrorCode SendOp::sendInternal_Pubrel()
{
    if constexpr (2 <= Config::MaxQos) {
        PubrelMsg pubrelMsg;
        pubrelMsg.field_msgId().setValue(m_publishMsg.field_msgId().value());
        auto ec = sendMessage(pubrelMsg);
        if (ec == CC_MqttsnErrorCode_Success) {
            restartTimer();
        }

        return ec;    
    }
    else {
        return CC_MqttsnErrorCode_NotSupported;
    }
}

void SendOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the publish operation have been exhausted.");
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

void SendOp::opTimeoutCb(void* data)
{
    asSendOp(data)->timeoutInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
