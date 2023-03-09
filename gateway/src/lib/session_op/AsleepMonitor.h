//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

class AsleepMonitor : public SessionOp
{
    typedef SessionOp Base;

public:
    AsleepMonitor(SessionState& sessionState);
    ~AsleepMonitor();

protected:
    virtual void tickImpl() override;

private:
    using Base::handle;
    virtual void handle(DisconnectMsg_SN& msg) override;
    virtual void handle(PingreqMsg_SN& msg) override;
    virtual void handle(MqttsnMessage& msg) override;
    virtual void handle(MqttMessage& msg) override;

    void checkTickRequired();
    void reqNextTick();

    Timestamp m_lastPing = 0;
    unsigned m_duration = 0U;
};

}  // namespace session_op

}  // namespace cc_mqttsn_gateway


