//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
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

class UnsubscribeOp final : public Op
{
    using Base = Op;
public:
    explicit UnsubscribeOp(ClientImpl& client);
    virtual ~UnsubscribeOp();

    CC_MqttsnErrorCode config(const CC_MqttsnUnsubscribeConfig* config);
    CC_MqttsnErrorCode send(CC_MqttsnUnsubscribeCompleteCb cb, void* cbData);
    CC_MqttsnErrorCode cancel();

    using Base::handle;
    void handle(UnsubackMsg& msg) override;

    void suspend()
    {
        m_suspended = true;
    }

    void resume();

protected:
    virtual Type typeImpl() const override;    
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status) override;

private:
    void completeOpInternal(CC_MqttsnAsyncOpStatus status);
    void restartTimer();
    CC_MqttsnErrorCode sendInternal();
    void timeoutInternal();

    static void opTimeoutCb(void* data);

    UnsubscribeMsg m_unsubscribeMsg;
    TimerMgr::Timer m_timer;  
    CC_MqttsnUnsubscribeCompleteCb m_cb = nullptr;
    void* m_cbData = nullptr;
    bool m_suspended = false;

    static_assert(ExtConfig::UnsubscribeOpTimers == 1U);
};

} // namespace op


} // namespace cc_mqttsn_client
