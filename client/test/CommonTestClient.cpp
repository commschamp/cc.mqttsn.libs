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

#include "CommonTestClient.h"

#include <cassert>

namespace
{

ClientLibFuncs createDefaultLibFuncs()
{
    ClientLibFuncs funcs;
    funcs.m_newFunc = &mqttsn_client_new;
    funcs.m_freeFunc = &mqttsn_client_free;
    funcs.m_nextTickProgramCallbackSetFunc = &mqttsn_client_set_next_tick_program_callback;
    funcs.m_cancelNextTickCallbackSetFunc = &mqttsn_client_set_cancel_next_tick_wait_callback;
    funcs.m_sentOutDataCallbackSetFunc = &mqttsn_client_set_send_output_data_callback;
    funcs.m_gwStatusReportCallbackSetFunc = &mqttsn_client_set_gw_status_report_callback;
    funcs.m_startFunc = &mqttsn_client_start;
    funcs.m_processDataFunc = &mqttsn_client_process_data;
    funcs.m_tickFunc = &mqttsn_client_tick;
    funcs.m_setGwAdvertisePeriodFunc = &mqttsn_client_set_gw_advertise_period;
    funcs.m_setRetryPeriodFunc = &mqttsn_client_set_retry_period;
    funcs.m_setBroadcastRadius = &mqttsn_client_set_broadcast_radius;
    return funcs;
}

}  // namespace

const ClientLibFuncs CommonTestClient::DefaultFuncs = createDefaultLibFuncs();

CommonTestClient::~CommonTestClient()
{
    assert(m_libFuncs.m_freeFunc != nullptr);
    m_libFuncs.m_freeFunc(m_client);
}

void CommonTestClient::setProgramNextTickCallback(ProgramNextTickCallback&& func)
{
    m_programNextTickCallback = std::move(func);
}

void CommonTestClient::setCancelNextTickCallback(CancelNextTickCallback&& func)
{
    m_cancelNextTickCallback = std::move(func);
}

void CommonTestClient::setSendDataCallback(SendDataCallback&& func)
{
    m_sendDataCallback = std::move(func);
}

void CommonTestClient::setGwStatusReportCallback(GwStatusReportCallback&& func)
{
    m_gwStatusReportCallback = std::move(func);
}

CommonTestClient::Ptr CommonTestClient::alloc(const ClientLibFuncs& libFuncs)
{
    return Ptr(new CommonTestClient(libFuncs));
}

bool CommonTestClient::start()
{
    assert(m_libFuncs.m_startFunc != nullptr);
    return (m_libFuncs.m_startFunc)(m_client);
}

void CommonTestClient::inputData(const std::uint8_t* buf, std::size_t bufLen)
{
    m_inData.insert(m_inData.end(), buf, buf + bufLen);
    assert(m_libFuncs.m_processDataFunc != nullptr);
    assert(!m_inData.empty());
    unsigned count = (m_libFuncs.m_processDataFunc)(m_client, &m_inData[0], m_inData.size());
    assert(count <= m_inData.size());
    m_inData.erase(m_inData.begin(), m_inData.begin() + count);
}

void CommonTestClient::tick(unsigned ms)
{
    assert(m_libFuncs.m_tickFunc != nullptr);
    (m_libFuncs.m_tickFunc)(m_client, ms);
}

void CommonTestClient::setGwAdvertisePeriod(unsigned ms)
{
    assert(m_libFuncs.m_setGwAdvertisePeriodFunc != nullptr);
    (m_libFuncs.m_setGwAdvertisePeriodFunc)(m_client, ms);
}

void CommonTestClient::setRetryPeriod(unsigned ms)
{
    assert(m_libFuncs.m_setRetryPeriodFunc != nullptr);
    (m_libFuncs.m_setRetryPeriodFunc)(m_client, ms);
}

void CommonTestClient::setBroadcastRadius(unsigned char val)
{
    assert(m_libFuncs.m_setBroadcastRadius != nullptr);
    (m_libFuncs.m_setBroadcastRadius)(m_client, val);
}

CommonTestClient::CommonTestClient(const ClientLibFuncs& libFuncs)
  : m_libFuncs(libFuncs),
    m_client((libFuncs.m_newFunc)())
{
    assert(m_libFuncs.m_nextTickProgramCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_cancelNextTickCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_sentOutDataCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_gwStatusReportCallbackSetFunc != nullptr);

    (m_libFuncs.m_nextTickProgramCallbackSetFunc)(m_client, &CommonTestClient::nextTickProgramCallback, this);
    (m_libFuncs.m_cancelNextTickCallbackSetFunc)(m_client, &CommonTestClient::cancelNextTickCallback, this);
    (m_libFuncs.m_sentOutDataCallbackSetFunc)(m_client, &CommonTestClient::sendOutputDataCallback, this);
    (m_libFuncs.m_gwStatusReportCallbackSetFunc)(m_client, &CommonTestClient::gwStatusReportCallback, this);
    // TODO: callbacks
}

void CommonTestClient::programNextTick(unsigned duration)
{
    if (m_programNextTickCallback) {
        m_programNextTickCallback(duration);
    }
}

unsigned CommonTestClient::cancelNextTick()
{
    if (m_cancelNextTickCallback) {
        return m_cancelNextTickCallback();
    }

    assert(!"Should not happen");
    return 0;
}

void CommonTestClient::sendOutputData(const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    if (m_sendDataCallback) {
        m_sendDataCallback(buf, bufLen, broadcast);
    }
}

void CommonTestClient::reportGwStatus(unsigned short gwId, MqttsnGwStatus status)
{
    if (m_gwStatusReportCallback) {
        m_gwStatusReportCallback(gwId, status);
    }
}

void CommonTestClient::nextTickProgramCallback(void* data, unsigned duration)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->programNextTick(duration);
}

unsigned CommonTestClient::cancelNextTickCallback(void* data)
{
    assert(data != nullptr);
    return reinterpret_cast<CommonTestClient*>(data)->cancelNextTick();
}

void CommonTestClient::sendOutputDataCallback(
    void* data,
    const unsigned char* buf,
    unsigned bufLen,
    bool broadcast)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->sendOutputData(buf, bufLen, broadcast);
}

void CommonTestClient::gwStatusReportCallback(
    void* data,
    unsigned short gwId,
    MqttsnGwStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportGwStatus(gwId, status);
}
