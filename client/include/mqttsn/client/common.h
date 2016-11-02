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
    MqttsnErrorCode_AlreadyStarted,
    MqttsnErrorCode_NotStarted,
    MqttsnErrorCode_Busy,
    MqttsnErrorCode_AlreadyConnected,
    MqttsnErrorCode_NotConnected,
    MqttsnErrorCode_NotSleeping,
    MqttsnErrorCode_BadParam,
};

enum MqttsnGwStatus
{
    MqttsnGwStatus_Invalid,
    MqttsnGwStatus_Available,
    MqttsnGwStatus_TimedOut
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

enum MqttsnSearchgwMode
{
    MqttsnSearchgwMode_UntilFirstGw,
    MqttsnSearchgwMode_Always,
    MqttsnSearchgwMode_Disabled,
};

typedef void* MqttsnClientHandle;
typedef unsigned short MqttsnTopicId;

struct MqttsnWillInfo
{
    const char* topic;
    const unsigned char* msg;
    unsigned msgLen;
    MqttsnQoS qos;
    bool retain;
};

struct MqttsnMessageInfo
{
    const char* topic;
    MqttsnTopicId topicId;
    const unsigned char* msg;
    unsigned msgLen;
    MqttsnQoS qos;
    bool retain;
};

typedef void (*MqttsnNextTickProgramFn)(void* data, unsigned duration);
typedef unsigned (*MqttsnCancelNextTickWaitFn)(void* data);
typedef void (*MqttsnSendOutputDataFn)(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);
typedef void (*MqttsnGwStatusReportFn)(void* data, unsigned short gwId, MqttsnGwStatus status);
typedef void (*MqttsnConnectionStatusReportFn)(void* data, MqttsnConnectionStatus status);
typedef void (*MqttsnPublishCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);
typedef void (*MqttsnSubscribeCompleteReportFn)(void* data, MqttsnAsyncOpStatus status, MqttsnQoS qos);
typedef void (*MqttsnUnsubscribeCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);
typedef void (*MqttsnWillUpdateCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);
typedef void (*MqttsnWillTopicUpdateCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);
typedef void (*MqttsnWillMsgUpdateCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);
typedef void (*MqttsnSleepCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);
typedef void (*MqttsnCheckMessagesCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);
typedef void (*MqttsnMessageReportFn)(void* data, const MqttsnMessageInfo* msgInfo);

#ifdef WIN32

#ifdef MQTTSN_CLIENT_LIB_EXPORT
#define MQTTSN_CLIENT_API __declspec(dllexport)
#else // #ifdef MQTTSN_GATEWAY_LIB_EXPORT
#define MQTTSN_CLIENT_API __declspec(dllimport)
#endif // #ifdef MQTTSN_GATEWAY_LIB_EXPORT

#else // #ifdef WIN32
#define MQTTSN_CLIENT_API
#endif // #ifdef WIN32


#ifdef __cplusplus
}
#endif
