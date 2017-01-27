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

#include "GatewayImpl.h"

#include <cassert>
#include <iterator>

namespace mqttsn
{

namespace gateway
{

bool GatewayImpl::start()
{
    if ((m_running) ||
        (!m_tickProgramCb) ||
        (!m_sendDataCb) ||
        (m_advertisePeriod == 0U)) {
        return false;
    }

    m_running = true;
    refresh();
    sendAndReprogram();
    return true;
}

void GatewayImpl::stop()
{
    m_running = false;
}

void GatewayImpl::tick()
{
    if (!m_running) {
        return;
    }

    sendAndReprogram();
}

void GatewayImpl::refresh()
{
    if (!m_running) {
        return;
    }

    AdvertiseMsg msg;
    auto& fields = msg.fields();
    auto& gwIdField = std::get<decltype(msg)::FieldIdx_gwId>(fields);
    auto& durationField = std::get<decltype(msg)::FieldIdx_duration>(fields);

    assert(m_advertisePeriod != 0U);
    gwIdField.value() = m_gwId;
    durationField.value() = m_advertisePeriod;

    auto iter = &m_outputData[0];
    auto es = m_protStack.write(msg, iter, m_outputData.size());
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    assert(std::distance(&m_outputData[0], iter) == AdvertiseLength);
}

void GatewayImpl::sendAndReprogram()
{
    assert(m_tickProgramCb);
    assert(m_sendDataCb);

    m_sendDataCb(&m_outputData[0], m_outputData.size());
    m_tickProgramCb(m_advertisePeriod * 1000U);
}

}  // namespace gateway

}  // namespace mqttsn


