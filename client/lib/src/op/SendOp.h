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

class SendOp final : public Op
{
    using Base = Op;
public:

    explicit SendOp(ClientImpl& client);
    virtual ~SendOp();

    CC_MqttsnErrorCode config(const CC_MqttsnPublishConfig* config);
    CC_MqttsnErrorCode send(CC_MqttsnPublishCompleteCb cb, void* cbData);
    CC_MqttsnErrorCode cancel();
    void proceedWithReg();

    void suspend()
    {
        m_suspended = true;
    }

    void resume();

    using Base::handle;
    void handle(RegackMsg& msg) override;
#if CC_MQTTSN_CLIENT_MAX_QOS > 0    
    void handle(PubackMsg& msg) override;
    void handle(PubrecMsg& msg) override;
    void handle(PubcompMsg& msg) override;
#endif // #if CC_MQTTSN_CLIENT_MAX_QOS > 0    

protected:
    virtual Type typeImpl() const override;    
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status) override;

private:
    enum Stage
    {
        Stage_Register,
        Stage_Publish,
        Stage_Acked,
        Stage_ValuesLimit
    };

    void completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info = nullptr);
    void restartTimer();
    CC_MqttsnErrorCode sendInternal();
    CC_MqttsnErrorCode sendInternal_Register();
    CC_MqttsnErrorCode sendInternal_Publish();
    CC_MqttsnErrorCode sendInternal_Pubrel();
    void timeoutInternal();

    static void opTimeoutCb(void* data);

    RegisterMsg m_registerMsg;
    PublishMsg m_publishMsg;
    TimerMgr::Timer m_timer;  
    CC_MqttsnPublishCompleteCb m_cb = nullptr;
    void* m_cbData = nullptr;
    Stage m_stage = Stage_Register;
    unsigned m_origRetryCount = 0U;
    bool m_suspended = false;

    static_assert(ExtConfig::SendOpTimers == 1U);
};

} // namespace op


} // namespace cc_mqttsn_client
