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

class DisconnectOp final : public Op
{
    using Base = Op;
public:
    explicit DisconnectOp(ClientImpl& client);

    CC_MqttsnErrorCode send(CC_MqttsnDisconnectCompleteCb cb, void* cbData);
    CC_MqttsnErrorCode config(const CC_MqttsnSleepConfig* config);
    CC_MqttsnErrorCode cancel();

    using Base::handle;
    void handle(DisconnectMsg& msg) override;

    bool isSleepConfigured() const
    {
        return m_disconnectMsg.field_duration().doesExist();
    }

protected:
    virtual Type typeImpl() const override;    
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status) override;

private:

    void completeOpInternal(CC_MqttsnAsyncOpStatus status);
    void restartTimer();
    CC_MqttsnErrorCode sendInternal();
    void timeoutInternal();

    static void opTimeoutCb(void* data);

    DisconnectMsg m_disconnectMsg;
    TimerMgr::Timer m_timer;  
    CC_MqttsnDisconnectCompleteCb m_cb = nullptr;
    void* m_cbData = nullptr;

    static_assert(ExtConfig::DisconnectOpTimers == 1U);
};

} // namespace op


} // namespace cc_mqttsn_client
