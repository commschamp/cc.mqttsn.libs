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

class KeepAliveOp final : public Op
{
    using Base = Op;
public:
    explicit KeepAliveOp(ClientImpl& client);

    void forceSendPing();
    void messageSent();

    using Base::handle;
    virtual void handle(PingreqMsg& msg) override;
    virtual void handle(PingrespMsg& msg) override;
    virtual void handle(ProtMessage& msg) override;

protected:
    virtual Type typeImpl() const override;    

private:
    void restartPingTimer();
    void restartRecvTimer();
    void restartRespTimer();
    void sendPing();
    void pingTimeoutInternal();

    static void sendPingCb(void* data);
    static void recvTimeoutCb(void* data);
    static void pingTimeoutCb(void* data);

    TimerMgr::Timer m_pingTimer;
    TimerMgr::Timer m_recvTimer;  
    TimerMgr::Timer m_respTimer;  

    static_assert(ExtConfig::KeepAliveOpTimers == 3U);
};

} // namespace op


} // namespace cc_mqttsn_client
