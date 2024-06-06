//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/ConnectOp.h"
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

inline ConnectOp* asConnectOp(void* data)
{
    return reinterpret_cast<ConnectOp*>(data);
}

} // namespace 
    

ConnectOp::ConnectOp(ClientImpl& client) : 
    Base(client),
    m_timer(client.timerMgr().allocTimer())
{
}   

CC_MqttsnErrorCode ConnectOp::config(const CC_MqttsnConnectConfig* config)
{
    if (config == nullptr) {
        errorLog("Connect configuration is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (config->m_clientId != nullptr) {
        m_connectMsg.field_clientId().value() = config->m_clientId;
    }

    comms::units::setSeconds(m_connectMsg.field_duration(), config->m_duration);
    m_connectMsg.field_flags().field_mid().setBitValue_CleanSession(config->m_cleanSession);
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode ConnectOp::willConfig(const CC_MqttsnWillConfig* config)
{
    if (config == nullptr) {
        errorLog("Will configuration is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    m_connectMsg.field_flags().field_mid().setBitValue_Will(true);
    if ((config->m_topic == nullptr) || (config->m_topic[0] != '\0')) {
        if (0U < config->m_dataLen) {
            errorLog("Will configuration contains empty topic and not empty message.");
            return CC_MqttsnErrorCode_BadParam;            
        }

        return CC_MqttsnErrorCode_Success;
    } 

    if ((config->m_qos < CC_MqttsnQoS_AtMostOnceDelivery) || (CC_MqttsnQoS_ExactlyOnceDelivery < config->m_qos)) {
        errorLog("Invalid will QoS configuration.");
        return CC_MqttsnErrorCode_BadParam;
    }

    m_willtopicMsg.field_flags().setExists();
    m_willtopicMsg.field_flags().field().field_qos().setValue(config->m_qos);
    m_willtopicMsg.field_willTopic().value() = config->m_topic;

    if (0U < config->m_dataLen) {
        if (config->m_data == nullptr) {
            errorLog("Invalid will data configuration.");
            return CC_MqttsnErrorCode_BadParam;
        }

        comms::util::assign(m_willmsgMsg.field_willMsg().value(), config->m_data, config->m_data + config->m_dataLen);
    }
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode ConnectOp::send(CC_MqttsnConnectCompleteCb cb, void* cbData) 
{
    client().allowNextPrepare();
    auto completeOnError = 
        comms::util::makeScopeGuard(
            [this]()
            {
                opComplete();
            });

    if (cb == nullptr) {
        errorLog("Connect completion callback is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (!m_timer.isValid()) {
        errorLog("The library cannot allocate required number of timers.");
        return CC_MqttsnErrorCode_InternalError;
    }    

    m_cb = cb;
    m_cbData = cbData;
    
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    completeOnError.release();
    m_origRetryCount = getRetryCount();
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode ConnectOp::cancel()
{
    if (m_cb == nullptr) {
        // hasn't been sent yet
        client().allowNextPrepare();
    }

    opComplete();
    return CC_MqttsnErrorCode_Success;
}

void ConnectOp::handle([[maybe_unused]] WilltopicreqMsg& msg)
{
    if (!m_connectMsg.field_flags().field_mid().getBitValue_Will()) {
        errorLog("WILLTOPICREQ message when will is disabled, ignoring");
        return;
    }

    setRetryCount(m_origRetryCount);
    m_stage = Stage_willTopic;

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }
}

void ConnectOp::handle([[maybe_unused]] WillmsgreqMsg& msg)
{
    if (!m_connectMsg.field_flags().field_mid().getBitValue_Will()) {
        errorLog("WILLMSGREQ message when will is disabled, ignoring");
        return;
    }

    setRetryCount(m_origRetryCount);
    m_stage = Stage_willMsg;

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }
}

void ConnectOp::handle(ConnackMsg& msg)
{
    auto info = CC_MqttsnConnectInfo();
    comms::cast_assign(info.m_returnCode) = msg.field_returnCode().value();

    if ((info.m_returnCode == CC_MqttsnReturnCode_Accepted) && 
        (m_stage < Stage_willMsg) && 
        (m_connectMsg.field_flags().field_mid().getBitValue_Will())) {
        errorLog("Connection accepted without full will inquiry");
    }

    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &info);
}

Op::Type ConnectOp::typeImpl() const
{
    return Type_Connect;
}

void ConnectOp::terminateOpImpl(CC_MqttsnAsyncOpStatus status)
{
    completeOpInternal(status);
}

void ConnectOp::completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info)
{
    auto cb = m_cb;
    auto* cbData = m_cbData;
    opComplete(); // mustn't access data members after destruction
    if (cb != nullptr) {
        cb(cbData, status, info);    
    }
}

void ConnectOp::restartTimer()
{
    m_timer.wait(getRetryPeriod(), &ConnectOp::opTimeoutCb, this);
}

CC_MqttsnErrorCode ConnectOp::sendInternal()
{
    using GetMsgFunc = const ProtMessage& (ConnectOp::*)() const;
    static const GetMsgFunc Map[] = {
        /* Stage_connect */ &ConnectOp::getConnectMsg,
        /* Stage_willTopic */ &ConnectOp::getWilltopicMsg,
        /* Stage_willMsg */ &ConnectOp::getWillmsgMsg,        
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
        COMMS_ASSERT(0U < getRetryCount());
        decRetryCount();
        restartTimer();
    }
    return ec;
}

void ConnectOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the connect operation have been exhausted.");
        completeOpInternal(CC_MqttsnAsyncOpStatus_Timeout);
        return;
    }  

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }  
}

const ProtMessage& ConnectOp::getConnectMsg() const
{
    return m_connectMsg;
}

const ProtMessage& ConnectOp::getWilltopicMsg() const
{
    return m_willtopicMsg;
}

const ProtMessage& ConnectOp::getWillmsgMsg() const
{
    return m_willmsgMsg;
}

void ConnectOp::opTimeoutCb(void* data)
{
    asConnectOp(data)->timeoutInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
