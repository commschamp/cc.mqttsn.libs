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
    if (ec == CC_MqttsnErrorCode_Success) {
        completeOnError.release();
    }
    
    return ec;
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
    if (getRetryCount() == 0U) {
        errorLog("All retries of the connect operation have been exhausted.");
        completeOpInternal(CC_MqttsnAsyncOpStatus_Timeout);
        return CC_MqttsnErrorCode_InternalError;
    }

    decRetryCount();

    return sendMessage(m_connectMsg);
}

void ConnectOp::opTimeoutCb(void* data)
{
    asConnectOp(data)->sendInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
