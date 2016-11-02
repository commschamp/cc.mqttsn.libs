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

#include "Sub.h"

#include <cassert>

namespace mqttsn
{

namespace client
{

namespace app
{

namespace sub
{

namespace udp
{

Sub::Sub()
  : m_client(mqttsn_client_new())
{
    mqttsn_client_set_next_tick_program_callback(
        m_client.get(), &Sub::nextTickProgramCb, this);

    mqttsn_client_set_cancel_next_tick_wait_callback(
        m_client.get(), &Sub::caneclTickCb, this);

    mqttsn_client_set_send_output_data_callback(
        m_client.get(), &Sub::sendDataCb, this);

    mqttsn_client_set_gw_status_report_callback(
        m_client.get(), &Sub::gwStatusReportCb, this);

    mqttsn_client_set_connection_status_report_callback(
        m_client.get(), &Sub::connectionStatusReportCb, this);

    mqttsn_client_set_message_report_callback(
        m_client.get(), &Sub::messageReportCb, this);


    connect(
        &m_timer, SIGNAL(timeout()),
        this, SLOT(tick()));
}

void Sub::tick()
{
    auto ms = m_reqTimeout;
    m_reqTimeout = 0;
    mqttsn_client_tick(m_client.get(), ms);
}

void Sub::nextTickProgram(unsigned ms)
{
    m_reqTimeout = ms;
    m_timer.setSingleShot(true);
    m_timer.start(ms);
}

void Sub::nextTickProgramCb(void* obj, unsigned ms)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->nextTickProgram(ms);
}

unsigned Sub::cancelTick()
{
    auto rem = m_timer.remainingTime();
    m_timer.stop();

    if (m_reqTimeout < static_cast<unsigned>(rem)) {
        rem = static_cast<decltype(rem)>(m_reqTimeout);
    }

    return m_reqTimeout - static_cast<unsigned>(rem);
}

unsigned Sub::caneclTickCb(void* obj)
{
    assert(obj != nullptr);
    return reinterpret_cast<Sub*>(obj)->cancelTick();
}

void Sub::sendData(const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    static_cast<void>(buf);
    static_cast<void>(bufLen);
    static_cast<void>(broadcast);
    assert(!"NYI");
    // TODO
}

void Sub::sendDataCb(void* obj, const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->sendData(buf, bufLen, broadcast);
}

void Sub::gwStatusReport(unsigned short gwId, MqttsnGwStatus status)
{
    static_cast<void>(gwId);
    static_cast<void>(status);
    assert(!"NYI");
    // TODO
}

void Sub::gwStatusReportCb(void* obj, unsigned short gwId, MqttsnGwStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->gwStatusReport(gwId, status);
}

void Sub::connectionStatusReport(MqttsnConnectionStatus status)
{
    static_cast<void>(status);
    assert(!"NYI");
    // TODO
}

void Sub::connectionStatusReportCb(void* obj, MqttsnConnectionStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->connectionStatusReport(status);
}

void Sub::messageReport(const MqttsnMessageInfo* msgInfo)
{
    assert(msgInfo != nullptr);
    static_cast<void>(msgInfo);
    assert(!"NYI");
    // TODO
}

void Sub::messageReportCb(void* obj, const MqttsnMessageInfo* msgInfo)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->messageReport(msgInfo);
}

}  // namespace udp

}  // namespace sub

}  // namespace app

}  // namespace client

}  // namespace mqttsn


