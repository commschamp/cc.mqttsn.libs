//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "op/Op.h"
#include "ExtConfig.h"
#include "ProtocolDefs.h"

#include "TimerMgr.h"

namespace cc_mqttsn_client
{

namespace op
{

class SubscribeOp final : public Op
{
    using Base = Op;
public:
    explicit SubscribeOp(ClientImpl& client);
    virtual ~SubscribeOp();

    CC_MqttsnErrorCode config(const CC_MqttsnSubscribeConfig* config);
    CC_MqttsnErrorCode send(CC_MqttsnSubscribeCompleteCb cb, void* cbData);
    CC_MqttsnErrorCode cancel();

    using Base::handle;
    void handle(SubackMsg& msg) override;

    void suspend()
    {
        m_suspended = true;
    }

    void resume();

protected:
    virtual Type typeImpl() const override;    
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status) override;

private:
    void completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info = nullptr);
    void restartTimer();
    CC_MqttsnErrorCode sendInternal();
    void timeoutInternal();

    static void opTimeoutCb(void* data);

    SubscribeMsg m_subscribeMsg;
    TimerMgr::Timer m_timer;  
    CC_MqttsnSubscribeCompleteCb m_cb = nullptr;
    void* m_cbData = nullptr;
    bool m_suspended = false;

    static_assert(ExtConfig::SubscribeOpTimers == 1U);
};

} // namespace op


} // namespace cc_mqttsn_client
