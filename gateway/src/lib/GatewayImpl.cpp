//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayImpl.h"

#include <cassert>
#include <iterator>

namespace cc_mqttsn_gateway
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

}  // namespace cc_mqttsn_gateway
