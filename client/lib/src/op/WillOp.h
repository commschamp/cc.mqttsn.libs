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

#if CC_MQTTSN_CLIENT_HAS_WILL

namespace cc_mqttsn_client
{

namespace op
{

class WillOp final : public Op
{
    using Base = Op;
public:
    explicit WillOp(ClientImpl& client);

    CC_MqttsnErrorCode config(const CC_MqttsnWillConfig* config);
    CC_MqttsnErrorCode send(CC_MqttsnWillCompleteCb cb, void* cbData);
    CC_MqttsnErrorCode cancel();

    using Base::handle;
    void handle(WilltopicrespMsg& msg) override;
    void handle(WillmsgrespMsg& msg) override;

protected:
    virtual Type typeImpl() const override;    
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status) override;

private:
    enum Stage : unsigned
    {
        Stage_willTopic,
        Stage_willMsg,
        Stage_valuesLimit
    };

    void completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnWillInfo* info = nullptr);
    void restartTimer();
    CC_MqttsnErrorCode sendInternal();
    void timeoutInternal();
    const ProtMessage& getWilltopicupdMsg() const;
    const ProtMessage& getWillmsgupdMsg() const;

    static void opTimeoutCb(void* data);

    WilltopicupdMsg m_willtopicupdMsg;
    WillmsgupdMsg m_willmsgupdMsg;
    TimerMgr::Timer m_timer;  
    unsigned m_stage = Stage_willTopic;
    unsigned m_origRetryCount = 0U;
    CC_MqttsnWillCompleteCb m_cb = nullptr;
    void* m_cbData = nullptr;
    CC_MqttsnWillInfo m_info;

    static_assert(ExtConfig::WillOpTimers == 1U);
};

} // namespace op

} // namespace cc_mqttsn_client

#endif // #if CC_MQTTSN_CLIENT_HAS_WILL        
