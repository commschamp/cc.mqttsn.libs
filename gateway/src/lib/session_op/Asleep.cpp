//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
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

#include "Asleep.h"

#include <cassert>
#include <algorithm>

namespace mqttsn
{

namespace gateway
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

void Asleep::handle(DisconnectMsg_SN& msg)
{
    typedef DisconnectMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& durationField = std::get<MsgType::FieldIdx_duration>(fields);

    if (durationField.getMode() != comms::field::OptionalMode::Exists) {
        // Monotor disconnect with duration in Disconnect op
        return;
    }

    DisconnectMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& respDurationField = std::get<decltype(respMsg)::FieldIdx_duration>(respFields);
    respDurationField.setMode(comms::field::OptionalMode::Missing);
    sendToClient(respMsg);

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

}  // namespace gateway

}  // namespace mqttsn



