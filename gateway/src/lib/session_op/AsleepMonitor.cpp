//
// Copyright 2016 - 2017 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "AsleepMonitor.h"

#include <cassert>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

AsleepMonitor::AsleepMonitor(SessionState& sessionState)
  : Base(sessionState)
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
        assert(!"Something is not right");
        return;
    }

    m_lastPing = state().m_timestamp;
    m_duration = ((static_cast<unsigned>(msg.field_duration().field().value()) * 3000) / 2);
    reqNextTick();
}

void AsleepMonitor::handle(PingreqMsg_SN& msg)
{
    static_cast<void>(msg);
    m_lastPing = state().m_timestamp;
    cancelTick();
    if (state().m_connStatus == ConnectionStatus::Asleep) {
        reqNextTick();
    }
}

void AsleepMonitor::handle(MqttsnMessage& msg)
{
    static_cast<void>(msg);
    checkTickRequired();
}

void AsleepMonitor::handle(MqttMessage& msg)
{
    static_cast<void>(msg);
    checkTickRequired();
}

void AsleepMonitor::checkTickRequired()
{
    if (state().m_connStatus != ConnectionStatus::Asleep) {
        cancelTick();
    }
}

void AsleepMonitor::reqNextTick()
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

}  // namespace gateway

}  // namespace mqttsn



