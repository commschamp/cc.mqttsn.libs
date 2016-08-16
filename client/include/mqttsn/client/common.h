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

#ifdef __cplusplus
extern "C" {
#else

#ifdef WIN32
#define bool char
#define true 1
#define false 0

#else // #ifdef WIN32
#include <stdbool.h>
#endif // #ifdef WIN32

#endif // #ifdef __cplusplus

enum MqttsnQoS
{
    MqttsnQoS_NoGwPublish = -1,
    MqttsnQoS_AtMostOnceDelivery,
    MqttsnQoS_AtLeastOnceDelivery,
    MqttsnQoS_ExactlyOnceDelivery
};

enum MqttsnConnectionStatus
{
    MqttsnConnectionStatus_Invalid,
    MqttsnConnectionStatus_Connected,
    MqttsnConnectionStatus_Denied,
    MqttsnConnectionStatus_Conjestion,
    MqttsnConnectionStatus_Timeout,
    MqttsnConnectionStatus_Disconnected,
    MqttsnConnectionStatus_DisconnectedAsleep,
    MqttsnConnectionStatus_ConnectAborted,
    MqttsnConnectionStatus_NumOfValues
};

enum MqttsnErrorCode
{
    MqttsnErrorCode_Success,
    MqttsnErrorCode_InvalidOperation,
    MqttsnErrorCode_Busy,
    MqttsnErrorCode_NotConnected,
    MqttsnErrorCode_BadParam,
};

enum MqttsnGwStatus
{
    MqttsnGwStatus_TimedOut,
    MqttsnGwStatus_Available
};

enum MqttsnAsyncOpStatus
{
    MqttsnAsyncOpStatus_Invalid,
    MqttsnAsyncOpStatus_Successful,
    MqttsnAsyncOpStatus_Conjestion,
    MqttsnAsyncOpStatus_InvalidId,
    MqttsnAsyncOpStatus_NotSupported,
    MqttsnAsyncOpStatus_NoResponse,
    MqttsnAsyncOpStatus_Aborted,
};

struct MqttsnWillInfo
{
    const char* topic;
    const unsigned char* msg;
    unsigned msgLen;
    MqttsnQoS qos;
    bool retain;
};

typedef void* ClientHandle;
typedef unsigned short MqttsnTopicId;

typedef void (*NextTickProgramFn)(void* data, unsigned duration);
typedef unsigned (*CancelNextTickWaitFn)(void* data);
typedef void (*SendOutputDataFn)(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);
typedef void (*GwStatusReportFn)(void* data, unsigned short gwId, MqttsnGwStatus status);
typedef void (*ConnectionStatusReportFn)(void* data, MqttsnConnectionStatus status);
typedef void (*PublishCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);

#ifdef __cplusplus
}
#endif
