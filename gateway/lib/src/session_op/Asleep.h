//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "SessionOp.h"
#include "common.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

class Asleep final : public SessionOp
{
    typedef SessionOp Base;

public:
    explicit Asleep(SessionImpl& session);
    ~Asleep();

protected:
    virtual void tickImpl() override;
    virtual void brokerConnectionUpdatedImpl() override;

private:
    using Base::handle;
    virtual void handle(DisconnectMsg_SN& msg) override;
    virtual void handle(MqttsnMessage& msg) override;
    virtual void handle(PingrespMsg& msg) override;
    virtual void handle(MqttMessage& msg) override;

    void doPing();
    void reqNextTick();

    unsigned m_attempt = 0;
    Timestamp m_lastReq = 0;
    Timestamp m_lastResp = 0;
};

}  // namespace session_op

}  // namespace cc_mqttsn_gateway


