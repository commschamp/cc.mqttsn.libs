//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief C interface for MQTT-SN Gateway library.

#pragma once

#include "version.h"

#ifdef __cplusplus
extern "C" {
#else // #ifdef __cplusplus
#include <stdbool.h>
#endif // #ifdef __cplusplus

/*===================== Gateway Object ======================*/

/// @brief Declaration of the struct for the @ref CC_MqttsnGatewayHandle definition.
struct CC_MqttsnGateway;

/// @brief Handle for gateway object used in all @b cc_mqttsn_gw_* functions.
typedef struct CC_MqttsnGateway* CC_MqttsnGatewayHandle;

/// @brief Type of callback function, to be used to request time measurement for
///     the @b Gateway object.
/// @details The callback is set using cc_mqttsn_gw_set_tick_req_cb(). When
///     the required time expires, the driving code is responsible to invoke
///     cc_mqttsn_gw_tick() function.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] duration Requested time measurement duration in @b milliseconds.
typedef void (*CC_MqttsnGwTickReqCb)(void* userData, unsigned duration);

/// @brief Type of callback function, to be used to request broadcast of serialised
///     @b ADVERTISE message.
/// @details The callback is set using cc_mqttsn_gw_set_advertise_broadcast_req_cb().
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] buf Pointer to the buffer of data, that needs to be sent.
/// @param[in] bufLen Number of bytes in the buffer.
/// @note The provided buffer resides in private data structres of the @b Gateway
///     object and can be updated right after the callback function returns. The
///     driving code may require to copy the buffer to its own data structures
///     and preserve it intact until send over I/O link is complete.
typedef void (*CC_MqttsnGwBroadcastReqCb)(void* userData, const unsigned char* buf, unsigned bufLen);

/// @brief Allocate @b Gateway object.
/// @details The returned handle need to be passed as first parameter
///     to all relevant functions. Note that the @b Gateway object is
///     dynamically allocated and needs to be freed using
///     cc_mqttsn_gw_free() function.
/// @return Handler to the allocated @b Gateway object.
CC_MqttsnGatewayHandle cc_mqttsn_gw_alloc(void);

/// @brief Free allocated @b Gateway object.
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
void cc_mqttsn_gw_free(CC_MqttsnGatewayHandle gw);

/// @brief Set the advertise period.
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
/// @param[in] value Advertise period in @b seconds.
void cc_mqttsn_gw_set_advertise_period(CC_MqttsnGatewayHandle gw, unsigned short value);

/// @brief Get current configuration of the advertise period
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
unsigned short cc_mqttsn_gw_get_advertise_period(CC_MqttsnGatewayHandle gw);

/// @brief Set the numeric gateway ID.
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
/// @param[in] id Numeric gateway ID.
void cc_mqttsn_gw_set_id(CC_MqttsnGatewayHandle gw, unsigned char id);

/// @brief Get current configuration of the numeric gateway id
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
unsigned char cc_mqttsn_gw_get_id(CC_MqttsnGatewayHandle gw);

/// @brief Set callback that requests to perform time measurement.
/// @details The @b Gateway object will invoke the callback to request time
///     measurement. When requested time expires, the driving code is responsible
///     to call cc_mqttsn_gw_tick().
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Any user data, to be passed as first parameter to the callback
void cc_mqttsn_gw_set_tick_req_cb(CC_MqttsnGatewayHandle gw, CC_MqttsnGwTickReqCb cb, void* data);

/// @brief Set callback that requests to send serialised @b ADVERTISE message.
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Any user data, to be passed as first parameter to the callback
void cc_mqttsn_gw_set_advertise_broadcast_req_cb(
    CC_MqttsnGatewayHandle gw,
    CC_MqttsnGwBroadcastReqCb cb,
    void* data);

/// @brief Start operation of the @b Gateway object.
/// @details The function check whether all callbacks and proper advertise
///     period were set properly and start the operation if everything is in
///     place. If start is successful, the callbacks of requesting data send
///     as well as data measurement request will be invoked.
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
/// @return @b true if the operation was successfully started, @b false otherwise.
bool cc_mqttsn_gw_start(CC_MqttsnGatewayHandle gw);

/// @brief Stop operation of the @b Gateway object.
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
void cc_mqttsn_gw_stop(CC_MqttsnGatewayHandle gw);

/// @brief Notify the @b Gateway object about requested time expiry.
/// @details Invocation of this function will cause  invocation of the send
///     data request callback as well as new time measurement request.
/// @param[in] gw Handle returned by cc_mqttsn_gw_alloc() function.
void cc_mqttsn_gw_tick(CC_MqttsnGatewayHandle gw);

/*===================== Session Object ======================*/

/// @brief Declaration of the struct for the @ref CC_MqttsnSessionHandle definition.
struct CC_MqttsnSession;

/// @brief Handle for session object used in all @b cc_mqttsn_gw_session_* functions.
typedef struct CC_MqttsnSession* CC_MqttsnSessionHandle;

/// @brief Type of callback, used to request new time measurement.
/// @details When the requested time is due, the driving code is expected
///     to call cc_mqttsn_gw_session_tick() member function.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
/// @param[in] duration Number of @b milliseconds to measure.
typedef void (*CC_MqttsnSessionTickReqCb)(void* userData, CC_MqttsnSessionHandle session, unsigned duration);

/// @brief Type of callback, used to cancel existing time measurement.
/// @details When invoked the existing time measurement needs to be cancelled.
///     The function also needs to return amount of @b milliseconds elapsed
///     since last timer programming request.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
/// @return Number of elapsed @b milliseconds since last timer programming
///     request.
typedef unsigned (*CC_MqttsnSessionCancelTickReqCb)(void* userData, CC_MqttsnSessionHandle session);

/// @brief Type of callback, used to request delivery of serialised message
///     to the client.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
/// @param[in] buf Buffer containing serialised message.
/// @param[in] bufLen Number of bytes in the buffer
/// @param[in] broadcastRadius Broadcast radius. 0 means unicast.
typedef void (*CC_MqttsnSessionClientSendDataReqCb)(void* userData, CC_MqttsnSessionHandle session, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);

/// @brief Type of callback, used to request delivery of serialised message
///     to the broker.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
/// @param[in] buf Buffer containing serialised message.
/// @param[in] bufLen Number of bytes in the buffer
typedef void (*CC_MqttsnSessionBrokerSendDataReqCb)(void* userData, CC_MqttsnSessionHandle session, const unsigned char* buf, unsigned bufLen);

/// @brief Type of callback, used to request session termination.
/// @details When the callback is invoked, the driving code must flush
///     all the previously sent messages to appropriate I/O links and
///     delete this session object.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
typedef void (*CC_MqttsnSessionTermReqCb)(void* userData, CC_MqttsnSessionHandle session);

/// @brief Type of callback used to request reconnection to the broker.
/// @details When the callback is invoked, the driving code must close
///     existing TCP/IP connection to the broker and create a new one.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
typedef void (*CC_MqttsnSessionBrokerReconnectReqCb)(void* userData, CC_MqttsnSessionHandle session);

/// @brief Type of callback used to report client ID of the newly connected
///     MQTT-SN client.
/// @details The callback can be used to provide additional client specific
///     information, such as predefined topic IDs.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the report
/// @param[in] clientId Client ID
typedef void (*CC_MqttsnSessionClientConnectReportCb)(void* userData, CC_MqttsnSessionHandle session, const char* clientId);

/// @brief Type of callback used to request authentication information of
///     the client that is trying to connect.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
/// @param[in] clientId Client ID
/// @param[out] username Username string
/// @param[out] password Binary password buffer
/// @param[out] passwordLen Length of the binary password
typedef void (*CC_MqttsnSessionAuthInfoReqCb)(
    void* userData,
    CC_MqttsnSessionHandle session,
    const char* clientId,
    const char** username,
    const unsigned char** password,
    unsigned* passwordLen);

/// @brief Type of callback used to report error messages detected by the session.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session performing the request
/// @param[in] msg Error message
typedef void (*CC_MqttsnSessionErrorReportCb)(void* userData, CC_MqttsnSessionHandle session, const char* msg);    

/// @brief Type of callback used to report forwarding encapsulated session creation.
/// @details The application is responsible to perform the necessary session configuration
///     as well as set all the callbacks except the one set by the @ref cc_mqttsn_gw_session_set_send_data_to_client_cb()
///     and @ref cc_mqttsn_gw_session_set_term_req_cb(). The data sent to the client as well as the session
///     termination are managed by the calling session object. The application is responsible
///     to manage the timer as well as broker connection of the reported session.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of created session object
/// @return @b true in case of success, @b false in case of falure.
typedef bool (*CC_MqttsnSessionFwdEncSessionCreatedCb)(void* userData, CC_MqttsnSessionHandle session);

/// @brief Type of callback used to report forwarding encapsulated session about to be deleted.
/// @details The application is responsible to remove any reference to the session object from its internal data structes.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] session Handle of session object about to be deleted
typedef void (*CC_MqttsnSessionFwdEncSessionDeletedCb)(void* userData, CC_MqttsnSessionHandle session);

/// @brief Allocate @b Session object.
/// @details The returned handle need to be passed as first parameter
///     to all relevant functions. Note that the @b Session object is
///     dynamically allocated and needs to be freed using
///     cc_mqttsn_gw_session_free() function.
/// @return Handler to the allocated @b Session object.
CC_MqttsnSessionHandle cc_mqttsn_gw_session_alloc(void);

/// @brief Free allocated @b Session object.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
void cc_mqttsn_gw_session_free(CC_MqttsnSessionHandle session);

/// @brief Set the callback to be invoked when new time measurement is required.
/// @details This is a must have callback, without it the object can not
///     be started (see cc_mqttsn_gw_session_start()).
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_tick_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionTickReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when previously requested time
///     measurement needs to be cancelled.
/// @details This is a must have callback, without it the object can not
///     be started (see cc_mqttsn_gw_session_start()).
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_cancel_tick_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionCancelTickReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when new data needs to be sent
///     to the @b client.
/// @details This is a must have callback, without it the object can not
///     be started (see cc_mqttsn_gw_session_start()).
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_send_data_to_client_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionClientSendDataReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when new data needs to be sent
///     to the @b broker.
/// @details This is a must have callback, without it the object can not
///     be started (see cc_mqttsn_gw_session_start()).
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_send_data_to_broker_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionBrokerSendDataReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when the @b Session needs to be
///     terminated and the calling @b Session object deleted.
/// @details This is a must have callback, without it the object can not
///     be started (see cc_mqttsn_gw_session_start()).
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_term_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionTermReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when the @b Session needs to close
///     existing TCP/IP connection to the broker and open a new one.
/// @details This is a must have callback, without it the object can not
///     be started (see cc_mqttsn_gw_session_start()).
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_broker_reconnect_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionBrokerReconnectReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when MQTT-SN client is successfully
///     connected to the broker.
/// @details This is an optional callback. It can be used when there is a
///     need to provide client specific configuration, such as predefined
///     topic IDs, valid only for specific client.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_client_connect_report_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionClientConnectReportCb cb,
    void* data);

/// @brief Set the callback to be used to request authentication information
///     for specific client.
/// @details This is an optional callback. It can be used when there is a
///     need to provide authentication details (username/password) for
///     specific clients.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_auth_info_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionAuthInfoReqCb cb,
    void* data);

/// @brief Set the callback to be used to report error messages detected by the session.
/// @details This is an optional callback. 
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_error_report_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionErrorReportCb cb,
    void* data);    

/// @brief Set the callback to be invoked when the forwarding encapsulation session 
///     is detected and to notify application about such session creation.
/// @details When not set, the forwarding enapsulation messages will be ignored
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_fwd_enc_session_created_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionFwdEncSessionCreatedCb cb,
    void* data);

/// @brief Set the callback to be invoked when the forwarding encapsulation session 
///     is about to be deleted.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void cc_mqttsn_gw_session_set_fwd_enc_session_deleted_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionFwdEncSessionDeletedCb cb,
    void* data);    

/// @brief Set gateway numeric ID to be reported when requested.
/// @details If not set, default value @b 0 is assumed.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] id Gateway numeric ID.
void cc_mqttsn_gw_session_set_id(CC_MqttsnSessionHandle session, unsigned char id);

/// @brief Get current gateway numeric ID configuration.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
unsigned char cc_mqttsn_gw_session_get_id(CC_MqttsnSessionHandle session);

/// @brief Set retry period to wait between resending unacknowledged message
///     to the client and/or broker.
/// @details Some messages, may require acknowledgement by
///     the client and/or broker. The delay (in seconds) between such
///     attempts to resend the message may be specified using this function.
///     The default value is @b 10 seconds.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] value Number of @b seconds to wait before making an attempt to resend.
void cc_mqttsn_gw_session_set_retry_period(CC_MqttsnSessionHandle session, unsigned value);

/// @brief Get the current configuration of the retry period.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
unsigned cc_mqttsn_gw_session_get_retry_period(CC_MqttsnSessionHandle session);

/// @brief Set number of retry attempts to perform before abandoning attempt
///     to send unacknowledged message.
/// @details Some messages, may require acknowledgement by
///     the client and/or broker. The amount of retry attempts before
///     abandoning the attempt to deliver the message may be specified
///     using this function. The default value is @b 3.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] value Number of retry attempts.
void cc_mqttsn_gw_session_set_retry_count(CC_MqttsnSessionHandle session, unsigned value);

/// @brief Get the current configuration of the retry count.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
unsigned cc_mqttsn_gw_session_get_retry_count(CC_MqttsnSessionHandle session);

/// @brief Provide limit to number pending messages being accumulated for
///     the sleeping client.
/// @details When client is known to be in "ASLEEP" state, the gateway must
///     accumulate all the messages the broker sends until client wakes up
///     or explicitly requests to send them. This function may be used
///     to limit amount of such messages to prevent acquiring lots of
///     RAM by the gateway application.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] value Max number of pending messages.
void cc_mqttsn_gw_session_set_sleeping_client_msg_limit(
    CC_MqttsnSessionHandle session,
    unsigned long long value);

/// @brief Get currenly configured limit to pending messages being accumulated for the sleeping client.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
unsigned long long cc_mqttsn_gw_session_get_sleeping_client_msg_limit(CC_MqttsnSessionHandle session);

/// @brief Provide default client ID for clients that report empty one
///     in their attempt to connect.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] clientId Default client ID string.
void cc_mqttsn_gw_session_set_default_client_id(CC_MqttsnSessionHandle session, const char* clientId);

/// @brief Get current default client ID configuration.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
const char* cc_mqttsn_gw_session_get_default_client_id(CC_MqttsnSessionHandle session);

/// @brief Provide default "keep alive" period for "publish only" clients,
///     that do not make an attempt to connect to the gateway.
/// @details MQTT-SN protocol allows "publish only" clients that don't
///     make any attempt to connect to the gateway/broker and send all
///     their messages with QoS=-1. In this case, the gateway must connect
///     to the broker on behalf of the "publish only" client. Such connection
///     attempt requires to specify "keep alive" period. Use this function
///     to set the value.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] value Max number of seconds between messages the "publish only"
///     client is going to send.
/// @pre The provided value mustn't exceed 65535.
void cc_mqttsn_gw_session_set_pub_only_keep_alive(CC_MqttsnSessionHandle session, unsigned value);

/// @brief Get current configuration of the default "keep alive" period for "publish only" clients.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
unsigned cc_mqttsn_gw_session_get_pub_only_keep_alive(CC_MqttsnSessionHandle session);

/// @brief Start the @b Session's object's operation.
/// @details The function will check whether all necessary callbacks have been
///     set.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @return true if the operation has been successfully started, false in
///     case some necessary callback hasn't been set.
bool cc_mqttsn_gw_session_start(CC_MqttsnSessionHandle session);

/// @brief Stop the operation of the @b Session object.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
void cc_mqttsn_gw_session_stop(CC_MqttsnSessionHandle session);

/// @brief Notify the @b Session object about requested time period expiry.
/// @details This function needs to be called from the driving code after
///     the requested time measurement has expired.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
void cc_mqttsn_gw_session_tick(CC_MqttsnSessionHandle session);

/// @brief Provide data received from the @b client for processing.
/// @details This call may cause invocation of some callbacks, such as
///     request to cancel the currently running time measurement,
///     send new message(s) and/or (re)start time measurement.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] buf Pointer to the buffer of data to process.
/// @param[in] bufLen Number of bytes in the data buffer.
/// @return Number of processed bytes.
/// @note The function returns number of bytes that were actually consumed, and
///     can be removed from the holding buffer.
unsigned cc_mqttsn_gw_session_data_from_client(
    CC_MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen);

/// @brief Provide data received from the @b broker for processing.
/// @details This call may cause invocation of some callbacks, such as
///     request to cancel the currently running time measurement,
///     send new message(s) and/or (re)start time measurement.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] buf Pointer to the buffer of data to process.
/// @param[in] bufLen Number of bytes in the data buffer.
/// @return Number of processed bytes.
/// @note The function returns number of bytes that were actually consumed, and
///     can be removed from the holding buffer.
unsigned cc_mqttsn_gw_session_data_from_broker(
    CC_MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen);

/// @brief Notify the @b Session object about broker being connected / disconnected
/// @details The report of broker being connected or disconnected must
///     be performed only when the session's operation has been successfully
///     started (see cc_mqttsn_gw_session_start()). Otherwise the call to this function gets
///     ignored.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] connected Connection status - @b true means connected, @b false disconnected.
void cc_mqttsn_gw_session_set_broker_connected(CC_MqttsnSessionHandle session, bool connected);

/// @brief Get currently recorded broker connection status.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
bool cc_mqttsn_gw_session_get_broker_connected(CC_MqttsnSessionHandle session);

/// @brief Add predefined topic string and ID information.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] topic Topic string
/// @param[in] topicId Numeric topic ID.
/// @return success/failure status
bool cc_mqttsn_gw_session_add_predefined_topic(
    CC_MqttsnSessionHandle session,
    const char* topic,
    unsigned short topicId);

/// @brief Limit range of topic IDs allocated for newly registered topics.
/// @param[in] session Handle returned by cc_mqttsn_gw_session_alloc() function.
/// @param[in] minTopicId Min topic ID.
/// @param[in] maxTopicId Max topic ID.
/// @return success/failure status
bool cc_mqttsn_gw_session_set_topic_id_alloc_range(
    CC_MqttsnSessionHandle session,
    unsigned short minTopicId,
    unsigned short maxTopicId);

/*===================== Config Object ======================*/

/// @brief Info about single predefined topic
typedef struct
{
    const char* clientId; ///< Client ID
    const char* topic; ///< Topic string
    unsigned short topicId; ///< Numeric topic ID
} CC_MqttsnPredefinedTopicInfo;

/// @brief Authentication infor for a single client.
typedef struct
{
    const char* clientId; ///< Client ID
    const char* username; ///< Username string
    const char* password; ///< Password string (from the configuration)
} CC_MqttsnAuthInfo;

/// @brief Client I/O socket connection type
typedef enum
{
    CC_MqttsnClientConnectionType_Udp, ///< UDP/IP
    CC_MqttsnClientConnectionType_ValuesLimit ///< Limit to available values, must be last
} CC_MqttsnClientConnectionType;    

/// @brief Broker I/O socket connection type
typedef enum
{
    CC_MqttsnBrokerConnectionType_Tcp, ///< TCP/IP
    CC_MqttsnBrokerConnectionType_ValuesLimit ///< Limit to available values, must be last
} CC_MqttsnBrokerConnectionType;    

/// @brief Declaration of the struct for the @ref CC_MqttsnConfigHandle definition.
struct CC_MqttsnConfig;

/// @brief Handle for session object used in all @b cc_mqttsn_gw_config_* functions.
typedef struct CC_MqttsnConfig* CC_MqttsnConfigHandle;

/// @brief Allocate @b Config object.
/// @details The returned handle need to be passed as first parameter
///     to all relevant functions. Note that the @b Config object is
///     dynamically allocated and needs to be freed using
///     cc_mqttsn_gw_config_free() function.
/// @return Handler to the allocated @b Config object.
CC_MqttsnConfigHandle cc_mqttsn_gw_config_alloc(void);

/// @brief Free allocated @b Config object.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
void cc_mqttsn_gw_config_free(CC_MqttsnConfigHandle config);

/// @brief Parse configuration contents from string
/// @details Updates the default values with values read from string buffer.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @param[in] str Pointer to the string buffer, containing configuration.
void cc_mqttsn_gw_config_parse(CC_MqttsnConfigHandle config, const char* str);

/// @brief Read configuration file
/// @details Updates the default values with values read from the file.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @param[in] filename Path to the file.
bool cc_mqttsn_gw_config_read(CC_MqttsnConfigHandle config, const char* filename);

/// @brief Get gateway numeric ID.
/// @details Default value is @b 0.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @return Numeric gateway ID.
unsigned char cc_mqttsn_gw_config_id(CC_MqttsnConfigHandle config);

/// @brief Get advertise period.
/// @details Default value is @b 900 seconds (15 minutes).
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @return Advertise period in @b seconds.
unsigned short cc_mqttsn_gw_config_advertise_period(CC_MqttsnConfigHandle config);

/// @brief Get retry period
/// @details Default value is @b 10 seconds.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @return Retry period in @b seconds
unsigned cc_mqttsn_gw_config_retry_period(CC_MqttsnConfigHandle config);

/// @brief Get number of retry attempts.
/// @details Default value is @b 3.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @return Number of retry attempts
unsigned cc_mqttsn_gw_config_retry_count(CC_MqttsnConfigHandle config);

/// @brief Get default client ID.
/// @details Default value is empty string.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @return Default client ID.
const char* cc_mqttsn_gw_config_default_client_id(CC_MqttsnConfigHandle config);

/// @brief Get keep alive period for publish only clients
/// @details Default value is @b 60 seconds.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @return Keep alive period for publish only clients.
unsigned cc_mqttsn_gw_config_pub_only_keep_alive(CC_MqttsnConfigHandle config);

/// @brief Get limit for max number of messages to accumulate for sleeping
///     clients.
/// @details Default value is @b MAX_UINT seconds.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @return Max number of accumulated messages for sleeping clients.
unsigned cc_mqttsn_gw_config_sleeping_client_msg_limit(CC_MqttsnConfigHandle config);

/// @brief Get number of available predefined topic IDs.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
unsigned cc_mqttsn_gw_config_available_predefined_topics(CC_MqttsnConfigHandle config);

/// @brief Read information about available topic IDs into a buffer.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @param[out] buf Buffer to write information into
/// @param[in] bufLen Max number of element to write into the buffer.
/// @return Actual number of elements that have been written into a buffer.
unsigned cc_mqttsn_gw_config_get_predefined_topics(
    CC_MqttsnConfigHandle config,
    CC_MqttsnPredefinedTopicInfo* buf,
    unsigned bufLen);

/// @brief Get number of available authenticatin infos for all the clients.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
unsigned cc_mqttsn_gw_config_available_auth_infos(CC_MqttsnConfigHandle config);

/// @brief Read clients' authentication information into a buffer
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @param[out] buf Buffer to write information into
/// @param[in] bufLen Max number of element to write into the buffer.
/// @return Actual number of elements that have been written into a buffer.
unsigned cc_mqttsn_gw_config_get_auth_infos(
    CC_MqttsnConfigHandle config,
    CC_MqttsnAuthInfo* buf,
    unsigned bufLen);

/// @brief Get range of allowed topic IDs for allocation.
/// @details Default range is [1, 0xfffe]
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @param[out] min Minimal allowed topic ID.
/// @param[out] max Maximal allowed topic ID.
void cc_mqttsn_gw_config_topic_id_alloc_range(
    CC_MqttsnConfigHandle config,
    unsigned short* min,
    unsigned short* max);

/// @brief Get TCP/IP address of the broker.
/// @details Default address is @b 127.0.0.1
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
const char* cc_mqttsn_gw_config_broker_address(CC_MqttsnConfigHandle config);

/// @brief Get TCP/IP port of the broker.
/// @details Default value is @b 1883
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
unsigned short cc_mqttsn_gw_config_broker_port(CC_MqttsnConfigHandle config);

/// @brief Get log file.
/// @details Expected to be an aboslute file path or one of the special values:
///     "stdout", "stderr". Defaults to "stdout"
const char* cc_mqttsn_gw_config_log_file(CC_MqttsnConfigHandle config);

/// @brief Get client I/O socket connection type
/// @param config Handle returned by cc_mqttsn_gw_config_alloc() function.
CC_MqttsnClientConnectionType cc_mqttsn_gw_config_client_connection_type(CC_MqttsnConfigHandle config);

/// @brief Get broker I/O socket connection type
/// @param config Handle returned by cc_mqttsn_gw_config_alloc() function.
CC_MqttsnBrokerConnectionType cc_mqttsn_gw_config_broker_connection_type(CC_MqttsnConfigHandle config);

/// @brief Get number of available configuration values for the provided key
/// @details The key is the first word in the configuration line, and the
///     value is rest of the string until the end of the line.
/// @param[in] config Handle returned by cc_mqttsn_gw_config_alloc() function.
/// @param[in] key Key string.
unsigned cc_mqttsn_gw_config_values_count(CC_MqttsnConfigHandle config, const char* key);

/// @brief Get the available value for the configuration key.
/// @details The key is the first word in the configuration line, and the
///     value is rest of the string until the end of the line.
///     If the configuration value doesn't exist, @b NULL is returned.
const char* cc_mqttsn_gw_config_get_value(CC_MqttsnConfigHandle config, const char* key, unsigned idx);


#ifdef __cplusplus
}
#endif


