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

class ConnectOp final : public Op
{
    using Base = Op;
public:
    explicit ConnectOp(ClientImpl& client);

    CC_MqttsnErrorCode config(const CC_MqttsnConnectConfig* config);
    CC_MqttsnErrorCode willConfig(const CC_MqttsnWillConfig* config);
    CC_MqttsnErrorCode send(CC_MqttsnConnectCompleteCb cb, void* cbData);
    CC_MqttsnErrorCode cancel();

    using Base::handle;
#if CC_MQTTSN_CLIENT_HAS_WILL    
    void handle(WilltopicreqMsg& msg) override;
    void handle(WillmsgreqMsg& msg) override;
#endif    
    void handle(ConnackMsg& msg) override;

protected:
    virtual Type typeImpl() const override;    
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status) override;

private:
    enum Stage : unsigned
    {
        Stage_connect,
#if CC_MQTTSN_CLIENT_HAS_WILL        
        Stage_willTopic,
        Stage_willMsg,
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL        
        Stage_valuesLimit
    };

    void completeOpInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info = nullptr);
    void restartTimer();
    CC_MqttsnErrorCode sendInternal();
    void timeoutInternal();
    const ProtMessage& getConnectMsg() const;

#if CC_MQTTSN_CLIENT_HAS_WILL
    const ProtMessage& getWilltopicMsg() const;
    const ProtMessage& getWillmsgMsg() const;
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL    

    static void opTimeoutCb(void* data);

    ConnectMsg m_connectMsg;
    TimerMgr::Timer m_timer;  
    unsigned m_stage = Stage_connect;
    unsigned m_origRetryCount = 0U;
    CC_MqttsnConnectCompleteCb m_cb = nullptr;
    void* m_cbData = nullptr;

#if CC_MQTTSN_CLIENT_HAS_WILL      
    WilltopicMsg m_willtopicMsg;
    WillmsgMsg m_willmsgMsg;
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL       

    static_assert(ExtConfig::ConnectOpTimers == 1U);
};

} // namespace op


} // namespace cc_mqttsn_client
