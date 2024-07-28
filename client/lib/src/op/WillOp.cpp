//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#include "op/WillOp.h"

#if CC_MQTTSN_CLIENT_HAS_WILL

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

inline WillOp* asWillOp(void* data)
{
    return reinterpret_cast<WillOp*>(data);
}

CC_MqttsnWillInfo initWillInfo()
{
    auto info = CC_MqttsnWillInfo();
    info.m_topicUpdReturnCode = CC_MqttsnReturnCode_ValuesLimit;
    info.m_msgUpdReturnCode = CC_MqttsnReturnCode_ValuesLimit;
    return info;
}

} // namespace 

WillOp::WillOp(ClientImpl& client) : 
    Base(client),
    m_timer(client.timerMgr().allocTimer()),
    m_info(initWillInfo())
{
}   

CC_MqttsnErrorCode WillOp::config(const CC_MqttsnWillConfig* config)
{
    if (config == nullptr) {
        errorLog("Will configuration is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if ((0U < config->m_dataLen) && (config->m_topic == nullptr)) {
        errorLog("Will data is provided without will topic.");
        return CC_MqttsnErrorCode_BadParam;          
    }    

    if ((0U < config->m_dataLen) && (config->m_data == nullptr)) {
        errorLog("Bad will message data.");
        return CC_MqttsnErrorCode_BadParam;          
    }

    if (static_cast<decltype(config->m_qos)>(Config::MaxQos) < config->m_qos) {
        errorLog("Bad will qos value.");
        return CC_MqttsnErrorCode_BadParam;        
    }       

    m_willtopicupdMsg.field_willTopic().value().clear();
    m_willtopicupdMsg.field_flags().setMissing();
    m_willmsgupdMsg.field_willMsg().value().clear();

    if (config->m_topic != nullptr) {
        m_willtopicupdMsg.field_willTopic().value() = config->m_topic;
        m_willtopicupdMsg.field_flags().setExists();
        m_willtopicupdMsg.field_flags().field().field_qos().setValue(config->m_qos);
        m_willtopicupdMsg.field_flags().field().field_mid().setBitValue_Retain(config->m_retain);
    }

    if (0U < config->m_dataLen) {
        comms::util::assign(m_willmsgupdMsg.field_willMsg().value(), config->m_data, config->m_data + config->m_dataLen);
    }

    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode WillOp::send(CC_MqttsnWillCompleteCb cb, void* cbData) 
{
    client().allowNextPrepare();
    auto completeOnError = 
        comms::util::makeScopeGuard(
            [this]()
            {
                opComplete();
            });

    if (cb == nullptr) {
        errorLog("Will completion callback is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    auto guard = client().apiEnter();
    m_cb = cb;
    m_cbData = cbData;

    auto& prevWill = client().reuseState().m_prevWill;
    m_stage = Stage_willTopic;
    if ((!prevWill.m_topic.empty()) &&
        (m_willtopicupdMsg.field_willTopic().value() == prevWill.m_topic) &&
        (prevWill.m_qos == static_cast<decltype(prevWill.m_qos)>(m_willtopicupdMsg.field_flags().field().field_qos().value())) &&
        (prevWill.m_retain == m_willtopicupdMsg.field_flags().field().field_mid().getBitValue_Retain())) {
        m_stage = Stage_willMsg;
    }    

    if ((m_stage == Stage_willMsg) &&
        (m_willmsgupdMsg.field_willMsg().value() == prevWill.m_msg)) {
        completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &m_info);
        return CC_MqttsnErrorCode_Success;
    }

    if (m_stage == Stage_willTopic) {
        prevWill.m_topic.clear();
    }

    if (m_willtopicupdMsg.field_willTopic().value().empty()) {
        prevWill = ReuseState::WillInfo();
    }

    m_origRetryCount = getRetryCount();
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    completeOnError.release();
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode WillOp::cancel()
{
    if (m_cb == nullptr) {
        // hasn't been sent yet
        client().allowNextPrepare();
    }

    opComplete();
    return CC_MqttsnErrorCode_Success;
}

void WillOp::handle(WilltopicrespMsg& msg)
{
    if (m_stage != Stage_willTopic) {
        errorLog("Unexpected WILLTOPICRESP message, ignoring");
        return;
    }

    auto rc = static_cast<CC_MqttsnReturnCode>(msg.field_returnCode().value());
    if (rc != CC_MqttsnReturnCode_Accepted) {
        m_info.m_topicUpdReturnCode = rc;
        completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &m_info);
        return;
    }

    auto& prevWill = client().reuseState().m_prevWill;
    prevWill.m_topic = m_willtopicupdMsg.field_willTopic().value();
    prevWill.m_qos = static_cast<decltype(prevWill.m_qos)>(m_willtopicupdMsg.field_flags().field().field_qos().value());
    prevWill.m_retain = m_willtopicupdMsg.field_flags().field().field_mid().getBitValue_Retain();

    m_stage = Stage_willMsg;
    if (m_willmsgupdMsg.field_willMsg().value() == prevWill.m_msg) {
        completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &m_info);    
        return;
    }

    prevWill.m_msg.clear();
    setRetryCount(m_origRetryCount);
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }     
}

void WillOp::handle(WillmsgrespMsg& msg)
{
    if (m_stage != Stage_willMsg) {
        errorLog("Unexpected WILLMSGRESP message, ignoring");
        return;
    }

    auto rc = static_cast<CC_MqttsnReturnCode>(msg.field_returnCode().value());
    if (rc == CC_MqttsnReturnCode_Accepted) {
        client().reuseState().m_prevWill.m_msg = m_willmsgupdMsg.field_willMsg().value();
    }

    m_info.m_msgUpdReturnCode = rc;
    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &m_info);    
}

Op::Type WillOp::typeImpl() const
{
    return Type_Will;
}

void WillOp::terminateOpImpl(CC_MqttsnAsyncOpStatus status)
{
    completeOpInternal(status);
}

void WillOp::completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnWillInfo* info)
{
    auto cb = m_cb;
    auto* cbData = m_cbData;
    opComplete(); // mustn't access data members after destruction
    if (cb != nullptr) {
        cb(cbData, status, info);    
    }
}

void WillOp::restartTimer()
{
    m_timer.wait(getRetryPeriod(), &WillOp::opTimeoutCb, this);
}

CC_MqttsnErrorCode WillOp::sendInternal()
{
    using GetMsgFunc = const ProtMessage& (WillOp::*)() const;
    static const GetMsgFunc Map[] = {
        /* Stage_willTopic */ &WillOp::getWilltopicupdMsg,
        /* Stage_willMsg */ &WillOp::getWillmsgupdMsg,        
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == Stage_valuesLimit);

    if (MapSize <= m_stage) {
        COMMS_ASSERT(false);
        return CC_MqttsnErrorCode_InternalError;
    }

    auto func = Map[m_stage];
    auto ec = sendMessage((this->*func)());
    if (ec == CC_MqttsnErrorCode_Success) {
        restartTimer();
    }
    
    return ec;
}

void WillOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the will operation have been exhausted.");
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

void WillOp::opTimeoutCb(void* data)
{
    asWillOp(data)->timeoutInternal();
}

const ProtMessage& WillOp::getWilltopicupdMsg() const
{
    return m_willtopicupdMsg;
}

const ProtMessage& WillOp::getWillmsgupdMsg() const
{
    return m_willmsgupdMsg;
}

} // namespace op

} // namespace cc_mqttsn_client

#endif // #if CC_MQTTSN_CLIENT_HAS_WILL        