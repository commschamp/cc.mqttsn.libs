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

enum MqttsnConnectStatus
{
    MqttsnReturnCode_Connected,
    MqttsnReturnCode_Denied,
    MqttsnReturnCode_Conjestion,
    MqttsnReturnCode_Timeout,
};

enum MqttsnErrorCode
{
    MqttsnErrorCode_Success,
    MqttsnErrorCode_InvalidOperation,
    MqttsnErrorCode_Busy
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
typedef void (*NextTickProgramFn)(void* data, unsigned duration);
typedef unsigned (*CancelNextTickWaitFn)(void* data);
typedef void (*SendOutputDataFn)(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);
typedef void (*NewGwReportFn)(void* data, unsigned short gwId);
typedef void (*ConnectStatusReportFn)(void* data, MqttsnConnectStatus status);

#ifdef __cplusplus
}
#endif
