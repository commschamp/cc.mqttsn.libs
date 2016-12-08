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
/// @brief C interface for MQTT-SN Gateway library.

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

/*===================== Gateway Object ======================*/

/// @brief Handle for gateway object used in all @b mqttsn_gw_* functions.
typedef struct
{
    void* obj;
} MqttsnGatewayHandle;

/// @brief Type of callback function, to be used to request time measurement for
///     the @b Gateway object.
/// @details The callback is set using mqttsn_gw_set_tick_req_cb(). When
///     the required time expires, the driving code is responsible to invoke
///     mqttsn_gw_tick() function.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] duration Requested time measurement duration in @b milliseconds.
typedef void (*MqttsnGwTickReqCb)(void* userData, unsigned duration);

/// @brief Type of callback function, to be used to request broadcast of serialised
///     @b ADVERTISE message.
/// @details The callback is set using mqttsn_gw_set_advertise_broadcast_req_cb().
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] buf Pointer to the buffer of data, that needs to be sent.
/// @param[in] bufLen Number of bytes in the buffer.
/// @note The provided buffer resides in private data structres of the @b Gateway
///     object and can be updated right after the callback function returns. The
///     driving code may require to copy the buffer to its own data structures
///     and preserve it intact until send over I/O link is complete.
typedef void (*MqttsnGwBroadcastReqCb)(void* userData, const unsigned char* buf, unsigned bufLen);

/// @brief Allocate @b Gateway object.
/// @details The returned handle need to be passed as first parameter
///     to all relevant functions. Note that the @b Gateway object is
///     dynamically allocated and needs to be freed using
///     mqttsn_gw_free() function.
/// @return Handler to the allocated @b Gateway object.
MqttsnGatewayHandle mqttsn_gw_alloc(void);

/// @brief Free allocated @b Gateway object.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
void mqttsn_gw_free(MqttsnGatewayHandle gw);

/// @brief Set the advertise period.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
/// @param[in] value Advertise period in @b seconds.
void mqttsn_gw_set_advertise_period(MqttsnGatewayHandle gw, unsigned short value);

/// @brief Set the numeric gateway ID.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
/// @param[in] id Numeric gateway ID.
void mqttsn_gw_set_id(MqttsnGatewayHandle gw, unsigned char id);

/// @brief Set callback that requests to perform time measurement.
/// @details The @b Gateway object will invoke the callback to request time
///     measurement. When requested time expires, the driving code is responsible
///     to call mqttsn_gw_tick().
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Any user data, to be passed as first parameter to the callback
void mqttsn_gw_set_tick_req_cb(MqttsnGatewayHandle gw, MqttsnGwTickReqCb cb, void* data);

/// @brief Set callback that requests to send serialised @b ADVERTISE message.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Any user data, to be passed as first parameter to the callback
void mqttsn_gw_set_advertise_broadcast_req_cb(
    MqttsnGatewayHandle gw,
    MqttsnGwBroadcastReqCb cb,
    void* data);

/// @brief Start operation of the @b Gateway object.
/// @details The function check whether all callbacks and proper advertise
///     period were set properly and start the operation if everything is in
///     place. If start is successful, the callbacks of requesting data send
///     as well as data measurement request will be invoked.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
/// @return @b true if the operation was successfully started, @b false otherwise.
bool mqttsn_gw_start(MqttsnGatewayHandle gw);

/// @brief Stop operation of the @b Gateway object.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
void mqttsn_gw_stop(MqttsnGatewayHandle gw);

/// @brief Notify the @b Gateway object about requested time expiry.
/// @details Invocation of this function will cause  invocation of the send
///     data request callback as well as new time measurement request.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
void mqttsn_gw_tick(MqttsnGatewayHandle gw);

/*===================== Session Object ======================*/

/// @brief Handle for session object used in all @b mqttsn_gw_session_* functions.
typedef struct
{
    void* obj;
} MqttsnSessionHandle;

/// @brief Type of callback, used to request new time measurement.
/// @details When the requested time is due, the driving code is expected
///     to call mqttsn_gw_session_tick() member function.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] value Number of @b milliseconds to measure.
typedef void (*MqttsnSessionTickReqCb)(void* userData, unsigned duration);

/// @brief Type of callback, used to cancel existing time measurement.
/// @details When invoked the existing time measurement needs to be cancelled.
///     The function also needs to return amount of @b milliseconds elapsed
///     since last timer programming request.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @return Number of elapsed @b milliseconds since last timer programming
///     request.
typedef unsigned (*MqttsnSessionCancelTickReqCb)(void* userData);

/// @brief Type of callback, used to request delivery of serialised message
///     to the client or broker.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] buf Buffer containing serialised message.
/// @param[in] bufLen Number of bytes in the buffer
typedef void (*MqttsnSessionSendDataReqCb)(void* userData, const unsigned char* buf, unsigned bufLen);

/// @brief Type of callback, used to request session termination.
/// @details When the callback is invoked, the driving code must flush
///     all the previously sent messages to appropriate I/O links and
///     delete this session object.
/// @param[in] userData User data passed as the last parameter to the setting function.
typedef void (*MqttsnSessionTermReqCb)(void* userData);

/// @brief Type of callback used to request reconnection to the broker.
/// @details When the callback is invoked, the driving code must close
///     existing TCP/IP connection to the broker and create a new one.
/// @param[in] userData User data passed as the last parameter to the setting function.
typedef void (*MqttsnSessionBrokerReconnectReqCb)(void* userData);

/// @brief Type of callback used to report client ID of the newly connected
///     MQTT-SN client.
/// @details The callback can be used to provide additional client specific
///     information, such as predefined topic IDs.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] clientId Client ID
typedef void (*MqttsnSessionClientConnectReportCb)(void* userData, const char* clientId);

/// @brief Type of callback used to request authentication information of
///     the client that is trying to connect.
/// @param[in] userData User data passed as the last parameter to the setting function.
/// @param[in] clientId Client ID
/// @param[out] username Username string
/// @param[out] password Binary password buffer
/// @param[out] passwordLen Length of the binary password
typedef void (*MqttsnSessionAuthInfoReqCb)(
    void* userData,
    const char* clientId,
    const char** username,
    const unsigned char** password,
    unsigned* passwordLen);

/// @brief Allocate @b Session object.
/// @details The returned handle need to be passed as first parameter
///     to all relevant functions. Note that the @b Session object is
///     dynamically allocated and needs to be freed using
///     mqttsn_gw_session_free() function.
/// @return Handler to the allocated @b Session object.
MqttsnSessionHandle mqttsn_gw_session_alloc(void);

/// @brief Free allocated @b Session object.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
void mqttsn_gw_session_free(MqttsnSessionHandle session);

/// @brief Set the callback to be invoked when new time measurement is required.
/// @details This is a must have callback, without it the object can not
///     be started (see mqttsn_gw_session_start()).
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_tick_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionTickReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when previously requested time
///     measurement needs to be cancelled.
/// @details This is a must have callback, without it the object can not
///     be started (see mqttsn_gw_session_start()).
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_cancel_tick_cb(
    MqttsnSessionHandle session,
    MqttsnSessionCancelTickReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when new data needs to be sent
///     to the @b client.
/// @details This is a must have callback, without it the object can not
///     be started (see mqttsn_gw_session_start()).
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_send_data_to_client_cb(
    MqttsnSessionHandle session,
    MqttsnSessionSendDataReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when new data needs to be sent
///     to the @b broker.
/// @details This is a must have callback, without it the object can not
///     be started (see mqttsn_gw_session_start()).
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_send_data_to_broker_cb(
    MqttsnSessionHandle session,
    MqttsnSessionSendDataReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when the @b Session needs to be
///     terminated and the calling @b Session object deleted.
/// @details This is a must have callback, without it the object can not
///     be started (see mqttsn_gw_session_start()).
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_term_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionTermReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when the @b Session needs to close
///     existing TCP/IP connection to the broker and open a new one.
/// @details This is a must have callback, without it the object can not
///     be started (see mqttsn_gw_session_start()).
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_broker_reconnect_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionBrokerReconnectReqCb cb,
    void* data);

/// @brief Set the callback to be invoked when MQTT-SN client is successfully
///     connected to the broker.
/// @details This is an optional callback. It can be used when there is a
///     need to provide client specific configuration, such as predefined
///     topic IDs, valid only for specific client.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_client_connect_report_cb(
    MqttsnSessionHandle session,
    MqttsnSessionClientConnectReportCb cb,
    void* data);

/// @brief Set the callback to be used to request authentication information
///     for specific client.
/// @details This is an optional callback. It can be used when there is a
///     need to provide authentication details (username/password) for
///     specific clients.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] cb Pointer to callback function
/// @param[in] data Pointer to any user data, will be passed back as first
///     parameter to the callback.
void mqttsn_gw_session_set_auth_info_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionAuthInfoReqCb cb,
    void* data);

/// @brief Set gateway numeric ID to be reported when requested.
/// @details If not set, default value @b 0 is assumed.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] id Gateway numeric ID.
void mqttsn_gw_session_set_id(MqttsnSessionHandle session, unsigned char id);

/// @brief Set retry period to wait between resending unacknowledged message
///     to the client and/or broker.
/// @details Some messages, may require acknowledgement by
///     the client and/or broker. The delay (in seconds) between such
///     attempts to resend the message may be specified using this function.
///     The default value is @b 10 seconds.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] value Number of @b seconds to wait before making an attempt to resend.
void mqttsn_gw_session_set_retry_period(MqttsnSessionHandle session, unsigned value);

/// @brief Set number of retry attempts to perform before abandoning attempt
///     to send unacknowledged message.
/// @details Some messages, may require acknowledgement by
///     the client and/or broker. The amount of retry attempts before
///     abandoning the attempt to deliver the message may be specified
///     using this function. The default value is @b 3.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] value Number of retry attempts.
void mqttsn_gw_session_set_retry_count(MqttsnSessionHandle session, unsigned value);

/// @brief Provide limit to number pending messages being accumulated for
///     the sleeping client.
/// @details When client is known to be in "ASLEEP" state, the gateway must
///     accumulate all the messages the broker sends until client wakes up
///     or explicitly requests to send them. This function may be used
///     to limit amount of such messages to prevent acquiring lots of
///     RAM by the gateway application.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] value Max number of pending messages.
void mqttsn_gw_session_set_sleeping_client_msg_limit(
    MqttsnSessionHandle session,
    unsigned value);

/// @brief Provide default client ID for clients that report empty one
///     in their attempt to connect.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] clientId Default client ID string.
void mqttsn_gw_session_set_default_client_id(MqttsnSessionHandle session, const char* clientId);

/// @brief Provide default "keep alive" period for "publish only" clients,
///     that do not make an attempt to connect to the gateway.
/// @details MQTT-SN protocol allows "publish only" clients that don't
///     make any attempt to connect to the gateway/broker and send all
///     their messages with QoS=-1. In this case, the gateway must connect
///     to the broker on behalf of the "publish only" client. Such connection
///     attempt requires to specify "keep alive" period. Use this function
///     to set the value.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] value Max number of seconds between messages the "publish only"
///     client is going to send.
void mqttsn_gw_session_set_pub_only_keep_alive(MqttsnSessionHandle session, unsigned value);

/// @brief Start the @b Session's object's operation.
/// @details The function will check whether all necessary callbacks have been
///     set.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @return true if the operation has been successfully started, false in
///     case some necessary callback hasn't been set.
bool mqttsn_gw_session_start(MqttsnSessionHandle session);

/// @brief Stop the operation of the @b Session object.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
void mqttsn_gw_session_stop(MqttsnSessionHandle session);

/// @brief Notify the @b Session object about requested time period expiry.
/// @details This function needs to be called from the driving code after
///     the requested time measurement has expired.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
void mqttsn_gw_session_tick(MqttsnSessionHandle session);

/// @brief Provide data received from the @b client for processing.
/// @details This call may cause invocation of some callbacks, such as
///     request to cancel the currently running time measurement,
///     send new message(s) and/or (re)start time measurement.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] buf Pointer to the buffer of data to process.
/// @param[in] len Number of bytes in the data buffer.
/// @return Number of processed bytes.
/// @note The function returns number of bytes that were actually consumed, and
///     can be removed from the holding buffer.
unsigned mqttsn_gw_session_data_from_client(
    MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen);

/// @brief Provide data received from the @b broker for processing.
/// @details This call may cause invocation of some callbacks, such as
///     request to cancel the currently running time measurement,
///     send new message(s) and/or (re)start time measurement.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] buf Pointer to the buffer of data to process.
/// @param[in] len Number of bytes in the data buffer.
/// @return Number of processed bytes.
/// @note The function returns number of bytes that were actually consumed, and
///     can be removed from the holding buffer.
unsigned mqttsn_gw_session_data_from_broker(
    MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen);

/// @brief Notify the @b Session object about broker being connected / disconnected
/// @details The report of broker being connected or disconnected must
///     be performed only when the session's operation has been successfully
///     started (see mqttsn_gw_session_start()). Otherwise the call to this function gets
///     ignored.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] conneted Connection status - @b true means connected, @b false disconnected.
void mqttsn_gw_session_broker_connected(MqttsnSessionHandle session, bool connected);

/// @brief Add predefined topic string and ID information.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] topic Topic string
/// @param[in] topicId Numeric topic ID.
/// @return success/failure status
bool mqttsn_gw_session_add_predefined_topic(
    MqttsnSessionHandle session,
    const char* topic,
    unsigned short topicId);

/// @brief Limit range of topic IDs allocated for newly registered topics.
/// @param[in] session Handle returned by mqttsn_gw_session_alloc() function.
/// @param[in] minVal Min topic ID.
/// @param[in] maxVal Max topic ID.
/// @return success/failure status
bool mqttsn_gw_session_set_topic_id_alloc_range(
    MqttsnSessionHandle session,
    unsigned short minTopicId,
    unsigned short maxTopicId);

/*===================== Config Object ======================*/

/// @brief Info about single predefined topic
typedef struct
{
    const char* clientId; ///< Client ID
    const char* topic; ///< Topic string
    unsigned short topicId; ///< Numeric topic ID
} MqttsnPredefinedTopicInfo;

/// @brief Authentication infor for a single client.
typedef struct
{
    const char* clientId; ///< Client ID
    const char* username; ///< Username string
    const char* password; ///< Password string (from the configuration)
} MqttsnAuthInfo;

/// @brief Handle for configuration object used in all @b mqttsn_gw_config_* functions.
typedef struct
{
    void* obj;
} MqttsnConfigHandle;

/// @brief Allocate @b Config object.
/// @details The returned handle need to be passed as first parameter
///     to all relevant functions. Note that the @b Config object is
///     dynamically allocated and needs to be freed using
///     mqttsn_gw_config_free() function.
/// @return Handler to the allocated @b Config object.
MqttsnConfigHandle mqttsn_gw_config_alloc(void);

/// @brief Free allocated @b Config object.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
void mqttsn_gw_config_free(MqttsnConfigHandle config);

/// @brief Parse configuration contents from string
/// @details Updates the default values with values read from string buffer.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @param[in] str Pointer to the string buffer, containing configuration.
void mqttsn_gw_config_parse(MqttsnConfigHandle config, const char* str);

/// @brief Read configuration file
/// @details Updates the default values with values read from the file.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @param[in] filename Path to the file.
bool mqttsn_gw_config_read(MqttsnConfigHandle config, const char* filename);

/// @brief Get gateway numeric ID.
/// @details Default value is @b 0.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @return Numeric gateway ID.
unsigned char mqttsn_gw_config_id(MqttsnConfigHandle config);

/// @brief Get advertise period.
/// @details Default value is @b 900 seconds (15 minutes).
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @return Advertise period in @b seconds.
unsigned short mqttsn_gw_config_advertise_period(MqttsnConfigHandle config);

/// @brief Get retry period
/// @details Default value is @b 10 seconds.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @return Retry period in @b seconds
unsigned mqttsn_gw_config_retry_period(MqttsnConfigHandle config);

/// @brief Get number of retry attempts.
/// @details Default value is @b 3.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @return Number of retry attempts
unsigned mqttsn_gw_config_retry_count(MqttsnConfigHandle config);

/// @brief Get default client ID.
/// @details Default value is empty string.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @return Default client ID.
const char* mqttsn_gw_config_default_client_id(MqttsnConfigHandle config);

/// @brief Get keep alive period for publish only clients
/// @details Default value is @b 60 seconds.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @return Keep alive period for publish only clients.
unsigned mqttsn_gw_config_pub_only_keep_alive(MqttsnConfigHandle config);

/// @brief Get limit for max number of messages to accumulate for sleeping
///     clients.
/// @details Default value is @b MAX_UINT seconds.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @return Max number of accumulated messages for sleeping clients.
unsigned mqttsn_gw_config_sleeping_client_msg_limit(MqttsnConfigHandle config);

/// @brief Get number of available predefined topic IDs.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
unsigned mqttsn_gw_config_available_predefined_topics(MqttsnConfigHandle config);

/// @brief Read information about available topic IDs into a buffer.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @param[out] buf Buffer to write information into
/// @param[in] bufLen Max number of element to write into the buffer.
/// @return Actual number of elements that have been written into a buffer.
unsigned mqttsn_gw_config_get_predefined_topics(
    MqttsnConfigHandle config,
    MqttsnPredefinedTopicInfo* buf,
    unsigned bufLen);

/// @brief Get number of available authenticatin infos for all the clients.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
unsigned mqttsn_gw_config_available_auth_infos(MqttsnConfigHandle config);

/// @brief Read clients' authentication information into a buffer
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @param[out] buf Buffer to write information into
/// @param[in] bufLen Max number of element to write into the buffer.
/// @return Actual number of elements that have been written into a buffer.
unsigned mqttsn_gw_config_get_auth_infos(
    MqttsnConfigHandle config,
    MqttsnAuthInfo* buf,
    unsigned bufLen);

/// @brief Get range of allowed topic IDs for allocation.
/// @details Default range is [1, 0xfffe]
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @param[out] min Minimal allowed topic ID.
/// @param[out] max Maximal allowed topic ID.
void mqttsn_gw_config_topic_id_alloc_range(
    MqttsnConfigHandle config,
    unsigned short* min,
    unsigned short* max);

/// @brief Get TCP/IP address of the broker.
/// @details Default address is @b 127.0.0.1
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
const char* mqttsn_gw_config_broker_address(MqttsnConfigHandle config);

/// @brief Get TCP/IP port of the broker.
/// @details Default value is @b 1883
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
unsigned short mqttsn_gw_config_broker_port(MqttsnConfigHandle config);

/// @brief Get number of available configuration values for the provided key
/// @details The key is the first word in the configuration line, and the
///     value is rest of the string until the end of the line.
/// @param[in] config Handle returned by mqttsn_gw_config_alloc() function.
/// @param[in] key Key string.
unsigned mqttsn_gw_config_values_count(MqttsnConfigHandle config, const char* key);

/// @brief Get the available value for the configuration key.
/// @details The key is the first word in the configuration line, and the
///     value is rest of the string until the end of the line.
///     If the configuration value doesn't exist, @b NULL is returned.
const char* mqttsn_gw_config_get_value(MqttsnConfigHandle config, const char* key, unsigned idx);

#ifdef __cplusplus
}
#endif


