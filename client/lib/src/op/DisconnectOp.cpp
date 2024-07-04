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

#include <algorithm>
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
    auto onExit = 
        comms::util::makeScopeGuard(
            [&cl]()
            {
                cl.gatewayDisconnected();
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
        errorLog("All retries of the disconnect operation have been exhausted.");
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

void DisconnectOp::opTimeoutCb(void* data)
{
    asDisconnectOp(data)->timeoutInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
