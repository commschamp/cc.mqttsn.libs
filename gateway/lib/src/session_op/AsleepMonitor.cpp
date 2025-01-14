//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "AsleepMonitor.h"

#include <cassert>
#include <algorithm>

namespace cc_mqttsn_gateway
{

namespace session_op    
{

AsleepMonitor::AsleepMonitor(SessionImpl& session) :
    Base(session)
{
}

AsleepMonitor::~AsleepMonitor() = default;

void AsleepMonitor::tickImpl()
{
    if (state().m_connStatus == ConnectionStatus::Asleep) {
        sendToBroker(DisconnectMsg());
        termRequest();
    }
}

void AsleepMonitor::handle(DisconnectMsg_SN& msg)
{
    if (!msg.field_duration().doesExist()) {
        // Monitor disconnect with duration in Disconnect op
        return;
    }

    if (state().m_connStatus != ConnectionStatus::Asleep) {
        [[maybe_unused]] constexpr bool Should_not_happen = false;
        assert(Should_not_happen);
        return;
    }

    m_lastPing = state().m_timestamp;
    m_duration = ((static_cast<unsigned>(msg.field_duration().field().value()) * 3000) / 2);
    reqNextTickInternal();
}

void AsleepMonitor::handle([[maybe_unused]] PingreqMsg_SN& msg)
{
    m_lastPing = state().m_timestamp;
    cancelTick();
    if (state().m_connStatus == ConnectionStatus::Asleep) {
        reqNextTickInternal();
    }
}

void AsleepMonitor::handle([[maybe_unused]] MqttsnMessage& msg)
{
    checkTickRequired();
}

void AsleepMonitor::handle([[maybe_unused]] MqttMessage& msg)
{
    checkTickRequired();
}

void AsleepMonitor::checkTickRequired()
{
    if (state().m_connStatus != ConnectionStatus::Asleep) {
        cancelTick();
    }
}

void AsleepMonitor::reqNextTickInternal()
{
    assert(0 < m_lastPing);

    auto& st = state();
    auto nextTimestamp = m_lastPing + m_duration;
    if (nextTimestamp <= st.m_timestamp) {
        nextTickReq(1U);
        return;
    }

    nextTickReq(static_cast<unsigned>(nextTimestamp - st.m_timestamp));
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
