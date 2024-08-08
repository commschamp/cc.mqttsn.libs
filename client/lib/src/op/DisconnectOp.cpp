//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/DisconnectOp.h"
#include "ClientImpl.h"

#include "comms/util/assign.h"
#include "comms/util/ScopeGuard.h"
#include "comms/units.h"

#include <limits>

namespace cc_mqttsn_client
{

namespace op
{

namespace 
{

inline DisconnectOp* asDisconnectOp(void* data)
{
    return reinterpret_cast<DisconnectOp*>(data);
}

} // namespace 
    

DisconnectOp::DisconnectOp(ClientImpl& client) : 
    Base(client),
    m_timer(client.timerMgr().allocTimer())
{
}   

CC_MqttsnErrorCode DisconnectOp::config(const CC_MqttsnSleepConfig* config)
{
    if (config == nullptr) {
        errorLog("Sleep configuration is not provided");
        return CC_MqttsnErrorCode_BadParam;
    }

    auto& st = client().configState();
    auto minDurationMs = (st.m_retryPeriod * (st.m_retryCount + 1U));
    if ((config->m_duration * 1000) <= minDurationMs) {
        errorLog("Sleep duration is too low, must allow multiple attempts to send PINGREQ");
        return CC_MqttsnErrorCode_BadParam;
    }

    using DurationValueType = DisconnectMsg::Field_duration::Field::ValueType;
    if (std::numeric_limits<DurationValueType>::max() < config->m_duration) {
        errorLog("Sleep duration value is too high, doesn't fit into protocol");
        return CC_MqttsnErrorCode_BadParam;        
    }

    comms::units::setSeconds(m_disconnectMsg.field_duration().field(), config->m_duration);
    m_disconnectMsg.field_duration().setExists();
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode DisconnectOp::send(CC_MqttsnDisconnectCompleteCb cb, void* cbData) 
{
    client().allowNextPrepare();
    auto completeOnError = 
        comms::util::makeScopeGuard(
            [this]()
            {
                opComplete();
            });

    if (cb == nullptr) {
        errorLog("Disconnect completion callback is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (!m_timer.isValid()) {
        errorLog("The library cannot allocate required number of timers.");
        return CC_MqttsnErrorCode_InternalError;
    }    

    auto guard = client().apiEnter();
    m_cb = cb;
    m_cbData = cbData;
    
    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        return ec;
    }

    completeOnError.release();
    return CC_MqttsnErrorCode_Success;
}

CC_MqttsnErrorCode DisconnectOp::cancel()
{
    if (m_cb == nullptr) {
        // hasn't been sent yet
        client().allowNextPrepare();
    }

    opComplete();
    return CC_MqttsnErrorCode_Success;
}

void DisconnectOp::handle([[maybe_unused]] DisconnectMsg& msg)
{
    m_timer.cancel();

    auto& cl = client();
    unsigned sleepDurationMs = 0U;
    if (isSleepConfigured()) {
        sleepDurationMs = comms::units::getMilliseconds<decltype(sleepDurationMs)>(m_disconnectMsg.field_duration().field());
    }

    auto onExit = 
        comms::util::makeScopeGuard(
            [&cl, sleepDurationMs]()
            {
                if (0U < sleepDurationMs) {
                    cl.enterSleepMode(sleepDurationMs);
                    return;
                }

                cl.gatewayDisconnected(CC_MqttsnGatewayDisconnectReason_ValuesLimit, CC_MqttsnAsyncOpStatus_Aborted);
            });

    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete);
}

Op::Type DisconnectOp::typeImpl() const
{
    return Type_Disconnect;
}

void DisconnectOp::terminateOpImpl(CC_MqttsnAsyncOpStatus status)
{
    completeOpInternal(status);
}

void DisconnectOp::completeOpInternal(CC_MqttsnAsyncOpStatus status)
{
    auto cb = m_cb;
    auto* cbData = m_cbData;
    opComplete(); // mustn't access data members after destruction
    if (cb != nullptr) {
        cb(cbData, status);    
    }
}

void DisconnectOp::restartTimer()
{
    m_timer.wait(getRetryPeriod(), &DisconnectOp::opTimeoutCb, this);
}

CC_MqttsnErrorCode DisconnectOp::sendInternal()
{
    auto ec = sendMessage(m_disconnectMsg);
    if (ec == CC_MqttsnErrorCode_Success) {
        restartTimer();
    }
    
    return ec;
}

void DisconnectOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the disconnect or sleep operation have been exhausted.");
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

void DisconnectOp::opTimeoutCb(void* data)
{
    asDisconnectOp(data)->timeoutInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
