//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Ping.h"

#include <cassert>

namespace cc_mqttsn_gateway
{

namespace session_op    
{

Ping::Ping(SessionImpl& session) :
    Base(session)
{
}

Ping::~Ping() = default;

void Ping::tickImpl()
{
    sendPing();
}

void Ping::connStatusUpdatedImpl()
{
    restartPingTimer();
}

void Ping::handle([[maybe_unused]] MqttsnMessage& msg)
{
    restartPingTimer();
}

void Ping::sendPing()
{
    auto& st = state();

    if (st.m_retryCount <= m_attempt) {
        termRequest();
        return;
    }

    ++m_attempt;
    sendToClient(PingreqMsg_SN());
    nextTickReq(st.m_retryPeriod);
}

void Ping::restartPingTimer()
{
    auto& st = state();
    if (st.m_connStatus != ConnectionStatus::Connected) {
        cancelTick();
        return;
    }

    m_attempt = 0U;
    assert(0U < st.m_keepAlive);
    auto nextPingTick = (st.m_keepAlive * 1000);
    nextTickReq(nextPingTick);
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
