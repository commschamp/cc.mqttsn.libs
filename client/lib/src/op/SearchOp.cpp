//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/SearchOp.h"
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

inline SearchOp* asSearchOp(void* data)
{
    return reinterpret_cast<SearchOp*>(data);
}

} // namespace 
    

SearchOp::SearchOp(ClientImpl& client) : 
    Base(client),
    m_timer(client.timerMgr().allocTimer()),
    m_radius(client.configState().m_broadcastRadius)
{
}   

CC_MqttsnErrorCode SearchOp::send(CC_MqttsnSearchCompleteCb cb, void* cbData) 
{
    client().allowNextPrepare();
    auto completeOnError = 
        comms::util::makeScopeGuard(
            [this]()
            {
                opComplete();
            });

    if (cb == nullptr) {
        errorLog("Search completion callback is not provided.");
        return CC_MqttsnErrorCode_BadParam;
    }

    if (!m_timer.isValid()) {
        errorLog("The library cannot allocate required number of timers.");
        return CC_MqttsnErrorCode_InternalError;
    }    

    m_cb = cb;
    m_cbData = cbData;
    
    m_searchgwMsg.field_radius().setValue(m_radius);
    auto ec = sendInternal();
    if (ec == CC_MqttsnErrorCode_Success) {
        completeOnError.release();
        restartTimer();
    }
    
    return ec;
}

CC_MqttsnErrorCode SearchOp::cancel()
{
    if (m_cb == nullptr) {
        // hasn't been sent yet
        client().allowNextPrepare();
    }

    opComplete();
    return CC_MqttsnErrorCode_Success;
}

void SearchOp::handle(AdvertiseMsg& msg)
{
    m_timer.cancel();

    auto info = CC_MqttsnGatewayInfo();
    info.m_gwId = msg.field_gwId().value();
    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &info);
}

void SearchOp::handle(GwinfoMsg& msg)
{
    m_timer.cancel();

    auto info = CC_MqttsnGatewayInfo();
    info.m_gwId = msg.field_gwId().value();
    auto& addr = msg.field_gwAdd().value();
    if (!addr.empty()) {
        info.m_addr = addr.data();
        comms::cast_assign(info.m_addrLen) = addr.size();
    }

    completeOpInternal(CC_MqttsnAsyncOpStatus_Complete, &info);
}

Op::Type SearchOp::typeImpl() const
{
    return Type_Search;
}

void SearchOp::terminateOpImpl(CC_MqttsnAsyncOpStatus status)
{
    completeOpInternal(status);
}

void SearchOp::completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info)
{
    auto cb = m_cb;
    auto* cbData = m_cbData;
    opComplete(); // mustn't access data members after destruction
    if (cb != nullptr) {
        cb(cbData, status, info);    
    }
}

void SearchOp::restartTimer()
{
    m_timer.wait(getRetryPeriod(), &SearchOp::opTimeoutCb, this);
}

CC_MqttsnErrorCode SearchOp::sendInternal()
{
    auto ec = sendMessage(m_searchgwMsg, m_radius);
    if (ec == CC_MqttsnErrorCode_Success) {
        COMMS_ASSERT(0U < getRetryCount());
        decRetryCount();
        restartTimer();
    }
    return ec;    
}

void SearchOp::timeoutInternal()
{
    if (getRetryCount() == 0U) {
        errorLog("All retries of the search operation have been exhausted.");
        completeOpInternal(CC_MqttsnAsyncOpStatus_Timeout);
        return;
    }

    auto ec = sendInternal();
    if (ec != CC_MqttsnErrorCode_Success) {
        completeOpInternal(translateErrorCodeToAsyncOpStatus(ec));
        return;
    }      
}

void SearchOp::opTimeoutCb(void* data)
{
    asSearchOp(data)->sendInternal();
}

} // namespace op

} // namespace cc_mqttsn_client
