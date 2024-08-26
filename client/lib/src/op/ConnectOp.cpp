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

    if (client().clientState().m_firstConnect && (!config->m_cleanSession) && client().configState().m_verifySubFilter) {
        errorLog("First connect must force clean session");
        return CC_MqttsnErrorCode_BadParam;
    }    

    if (config->m_duration == 0U) {
        errorLog("The connect duration value must be greater than 0");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (config->m_clientId != nullptr) {
        m_connectMsg.field_clientId().value() = config->m_clientId;
    }

    comms::units::setSeconds(m_connectMsg.field_duration(), config->m_duration);
    m_connectMsg.field_flags().field_mid().setBitValue_CleanSession(config->m_cleanSession);
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode ConnectOp::willConfig([[maybe_unused]] const CC_MqttsnWillConfig* config)
{
#if CC_MQTTSN_CLIENT_HAS_WILL    
    if (config == nullptr) {
        errorLog("Will configuration is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    m_connectMsg.field_flags().field_mid().setBitValue_Will(true);
    if ((config->m_topic == nullptr) || (config->m_topic[0] == '\0')) {
        if (0U < config->m_dataLen) {
            errorLog("Will configuration contains empty topic and not empty message.");
            return CC_MqttsnErrorCode_BadParam;            
        }

        m_willtopicMsg.field_flags().setMissing();
        m_willtopicMsg.field_willTopic().value().clear();
        return CC_MqttsnErrorCode_Success;
    } 

    if ((config->m_qos < CC_MqttsnQoS_AtMostOnceDelivery) || (CC_MqttsnQoS_ExactlyOnceDelivery < config->m_qos)) {
        errorLog("Invalid will QoS configuration.");
        return CC_MqttsnErrorCode_BadParam;
    }

    m_willtopicMsg.field_flags().setExists();
    m_willtopicMsg.field_flags().field().field_qos().setValue(config->m_qos);
    m_willtopicMsg.field_flags().field().field_mid().setBitValue_Retain(config->m_retain);
    m_willtopicMsg.field_willTopic().value() = config->m_topic;

    if (0U < config->m_dataLen) {
        if (config->m_data == nullptr) {
            errorLog("Invalid will data configuration.");
            return CC_MqttsnErrorCode_BadParam;
        }

        comms::util::assign(m_willmsgMsg.field_willMsg().value(), config->m_data, config->m_data + config->m_dataLen);
    }
    return CC_MqttsnErrorCode_Success;
#else // #if CC_MQTTSN_CLIENT_HAS_WILL
    errorLog("Will configuration is not supported");
    return CC_MqttsnErrorCode_NotSupported;
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL    
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

    if (m_connectMsg.field_duration().value() == 0U) {
        errorLog("The connect operation hasn't been configured properly");
        return CC_MqttsnErrorCode_InsufficientConfig;
    }

    if ((!m_connectMsg.field_flags().field_mid().getBitValue_CleanSession()) && 
        (client().clientState().m_firstConnect) && 
        (client().configState().m_verifySubFilter)) {
        errorLog("Clean session flag needs to be set on the first connection attempt, perform configuration first.");
        return CC_MqttsnErrorCode_InsufficientConfig;
    }      

    if (!m_timer.isValid()) {
        errorLog("The library cannot allocate required number of timers.");
        return CC_MqttsnErrorCode_InternalError;
    }    

    auto guard = client().apiEnter();
    m_cb = cb;
    m_cbData = cbData;
    
    m_origRetryCount = getRetryCount();

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    auto& sessionState = client().sessionState();
    sessionState.m_clientId.clear();
    sessionState.m_connectionStatus = CC_MqttsnConnectionStatus_Disconnected;
    if (m_connectMsg.field_flags().field_mid().getBitValue_CleanSession()) {
        // Don't wait for acknowledgement, assume state cleared upon send
        client().reuseState() = ReuseState();
    }    

    completeOnError.release();
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

#if CC_MQTTSN_CLIENT_HAS_WILL
void ConnectOp::handle([[maybe_unused]] WilltopicreqMsg& msg)
{
    if (!m_connectMsg.field_flags().field_mid().getBitValue_Will()) {
        errorLog("WILLTOPICREQ message when will is disabled, ignoring");
        return;
    }

    if (m_willtopicMsg.field_willTopic().value().empty()) {
        // The will message won't be requested
        client().reuseState().m_prevWill = ReuseState::WillInfo();
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

    if (m_stage != Stage_willTopic) {
        errorLog("WILLMSGREQ before WILLTOPICREQ, ignoring");
        return;        
    }

    auto& prevWill = client().reuseState().m_prevWill;
    prevWill.m_topic = m_willtopicMsg.field_willTopic().value().c_str();
    prevWill.m_msg.clear();
    prevWill.m_qos = static_cast<decltype(prevWill.m_qos)>(m_willtopicMsg.field_flags().field().field_qos().value());
    prevWill.m_retain = m_willtopicMsg.field_flags().field().field_mid().getBitValue_Retain();
    
    setRetryCount(m_origRetryCount);
    m_stage = Stage_willMsg;

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }
}
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL

void ConnectOp::handle(ConnackMsg& msg)
{
    auto info = CC_MqttsnConnectInfo();
    comms::cast_assign(info.m_returnCode) = msg.field_returnCode().value();

#if CC_MQTTSN_CLIENT_HAS_WILL
    bool clearPrevWillInfo = (info.m_returnCode != CC_MqttsnReturnCode_Accepted);
    
    if ((info.m_returnCode == CC_MqttsnReturnCode_Accepted) && 
        (m_stage < Stage_willMsg) && 
        (m_connectMsg.field_flags().field_mid().getBitValue_Will())) {

        errorLog("Connection accepted without full will inquiry");
        clearPrevWillInfo = true;
    }

    if (clearPrevWillInfo) {
        client().reuseState().m_prevWill = ReuseState::WillInfo();
    }
    else if (Stage_willMsg <= m_stage) {
        auto& dataVec = m_willmsgMsg.field_willMsg().value();
        comms::util::assign(client().reuseState().m_prevWill.m_msg, dataVec.begin(), dataVec.end());
    }

#endif // #if CC_MQTTSN_CLIENT_HAS_WILL    

    if (info.m_returnCode == CC_MqttsnReturnCode_Accepted) {
        client().sessionState().m_clientId = m_connectMsg.field_clientId().value();
        client().sessionState().m_keepAliveMs = comms::units::getMilliseconds<unsigned>(m_connectMsg.field_duration());
        client().gatewayConnected();
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
#if CC_MQTTSN_CLIENT_HAS_WILL        
        /* Stage_willTopic */ &ConnectOp::getWilltopicMsg,
        /* Stage_willMsg */ &ConnectOp::getWillmsgMsg,        
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL        
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

void ConnectOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the connect operation have been exhausted.");
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

const ProtMessage& ConnectOp::getConnectMsg() const
{
    return m_connectMsg;
}

#if CC_MQTTSN_CLIENT_HAS_WILL
const ProtMessage& ConnectOp::getWilltopicMsg() const
{
    return m_willtopicMsg;
}

const ProtMessage& ConnectOp::getWillmsgMsg() const
{
    return m_willmsgMsg;
}
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL

void ConnectOp::opTimeoutCb(void* data)
{
    asConnectOp(data)->timeoutInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
