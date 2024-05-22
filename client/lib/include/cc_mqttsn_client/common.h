//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Common definition for MQTT-SN clients.

#pragma once

#ifdef __cplusplus
extern "C" {
#else // #ifdef __cplusplus  
#include <stdbool.h>
#endif // #ifdef __cplusplus

/// @brief Major verion of the library
#define CC_MQTTSN_CLIENT_MAJOR_VERSION 2U

/// @brief Minor verion of the library
#define CC_MQTTSN_CLIENT_MINOR_VERSION 0U

/// @brief Patch level of the library
#define CC_MQTTSN_CLIENT_PATCH_VERSION 0U

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
    CC_MqttsnErrorCode_Success = 0, ///< The requested operation was successfully started.
    CC_MqttsnErrorCode_InternalError = 1, ///< Internal library error, please submit bug report    
    CC_MqttsnErrorCode_NotIntitialized = 2, ///< The allocated client hasn't been initialized.
    CC_MqttsnErrorCode_Busy = 3, ///< The client library is in the middle of previous operation(s), cannot start a new one.
    CC_MqttsnErrorCode_NotConnected = 4, ///< The client library is not connected to the gateway. Returned by operations that require connection to the gateway.
    CC_MqttsnErrorCode_AlreadyConnected = 5, ///< The client library is already connected to the gateway. Returned when cc_mqttsn_client_connect() invoked second time.
    CC_MqttsnErrorCode_BadParam = 6, ///< Bad parameter is passed to the function.
    CC_MqttsnErrorCode_InsufficientConfig = 7, ///< The required configuration hasn't been performed.
    CC_MqttsnErrorCode_OutOfMemory = 8, ///< Memory allocation failed.
    CC_MqttsnErrorCode_BufferOverflow = 9, ///< Output buffer is too short
    CC_MqttsnErrorCode_NotSupported = 10, ///< Feature is not supported
    CC_MqttsnErrorCode_RetryLater = 11, ///< Retry in next event loop iteration.
    CC_MqttsnErrorCode_Disconnecting = 12, ///< The client is in "disconnecting" state, (re)connect is required in the next iteration loop.
    CC_MqttsnErrorCode_NotSleeping = 13, ///< The client is not in ASLEEP mode.
    CC_MqttsnErrorCode_PreparationLocked = 14, ///< Another operation is being prepared, cannot create a new one without performing "send" or "cancel".
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
    CC_MqttsnAsyncOpStatus_Congestion, ///< The gateway/gateway was busy and could not handle the request, try again
    CC_MqttsnAsyncOpStatus_InvalidId, ///< Publish message used invalid topic ID.
    CC_MqttsnAsyncOpStatus_NotSupported, ///< The issued request is not supported by the gateway.
    CC_MqttsnAsyncOpStatus_NoResponse, ///< The gateway/gateway didn't respond the the request
    CC_MqttsnAsyncOpStatus_Aborted, ///< The operation was cancelled using cc_mqttsn_client_cancel() call.
} CC_MqttsnAsyncOpStatus;

/// @brief Reason for reporting unsolicited gateway disconnection
/// @ingroup global
typedef enum
{
    CC_MqttsnGatewayDisconnectReason_DisconnectMsg = 0, ///< Gateway sent @b DISCONNECT message.
    CC_MqttsnGatewayDisconnectReason_NoGatewayResponse = 0, ///< No messages from the gateway and no response to @b PINGREQ.
    CC_MqttsnGatewayDisconnectReason_ValuesLimit ///< Limit for the values
} CC_MqttsnGatewayDisconnectReason;

/// @brief Declaration of struct for the @ref CC_MqttsnClientHandle;
struct CC_MqttsnClient;

/// @brief Handler used to access client specific data structures.
/// @details Returned by cc_mqttsn_client_new() function.
typedef struct CC_MqttsnClient* CC_MqttsnClientHandle;

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

/// @brief Gateway information
typedef struct
{
   unsigned char gwId; ///< Gateway ID
   CC_MqttsnGwStatus status; ///< Gateway status
} CC_MqttsnGatewayInfo;

/// @brief Callback used to request time measurement.
/// @details The callback is set using
///     cc_mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_next_tick_program_callback() function.
/// @param[in] duration Time duration in @b milliseconds. After the requested
///     time expires, the cc_mqttsn_client_tick() function is expected to be invoked.
typedef void (*CC_MqttsnNextTickProgramCb)(void* data, unsigned duration);

/// @brief Callback used to request termination of existing time measurement.
/// @details The callback is set using
///     cc_mqttsn_client_set_cancel_next_tick_wait_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_cancel_next_tick_wait_callback() function.
/// @return Number of elapsed milliseconds since last time measurement request.
typedef unsigned (*CC_MqttsnCancelNextTickWaitCb)(void* data);

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
/// @param[in] broadcastRadius Broadcast radius. When @b 0, means unicast to the connected gateway.
typedef void (*CC_MqttsnSendOutputDataCb)(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);

/// @brief Callback used to report gateway status.
/// @details The callback is set using
///     cc_mqttsn_client_set_gw_status_report_callback() function.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_gw_status_report_callback() function.
/// @param[in] info Gateway status info.
typedef void (*CC_MqttsnGwStatusReportCb)(void* data, const CC_MqttsnGatewayInfo* info);

/// @brief Callback used to report unsolicited disconnection of the gateway.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] reason Reason of the disconnection.
typedef void (*CC_MqttsnGwDisconnectedReportCb)(void* data, CC_MqttsnGatewayDisconnectReason reason);

/// @brief Callback used to report completion of the asynchronous operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] status Status of the asynchronous operation.
typedef void (*CC_MqttsnAsyncOpCompleteReportCb)(void* data, CC_MqttsnAsyncOpStatus status);

/// @brief Callback used to report completion of the subscribe operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the subscribe request.
/// @param[in] status Status of the subscribe operation.
/// @param[in] qos Maximal level of quality of service, the gateway/gateway is going to use to publish incoming messages.
typedef void (*CC_MqttsnSubscribeCompleteReportCb)(void* data, CC_MqttsnAsyncOpStatus status, CC_MqttsnQoS qos);

/// @brief Callback used to report incoming messages.
/// @details The callback is set using
///     cc_mqttsn_client_set_message_report_callback() function. The reported
///     data resides in internal data structures of the client library, and
///     it can be updated right after the callback function returns.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_message_report_callback() function.
/// @param[in] msgInfo Information about incoming message.
typedef void (*CC_MqttsnMessageReportCb)(void* data, const CC_MqttsnMessageInfo* msgInfo);

typedef void (*CC_MqttsnErrorLogCb)(void* data, const char* msg);

#ifdef __cplusplus
}
#endif
