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

    state().m_connStatus = ConnectionStatus::Asleep;
    m_attempt = 0;
    doPing();
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
    unsigned period = state().m_retryPeriod;
    if ((m_lastReq <= m_lastResp) && (0 < m_lastReq)) {
        auto nextKeepAlivePeriod =
            static_cast<unsigned>(
                (m_lastReq + (state().m_keepAlive * 1000)) - state().m_timestamp);
        period = std::min(nextKeepAlivePeriod, period);
    }

    if ((m_lastResp < m_lastReq) || (m_lastResp == 0)) {
        nextTickReq(period);
    }
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



