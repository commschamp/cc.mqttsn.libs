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

class SearchOp final : public Op
{
    using Base = Op;
public:
    explicit SearchOp(ClientImpl& client);

    CC_MqttsnErrorCode send(CC_MqttsnSearchCompleteCb cb, void* cbData);
    CC_MqttsnErrorCode cancel();

    using Base::handle;
    virtual void handle(AdvertiseMsg& msg) override;
    virtual void handle(GwinfoMsg& msg) override;

    void setBroadcastRadius(unsigned value)
    {
        m_radius = value;
    }

    unsigned getBroadcastRadius() const
    {
        return m_radius;
    }

protected:
    virtual Type typeImpl() const override;    
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status) override;

private:
    void completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info = nullptr);
    void restartTimer();
    CC_MqttsnErrorCode sendInternal();
    void timeoutInternal();

    static void opTimeoutCb(void* data);

    SearchgwMsg m_searchgwMsg;
    TimerMgr::Timer m_timer;  
    unsigned m_radius = 0U;
    CC_MqttsnSearchCompleteCb m_cb = nullptr;
    void* m_cbData = nullptr;

    static_assert(ExtConfig::SearchOpTimers == 1U);
};

} // namespace op


} // namespace cc_mqttsn_client
