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
/// @ingroup global
#define CC_MQTTSN_CLIENT_MAJOR_VERSION 2U

/// @brief Minor verion of the library
/// @ingroup global
#define CC_MQTTSN_CLIENT_MINOR_VERSION 0U

/// @brief Patch level of the library
/// @ingroup global
#define CC_MQTTSN_CLIENT_PATCH_VERSION 0U

/// @brief Macro to create numeric version as single unsigned number
#define CC_MQTTSN_CLIENT_MAKE_VERSION(major_, minor_, patch_) \
    ((static_cast<unsigned>(major_) << 24) | \
     (static_cast<unsigned>(minor_) << 8) | \
     (static_cast<unsigned>(patch_)))

/// @brief Version of the library as single numeric value
#define CC_MQTTSN_CLIENT_VERSION CC_MQTTSN_CLIENT_MAKE_VERSION(CC_MQTTSN_CLIENT_MAJOR_VERSION, CC_MQTTSN_CLIENT_MINOR_VERSION, CC_MQTTSN_CLIENT_PATCH_VERSION)

/// @brief Quality of Service
/// @ingroup client
typedef enum
{
    CC_MqttsnQoS_AtMostOnceDelivery = 0, ///< QoS=0. At most once delivery.
    CC_MqttsnQoS_AtLeastOnceDelivery = 1, ///< QoS=1. At least once delivery.
    CC_MqttsnQoS_ExactlyOnceDelivery = 2 ///< QoS=2. Exactly once delivery.
} CC_MqttsnQoS;

/// @brief Error code returned by various API functions.
/// @ingroup client
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
    CC_MqttsnErrorCode_ValuesLimit ///< Upper limit of the values
} CC_MqttsnErrorCode;

/// @brief Status of the gateway
/// @ingroup client
typedef enum
{
    CC_MqttsnGwStatus_AddedByGateway = 0, ///< Added by the @b ADVERTISE or @b GWINFO sent by the gateway messages
    CC_MqttsnGwStatus_AddedByClient = 1, ///< Added by the @b GWINFO message sent by another client.
    CC_MqttsnGwStatus_UpdatedByClient = 2, ///< The gateway's address was updated by another client.
    CC_MqttsnGwStatus_Alive = 3, ///< The @b ADVERTISE or @b GWINFO message have been received from the gateway indicating it's alive.
    CC_MqttsnGwStatus_Tentative = 4, ///< The gateway hasn't advertised its presence in time, assumed packet loss.
    CC_MqttsnGwStatus_Removed = 5, ///< The gateway hasn't advertised its presence in time, assumed no longer available.
    CC_MqttsnGwStatus_ValuesLimit ///< Limit for the values
} CC_MqttsnGwStatus;

/// @brief Status of the asynchronous operation
/// @ingroup client
typedef enum
{
    CC_MqttsnAsyncOpStatus_Complete = 0, ///< The requested operation has been completed, refer to reported extra details for information
    CC_MqttsnAsyncOpStatus_InternalError = 1, ///< Internal library error, please submit bug report    
    CC_MqttsnAsyncOpStatus_Timeout = 2, ///< The required response from broker hasn't been received in time
    CC_MqttsnAsyncOpStatus_Aborted = 3, ///< The operation has been aborted before completion due to client's side operation.
    CC_MqttsnAsyncOpStatus_OutOfMemory = 4, ///< The client library wasn't able to allocate necessary memory.
    CC_MqttsnAsyncOpStatus_BadParam = 5, ///< Bad value has been returned from the relevant callback.
    CC_MqttsnAsyncOpStatus_GatewayDisconnected = 6, ///< Gateway disconnection detected during the operation execution.
    CC_MqttsnAsyncOpStatus_ValuesLimit ///< Limit for the values
} CC_MqttsnAsyncOpStatus;

/// @brief Reason for reporting unsolicited gateway disconnection
/// @ingroup global
typedef enum
{
    CC_MqttsnGatewayDisconnectReason_DisconnectMsg = 0, ///< Gateway sent @b DISCONNECT message.
    CC_MqttsnGatewayDisconnectReason_NoGatewayResponse = 1, ///< No messages from the gateway and no response to @b PINGREQ.
    CC_MqttsnGatewayDisconnectReason_ValuesLimit ///< Limit for the values
} CC_MqttsnGatewayDisconnectReason;

/// @brief Return code as per MQTT-SN specification
/// @ingroup client
typedef enum
{
    CC_MqttsnReturnCode_Accepted = 0, ///< Accepted
    CC_MqttsnReturnCode_Conjestion = 1, ///< Rejected due to conjesion
    CC_MqttsnReturnCode_InvalidTopicId = 2, ///< Rejected due to invalid topic ID
    CC_MqttsnReturnCode_NotSupported = 3, ///< Rejected as not supported
    CC_MqttsnReturnCode_ValuesLimit ///< Limit for the values
} CC_MqttsnReturnCode;

/// @brief Declaration of struct for the @ref CC_MqttsnClientHandle;
struct CC_MqttsnClient;

/// @brief Handler used to access client specific data structures.
/// @details Returned by cc_mqttsn_client_alloc() function.
typedef struct CC_MqttsnClient* CC_MqttsnClientHandle;

/// @brief Declaration of the hidden structure used to define @ref CC_MqttsnSearchHandle
/// @ingroup search
struct CC_MqttsnSearch;

/// @brief Handle for "search" operation.
/// @details Returned by @b cc_mqttsn_client_search_prepare() function.
/// @ingroup "search".
typedef struct CC_MqttsnSearch* CC_MqttsnSearchHandle;

/// @brief Declaration of the hidden structure used to define @ref CC_MqttsnConnectHandle
/// @ingroup connect
struct CC_MqttsnConnect;

/// @brief Handle for "connect" operation.
/// @details Returned by @b cc_mqttsn_client_connect_prepare() function.
/// @ingroup "connect".
typedef struct CC_MqttsnConnect* CC_MqttsnConnectHandle;

/// @brief Declaration of the hidden structure used to define @ref CC_MqttsnDisconnectHandle
/// @ingroup disconnect
struct CC_MqttsnDisconnect;

/// @brief Handle for "disconnect" operation.
/// @details Returned by @b cc_mqttsn_client_disconnect_prepare() function.
/// @ingroup "disconnect".
typedef struct CC_MqttsnDisconnect* CC_MqttsnDisconnectHandle;

/// @brief Declaration of the hidden structure used to define @ref CC_MqttsnSubscribeHandle
/// @ingroup subscribe
struct CC_MqttsnSubscribe;

/// @brief Handle for "subscribe" operation.
/// @details Returned by @b cc_mqttsn_client_subscribe_prepare() function.
/// @ingroup "subscribe".
typedef struct CC_MqttsnSubscribe* CC_MqttsnSubscribeHandle;


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
/// @ingroup client
typedef struct
{
    unsigned char m_gwId; ///< Gateway ID
    const unsigned char* m_addr; ///< Address of the gateway if known, NULL if not.
    unsigned m_addrLen; ///< Length of the address
} CC_MqttsnGatewayInfo;

/// @brief Configuration the "connect" operation
/// @ingroup connect
typedef struct
{
    const char* m_clientId; ///< Client ID
    unsigned m_duration; ///< Duration (Keep alive) configuration in seconds. Defaults to 60 when initialized.
    bool m_cleanSession; ///< Clean session configuration
} CC_MqttsnConnectConfig;

/// @brief Information on the "connect" operation completion
/// @ingroup connect
typedef struct
{
    CC_MqttsnReturnCode m_returnCode; ///< Return code reported by the @b CONNACK message
} CC_MqttsnConnectInfo;

/// @brief Configuration the will for "connect" and "will" operations
/// @ingroup connect
typedef struct
{
    const char* m_topic; ///< Will topic.
    const unsigned char* m_data; ///< Will data (message).
    unsigned m_dataLen; ///< Will data (message) length.
    CC_MqttsnQoS m_qos; ///< Will message QoS.
    bool m_retain; ///< Will message retain configuration.
} CC_MqttsnWillConfig;

/// @brief Configuration the "subscribe" operation
/// @ingroup subscribe
typedef struct
{
    const char* m_topic; ///< Subscription topic, can be NULL when pre-defined topic ID is used.
    CC_MqttsnTopicId m_topicId; ///< Pre-defined topic ID, should be @b 0 when topic is not NULL.
    CC_MqttsnQoS m_qos; ///< Max QoS value
} CC_MqttsnSubscribeConfig;

/// @brief Information on the "subscribe" operation completion
/// @ingroup subscribe
typedef struct
{
    CC_MqttsnReturnCode m_returnCode; ///< Return code reported by the @b SUBACK message
    CC_MqttsnTopicId m_topicId; ///< Granted topic ID (if applicable).
    CC_MqttsnQoS m_qos; ///< Granted max QoS value
} CC_MqttsnSubscribeInfo;

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
/// @param[in] status Current status of the gateway.
/// @param[in] info Currently stored gateway information.
typedef void (*CC_MqttsnGwStatusReportCb)(void* data, CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info);

/// @brief Callback used to report unsolicited disconnection of the gateway.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] reason Reason of the disconnection.
typedef void (*CC_MqttsnGwDisconnectedReportCb)(void* data, CC_MqttsnGatewayDisconnectReason reason);

/// @brief Callback used to report incoming messages.
/// @details The callback is set using
///     cc_mqttsn_client_set_message_report_callback() function. The reported
///     data resides in internal data structures of the client library, and
///     it can be updated right after the callback function returns.
/// @param[in] data Pointer to user data object, passed as last parameter to
///     cc_mqttsn_client_set_message_report_callback() function.
/// @param[in] msgInfo Information about incoming message.
typedef void (*CC_MqttsnMessageReportCb)(void* data, const CC_MqttsnMessageInfo* msgInfo);

/// @brief Callback used to report discovered errors.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] msg Error log message.
/// @ingroup client
typedef void (*CC_MqttsnErrorLogCb)(void* data, const char* msg);

/// @brief Callback used to request delay (in ms) to wait before 
///     responding with @b GWINFO message on behalf of a gateway.
/// @details In case function return 0U, the response on behalf of the gateway is disabled.
/// @return Number of milliseconds to wait for another @b GWINFO to cancel the intended send of @b GWINFO on behalf of the gateway.
typedef unsigned (*CC_MqttsnGwinfoDelayRequestCb)(void* data);

/// @brief Callback used to report completion of the asynchronous operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] status Status of the asynchronous operation.
/// @param[in] info Discovered gateway information. Not NULL if and only if @b status is @ref CC_MqttsnAsyncOpStatus_Complete.
/// @ingroup search
typedef void (*CC_MqttsnSearchCompleteCb)(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info);

/// @brief Callback used to report completion of the connect operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] status Status of the "connect" operation.
/// @param[in] info Information about op completion. Not-NULL is reported <b>if and onfly if</b>
///     the "status" is equal to @ref CC_MqttsnAsyncOpStatus_Complete.
/// @post The data members of the reported response can NOT be accessed after the function returns.
/// @ingroup connect
typedef void (*CC_MqttsnConnectCompleteCb)(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info);

/// @brief Callback used to report completion of the disconnect operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] status Status of the "disconnect" operation.
/// @post The data members of the reported response can NOT be accessed after the function returns.
/// @ingroup disconnect
typedef void (*CC_MqttsnDisconnectCompleteCb)(void* data, CC_MqttsnAsyncOpStatus status);

/// @brief Callback used to report completion of the subscribe operation.
/// @param[in] data Pointer to user data object, passed as the last parameter to
///     the request call.
/// @param[in] status Status of the "subscribe" operation.
/// @param[in] info Information about op completion. Not-NULL is reported <b>if and onfly if</b>
///     the "status" is equal to @ref CC_MqttsnAsyncOpStatus_Complete.
/// @post The data members of the reported response can NOT be accessed after the function returns.
/// @ingroup subscribe
typedef void (*CC_MqttsnSubscribeCompleteCb)(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info);

#ifdef __cplusplus
}
#endif
