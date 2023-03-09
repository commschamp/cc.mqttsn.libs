//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Asleep.h"

#include <cassert>
#include <algorithm>

namespace cc_mqttsn_gateway
{

namespace session_op
{

Asleep::Asleep(SessionState& sessionState)
  : Base(sessionState)
{
}

Asleep::~Asleep() = default;

void Asleep::tickImpl()
{
    auto& st = state();
    if ((st.m_pendingClientDisconnect) ||
        (st.m_connStatus != ConnectionStatus::Asleep)) {
        return;
    }

    doPing();
}

void Asleep::brokerConnectionUpdatedImpl()
{
    auto& st = state();
    if ((!st.m_brokerConnected) &&
        (!st.m_reconnectingBroker)) {
        cancelTick();
    }
}

void Asleep::handle(DisconnectMsg_SN& msg)
{
    if (msg.field_duration().isMissing()) {
        // Monitor disconnect with duration in Disconnect op
        return;
    }

    sendDisconnectToClient();
    state().m_connStatus = ConnectionStatus::Asleep;
    m_attempt = 0;
    doPing();
}

void Asleep::handle(MqttsnMessage& msg)
{
    static_cast<void>(msg);
    if (state().m_connStatus != ConnectionStatus::Asleep) {
        cancelTick();
    }
}

void Asleep::handle(PingrespMsg& msg)
{
    static_cast<void>(msg);
    auto& st = state();
    if ((st.m_pendingClientDisconnect) ||
        (st.m_connStatus != ConnectionStatus::Asleep)) {
        return;
    }

    m_lastResp = st.m_timestamp;
    m_attempt = 0;
    reqNextTick();
}

void Asleep::handle(MqttMessage& msg)
{
    static_cast<void>(msg);
    if (state().m_connStatus != ConnectionStatus::Asleep) {
        cancelTick();
    }
}

void Asleep::doPing()
{
    auto& st = state();
    if (st.m_retryCount <= m_attempt) {
        st.m_pendingClientDisconnect = true;
        return;
    }

    sendToBroker(PingreqMsg());
    m_lastReq = st.m_timestamp;
    reqNextTick();
}

void Asleep::reqNextTick()
{
    assert(0 < m_lastReq);

    auto& st = state();
    auto nextRetryTimestamp = std::numeric_limits<Timestamp>::max();
    if (m_lastResp < m_lastReq) {
        nextRetryTimestamp = (m_lastReq + st.m_retryPeriod);
        if (nextRetryTimestamp <= st.m_timestamp) {
            nextTickReq(1U);
            return;
        }
    }

    auto nextKeepAliveTimestamp = (m_lastReq + (st.m_keepAlive * 1000));
    if (nextKeepAliveTimestamp <= st.m_timestamp) {
        nextTickReq(1U);
        return;
    }

    auto nextTickTimestamp = std::min(nextRetryTimestamp, nextKeepAliveTimestamp);
    assert(st.m_timestamp < nextTickTimestamp);
    nextTickReq(static_cast<unsigned>(nextTickTimestamp - st.m_timestamp));
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
