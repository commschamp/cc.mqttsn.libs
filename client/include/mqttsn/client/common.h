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

/// @file
/// @brief Common definition for MQTT-SN clients.

#pragma once

#ifdef __cplusplus
extern "C" {
#else

#ifdef WIN32
#ifndef bool
#define bool char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#else // #ifdef WIN32
#include <stdbool.h>
#endif // #ifdef WIN32

#endif // #ifdef __cplusplus

/// @brief Quality of Service
typedef enum
{
    MqttsnQoS_NoGwPublish = -1, ///< QoS=-1. No gateway publish, used by publish only clients.
    MqttsnQoS_AtMostOnceDelivery, ///< QoS=0. At most once delivery.
    MqttsnQoS_AtLeastOnceDelivery, ///< QoS=1. At least once delivery.
    MqttsnQoS_ExactlyOnceDelivery ///< QoS=2. Exactly once delivery.
} MqttsnQoS;

/// @brief Error code returned by various API functions.
typedef enum
{
    MqttsnErrorCode_Success, ///< The requested operation was successfully started.
    MqttsnErrorCode_AlreadyStarted, ///< Returned by mqttsn_client_start() function if invoked twice.
    MqttsnErrorCode_NotStarted, ///< Returned by various operations if issued prior to successful start using mqttsn_client_start().
    MqttsnErrorCode_Busy, ///< The client library is in the middle of previous operation, cannot start a new one.
    MqttsnErrorCode_AlreadyConnected, ///< The client library is already connected to the gateway. Returned when mqttsn_client_connect() invoked second time.
    MqttsnErrorCode_NotConnected, ///< The client library is not connected to the gateway. Returned by operations that require connection to the gateway.
    MqttsnErrorCode_NotSleeping, ///< The client is not in ASLEEP mode.
    MqttsnErrorCode_BadParam, ///< Bad parameter is passed to the function.
} MqttsnErrorCode;

/// @brief Status of the gateway
typedef enum
{
    MqttsnGwStatus_Invalid, ///< Invalid value, should never be used
    MqttsnGwStatus_Available, ///< The gateway is available.
    MqttsnGwStatus_TimedOut, ///< The gateway hasn't advertised its presence in time, assumed disconnected.
    MqttsnGwStatus_Discarded ///< The gateway info was discarded using mqttsn_client_discard_gw() or mqttsn_client_discard_all_gw().
} MqttsnGwStatus;

/// @brief Status of the asynchronous operation
typedef enum
{
    MqttsnAsyncOpStatus_Invalid, ///< Invalid value, should never be used
    MqttsnAsyncOpStatus_Successful, ///< The operation was successful
    MqttsnAsyncOpStatus_Congestion, ///< The gateway/broker was busy and could not handle the request, try again
    MqttsnAsyncOpStatus_InvalidId, ///< Publish message used invalid topic ID.
    MqttsnAsyncOpStatus_NotSupported, ///< The issued request is not supported by the gateway.
    MqttsnAsyncOpStatus_NoResponse, ///< The gateway/broker didn't respond the the request
    MqttsnAsyncOpStatus_Aborted, ///< The operation was cancelled using mqttsn_client_cancel() call.
} MqttsnAsyncOpStatus;

/// @brief Handler used to access client specific data structures.
/// @details Returned by mqttsn_client_new() function.
typedef void* MqttsnClientHandle;

/// @brief Type used to hold Topic ID value.
typedef unsigned short MqttsnTopicId;

/// @brief Will Information
typedef struct
{
    const char* topic; ///< Topic of the will, can be NULL (means empty topic)
    const unsigned char* msg; ///< Pointer to the buffer containing will binary message.
    unsigned msgLen; ///< Length of the buffer containing will binary message.
    MqttsnQoS qos; ///< QoS level of the will message.
    bool retain; ///< Retain flag
} MqttsnWillInfo;

/// @brief Incoming message information
typedef struct
{
    const char* topic; ///< Topic the message was published with. May be NULL if message is reported with predefined topic ID.
    MqttsnTopicId topicId; ///< Predefined topic ID. This data member is used only if topic field has value NULL.
    const unsigned char* msg; ///< Pointer to reported message binary data.
    unsigned msgLen; ///< Number of bytes in reported message binary data.
    MqttsnQoS qos; ///< QoS level the message was received with.
    bool retain; ///< Retain flag of the message.
} MqttsnMessageInfo;

/// @brief Callback used to request time measurement.
/// @details The callback is set using
///     mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] duration Time duration in @b milliseconds. After the requested
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
///     mqttsn_client_set_send_output_data_callback() function. The reported
///     data resides in internal data structures of the client library, and
///     it can be updated right after the callback function returns. It means
///     the data may need to be copied into some other buffer which will be
///     held intact until the send over I/O link operation is complete.
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
typedef void (*MqttsnGwStatusReportFn)(void* data, unsigned char gwId, MqttsnGwStatus status);

/// @brief Callback used to report unsolicited disconnection of the gateway.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
typedef void (*MqttsnGwDisconnectReportFn)(void* data);

/// @brief Callback used to report completion of the asynchronous operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] status Status of the asynchronous operation.
typedef void (*MqttsnAsyncOpCompleteReportFn)(void* data, MqttsnAsyncOpStatus status);

/// @brief Callback used to report completion of the subscribe operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the subscribe request.
/// @param[in] status Status of the subscribe operation.
/// @param[in] qos Maximal level of quality of service, the gateway/broker is going to use to publish incoming messages.
typedef void (*MqttsnSubscribeCompleteReportFn)(void* data, MqttsnAsyncOpStatus status, MqttsnQoS qos);

/// @brief Callback used to report incoming messages.
/// @details The callback is set using
///     mqttsn_client_set_message_report_callback() function. The reported
///     data resides in internal data structures of the client library, and
///     it can be updated right after the callback function returns.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     mqttsn_client_set_message_report_callback() function.
/// @param[in] msgInfo Information about incoming message.
typedef void (*MqttsnMessageReportFn)(void* data, const MqttsnMessageInfo* msgInfo);

#ifdef __cplusplus
}
#endif
