//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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

/// @brief Major verion of the library
#define CC_MQTTSN_CLIENT_MAJOR_VERSION 1U

/// @brief Minor verion of the library
#define CC_MQTTSN_CLIENT_MINOR_VERSION 0U

/// @brief Patch level of the library
#define CC_MQTTSN_CLIENT_PATCH_VERSION 6U

/// @brief Macro to create numeric version as single unsigned number
#define CC_MQTTSN_CLIENT_MAKE_VERSION(major_, minor_, patch_) \
    ((static_cast<unsigned>(major_) << 24) | \
     (static_cast<unsigned>(minor_) << 8) | \
     (static_cast<unsigned>(patch_)))

/// @brief Version of the library as single numeric value
#define CC_MQTTSN_CLIENT_VERSION CC_MQTTSN_CLIENT_MAKE_VERSION(CC_MQTTSN_CLIENT_MAJOR_VERSION, CC_MQTTSN_CLIENT_MINOR_VERSION, CC_MQTTSN_CLIENT_PATCH_VERSION)

/// @brief Quality of Service
typedef enum
{
    CC_MqttsnQoS_NoGwPublish = -1, ///< QoS=-1. No gateway publish, used by publish only clients.
    CC_MqttsnQoS_AtMostOnceDelivery, ///< QoS=0. At most once delivery.
    CC_MqttsnQoS_AtLeastOnceDelivery, ///< QoS=1. At least once delivery.
    CC_MqttsnQoS_ExactlyOnceDelivery ///< QoS=2. Exactly once delivery.
} CC_MqttsnQoS;

/// @brief Error code returned by various API functions.
typedef enum
{
    CC_MqttsnErrorCode_Success, ///< The requested operation was successfully started.
    CC_MqttsnErrorCode_AlreadyStarted, ///< Returned by cc_mqttsn_client_start() function if invoked twice.
    CC_MqttsnErrorCode_NotStarted, ///< Returned by various operations if issued prior to successful start using cc_mqttsn_client_start().
    CC_MqttsnErrorCode_Busy, ///< The client library is in the middle of previous operation, cannot start a new one.
    CC_MqttsnErrorCode_AlreadyConnected, ///< The client library is already connected to the gateway. Returned when cc_mqttsn_client_connect() invoked second time.
    CC_MqttsnErrorCode_NotConnected, ///< The client library is not connected to the gateway. Returned by operations that require connection to the gateway.
    CC_MqttsnErrorCode_NotSleeping, ///< The client is not in ASLEEP mode.
    CC_MqttsnErrorCode_BadParam, ///< Bad parameter is passed to the function.
} CC_MqttsnErrorCode;

/// @brief Status of the gateway
typedef enum
{
    CC_MqttsnGwStatus_Invalid, ///< Invalid value, should never be used
    CC_MqttsnGwStatus_Available, ///< The gateway is available.
    CC_MqttsnGwStatus_TimedOut, ///< The gateway hasn't advertised its presence in time, assumed disconnected.
    CC_MqttsnGwStatus_Discarded ///< The gateway info was discarded using cc_mqttsn_client_discard_gw() or cc_mqttsn_client_discard_all_gw().
} CC_MqttsnGwStatus;

/// @brief Status of the asynchronous operation
typedef enum
{
    CC_MqttsnAsyncOpStatus_Invalid, ///< Invalid value, should never be used
    CC_MqttsnAsyncOpStatus_Successful, ///< The operation was successful
    CC_MqttsnAsyncOpStatus_Congestion, ///< The gateway/broker was busy and could not handle the request, try again
    CC_MqttsnAsyncOpStatus_InvalidId, ///< Publish message used invalid topic ID.
    CC_MqttsnAsyncOpStatus_NotSupported, ///< The issued request is not supported by the gateway.
    CC_MqttsnAsyncOpStatus_NoResponse, ///< The gateway/broker didn't respond the the request
    CC_MqttsnAsyncOpStatus_Aborted, ///< The operation was cancelled using cc_mqttsn_client_cancel() call.
} CC_MqttsnAsyncOpStatus;

/// @brief Handler used to access client specific data structures.
/// @details Returned by cc_mqttsn_client_new() function.
typedef struct 
{
    void* m_ptr;
} CC_MqttsnClientHandle;

/// @brief Type used to hold Topic ID value.
typedef unsigned short CC_MqttsnTopicId;

/// @brief Will Information
typedef struct
{
    const char* topic; ///< Topic of the will, can be NULL (means empty topic)
    const unsigned char* msg; ///< Pointer to the buffer containing will binary message.
    unsigned msgLen; ///< Length of the buffer containing will binary message.
    CC_MqttsnQoS qos; ///< QoS level of the will message.
    bool retain; ///< Retain flag
} CC_MqttsnWillInfo;

/// @brief Incoming message information
typedef struct
{
    const char* topic; ///< Topic the message was published with. May be NULL if message is reported with predefined topic ID.
    CC_MqttsnTopicId topicId; ///< Predefined topic ID. This data member is used only if topic field has value NULL.
    const unsigned char* msg; ///< Pointer to reported message binary data.
    unsigned msgLen; ///< Number of bytes in reported message binary data.
    CC_MqttsnQoS qos; ///< QoS level the message was received with.
    bool retain; ///< Retain flag of the message.
} CC_MqttsnMessageInfo;

/// @brief Callback used to request time measurement.
/// @details The callback is set using
///     cc_mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] duration Time duration in @b milliseconds. After the requested
///     time expires, the cc_mqttsn_client_tick() function is expected to be invoked.
typedef void (*CC_MqttsnNextTickProgramFn)(void* data, unsigned duration);

/// @brief Callback used to request termination of existing time measurement.
/// @details The callback is set using
///     cc_mqttsn_client_set_cancel_next_tick_wait_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_cancel_next_tick_wait_callback() function.
/// @return Number of elapsed milliseconds since last time measurement request.
typedef unsigned (*CC_MqttsnCancelNextTickWaitFn)(void* data);

/// @brief Callback used to request to send data to the gateway.
/// @details The callback is set using
///     cc_mqttsn_client_set_send_output_data_callback() function. The reported
///     data resides in internal data structures of the client library, and
///     it can be updated right after the callback function returns. It means
///     the data may need to be copied into some other buffer which will be
///     held intact until the send over I/O link operation is complete.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_send_output_data_callback() function.
/// @param[in] buf Pointer to the buffer containing data to send
/// @param[in] bufLen Number of bytes to send
/// @param[in] broadcast Indication whether data needs to be broadcasted or
///     sent directly to the gateway.
typedef void (*CC_MqttsnSendOutputDataFn)(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);

/// @brief Callback used to report gateway status.
/// @details The callback is set using
///     cc_mqttsn_client_set_gw_status_report_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_gw_status_report_callback() function.
/// @param[in] gwId ID of the gateway.
/// @param[in] status Status of the gateway.
typedef void (*CC_MqttsnGwStatusReportFn)(void* data, unsigned char gwId, CC_MqttsnGwStatus status);

/// @brief Callback used to report unsolicited disconnection of the gateway.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
typedef void (*CC_MqttsnGwDisconnectReportFn)(void* data);

/// @brief Callback used to report completion of the asynchronous operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] status Status of the asynchronous operation.
typedef void (*CC_MqttsnAsyncOpCompleteReportFn)(void* data, CC_MqttsnAsyncOpStatus status);

/// @brief Callback used to report completion of the subscribe operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the subscribe request.
/// @param[in] status Status of the subscribe operation.
/// @param[in] qos Maximal level of quality of service, the gateway/broker is going to use to publish incoming messages.
typedef void (*CC_MqttsnSubscribeCompleteReportFn)(void* data, CC_MqttsnAsyncOpStatus status, CC_MqttsnQoS qos);

/// @brief Callback used to report incoming messages.
/// @details The callback is set using
///     cc_mqttsn_client_set_message_report_callback() function. The reported
///     data resides in internal data structures of the client library, and
///     it can be updated right after the callback function returns.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_message_report_callback() function.
/// @param[in] msgInfo Information about incoming message.
typedef void (*CC_MqttsnMessageReportFn)(void* data, const CC_MqttsnMessageInfo* msgInfo);

#ifdef __cplusplus
}
#endif
