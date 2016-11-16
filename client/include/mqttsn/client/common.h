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

/// @brief Quality of Service
enum MqttsnQoS
{
    MqttsnQoS_NoGwPublish = -1, ///< QoS=-1. No gateway publish, used by publish only clients.
    MqttsnQoS_AtMostOnceDelivery, ///< QoS=0. At most once delivery.
    MqttsnQoS_AtLeastOnceDelivery, ///< QoS=1. At least once delivery.
    MqttsnQoS_ExactlyOnceDelivery ///< QoS=2. Exactly once delivery.
};

/// @brief Connection status to the gateway
enum MqttsnConnectionStatus
{
    MqttsnConnectionStatus_Invalid, ///< Invalid value, should not be used
    MqttsnConnectionStatus_Connected, ///< The client is connected to the gateway
    MqttsnConnectionStatus_Denied, ///< The gateway or broker has rejected the connection attempt
    MqttsnConnectionStatus_Congestion, ///< The gateway is busy doing something else, retry later
    MqttsnConnectionStatus_Timeout, ///< The connection request timed out
    MqttsnConnectionStatus_Disconnected, ///< The gateway has disconnected.
    MqttsnConnectionStatus_DisconnectedAsleep, ///< The gateway acknowledged client's ASLEEP state.
    MqttsnConnectionStatus_ConnectAborted, ///< The connection attempt has been aborted using mqttsn_client_cancel() call.
    MqttsnConnectionStatus_NumOfValues ///< Number of available enumeration values, must be last
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

/// @brief Status of the gateway
enum MqttsnGwStatus
{
    MqttsnGwStatus_Invalid, ///< Invalid value, should never be used
    MqttsnGwStatus_Available, ///< The gateway is available.
    MqttsnGwStatus_TimedOut, ///< The gateway hasn't advertised its presence in time, assumed disconnected.
    MqttsnGwStatus_Discarded ///< The gateway info was discarded using mqttsn_client_discard_gw() or mqttsn_client_discard_all_gw().
};

enum MqttsnAsyncOpStatus
{
    MqttsnAsyncOpStatus_Invalid,
    MqttsnAsyncOpStatus_Successful,
    MqttsnAsyncOpStatus_Congestion,
    MqttsnAsyncOpStatus_InvalidId,
    MqttsnAsyncOpStatus_NotSupported,
    MqttsnAsyncOpStatus_NoResponse,
    MqttsnAsyncOpStatus_Aborted,
};

/// @brief Handler used to access client specific data structures.
/// @details Returned by mqttsn_client_new() function.
typedef void* MqttsnClientHandle;

/// @brief Type used to hold Topic ID value.
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

/// @brief Callback used to request time measurement.
/// @details The callback is set using
///     mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] duration Time duration in milliseconds. After the requested
///     time expires, the mqttsn_client_tick() function is expected to be invoked.
typedef void (*MqttsnNextTickProgramFn)(void* data, unsigned duration);

/// @brief Callback used to request termination of existing time measurement.
/// @details The callback is set using
///     mqttsn_client_set_cancel_next_tick_wait_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     mqttsn_client_set_cancel_next_tick_wait_callback() function.
/// @return Number of elapsed milliseconds since last time measurement request.
typedef unsigned (*MqttsnCancelNextTickWaitFn)(void* data);

/// @brief Callback used to request to send data to the gateway.
/// @details The callback is set using
///     mqttsn_client_set_send_output_data_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     mqttsn_client_set_send_output_data_callback() function.
/// @param[in] buf Pointer to the buffer containing data to send
/// @param[in] bufLen Number of bytes to send
/// @param[in] broadcast Indication whether data needs to be broadcasted or
///     sent directly to the gateway.
typedef void (*MqttsnSendOutputDataFn)(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);

/// @brief Callback used to report gateway status.
/// @details The callback is set using
///     mqttsn_client_set_gw_status_report_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     mqttsn_client_set_gw_status_report_callback() function.
/// @param[in] gwId ID of the gateway.
/// @param[in] status Status of the gateway.
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

#ifdef __cplusplus
}
#endif
