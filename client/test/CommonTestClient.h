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


#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <cstdint>

#include "mqttsn/client/common.h"
#include "mqttsn/protocol/field.h"
#include "client.h"

typedef decltype(&mqttsn_client_new) ClientNewFunc;
typedef decltype(&mqttsn_client_free) ClientFreeFunc;
typedef decltype(&mqttsn_client_set_next_tick_program_callback) NextTickProgramCallbackSetFunc;
typedef decltype(&mqttsn_client_set_cancel_next_tick_wait_callback) CancelNextTickCallbackSetFunc;
typedef decltype(&mqttsn_client_set_send_output_data_callback) SendOutDataCallbackSetFunc;
typedef decltype(&mqttsn_client_set_gw_status_report_callback) GwStatusReportCallbackSetFunc;
typedef decltype(&mqttsn_client_start) StartFunc;
typedef decltype(&mqttsn_client_process_data) ProcessDataFunc;
typedef decltype(&mqttsn_client_tick) TickFunc;
typedef decltype(&mqttsn_client_set_gw_advertise_period) SetGwAdvertisePeriodFunc;
typedef decltype(&mqttsn_client_set_retry_period) SetRetryPeriodFunc;
typedef decltype(&mqttsn_client_set_retry_count) SetRetryCountFunc;
typedef decltype(&mqttsn_client_set_broadcast_radius) SetBroadcastRadiusFunc;
typedef decltype(&mqttsn_client_connect) ConnectFunc;


struct ClientLibFuncs
{
    ClientNewFunc m_newFunc = nullptr;
    ClientFreeFunc m_freeFunc = nullptr;
    NextTickProgramCallbackSetFunc m_nextTickProgramCallbackSetFunc = nullptr;
    CancelNextTickCallbackSetFunc m_cancelNextTickCallbackSetFunc = nullptr;
    SendOutDataCallbackSetFunc m_sentOutDataCallbackSetFunc = nullptr;
    GwStatusReportCallbackSetFunc m_gwStatusReportCallbackSetFunc = nullptr;
    StartFunc m_startFunc = nullptr;
    ProcessDataFunc m_processDataFunc = nullptr;
    TickFunc m_tickFunc = nullptr;
    SetGwAdvertisePeriodFunc m_setGwAdvertisePeriodFunc = nullptr;
    SetRetryPeriodFunc m_setRetryPeriodFunc = nullptr;
    SetRetryCountFunc m_setRetryCountFunc = nullptr;
    SetBroadcastRadiusFunc m_setBroadcastRadius = nullptr;
    ConnectFunc m_connectFunc = nullptr;
};

class CommonTestClient
{
public:
    typedef std::unique_ptr<CommonTestClient> Ptr;
    typedef std::function<void (unsigned)> ProgramNextTickCallback;
    typedef std::function<unsigned ()> CancelNextTickCallback;
    typedef std::function<void (const std::uint8_t* buf, unsigned bufLen, bool broadcast)> SendDataCallback;
    typedef std::function<void (unsigned short gwId, MqttsnGwStatus status)> GwStatusReportCallback;

    ~CommonTestClient();

    void setProgramNextTickCallback(ProgramNextTickCallback&& func);
    void setCancelNextTickCallback(CancelNextTickCallback&& func);
    void setSendDataCallback(SendDataCallback&& func);
    void setGwStatusReportCallback(GwStatusReportCallback&& func);

    static Ptr alloc(const ClientLibFuncs& libFuncs = DefaultFuncs);
    bool start();
    void inputData(const std::uint8_t* buf, std::size_t bufLen);
    void tick(unsigned ms);
    void setGwAdvertisePeriod(unsigned ms);
    void setRetryPeriod(unsigned ms);
    void setRetryCount(unsigned value);
    void setBroadcastRadius(unsigned char val);

    typedef std::function<void (MqttsnConnectStatus)> ConnectStatusReportCallback;
    MqttsnErrorCode connect(
        const char* clientId,
        unsigned short keepAliveSeconds,
        bool cleanSession,
        const MqttsnWillInfo* willInfo,
        ConnectStatusReportCallback&& cb);

    static MqttsnQoS transformQos(mqttsn::protocol::field::QosType val);

private:
    typedef std::vector<std::uint8_t> InputData;

    CommonTestClient(const ClientLibFuncs& libFuncs);

    void programNextTick(unsigned duration);
    unsigned cancelNextTick();
    void sendOutputData(const unsigned char* buf, unsigned bufLen, bool broadcast);
    void reportGwStatus(unsigned short gwId, MqttsnGwStatus status);
    void reportConnectStatus(MqttsnConnectStatus status);

    static void nextTickProgramCallback(void* data, unsigned duration);
    static unsigned cancelNextTickCallback(void* data);
    static void sendOutputDataCallback(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);
    static void gwStatusReportCallback(void* data, unsigned short gwId, MqttsnGwStatus status);
    static void connectStatusCallback(void* data, MqttsnConnectStatus status);

    ClientLibFuncs m_libFuncs;
    ClientHandle m_client = nullptr;
    InputData m_inData;

    ProgramNextTickCallback m_programNextTickCallback;
    CancelNextTickCallback m_cancelNextTickCallback;
    SendDataCallback m_sendDataCallback;
    GwStatusReportCallback m_gwStatusReportCallback;
    ConnectStatusReportCallback m_connectStatusReportCallback;

    static const ClientLibFuncs DefaultFuncs;
};
