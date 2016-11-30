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

typedef struct
{
    const char* clientId;
    const char* topic;
    unsigned short topicId;
} MqttsnPredefinedTopicInfo;

typedef struct
{
    const char* clientId;
    const char* username;
    const char* password;
} MqttsnAuthInfo;

/// @brief Handle for configuration object used in all @b mqttsn_gw_config_* functions.
typedef void* MqttsnConfigHandle;

/// @brief Handle for gateway object used in all @b mqttsn_gw_* functions.
typedef void* MqttsnGatewayHandle;

/// @brief Handle for session object used in all @b mqttsn_gw_session_* functions.
typedef void* MqttsnSessionHandle;

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
/// @note The provided buffer resides in private data structres of the @ref Gateway
///     object and can be updated right after the callback function returns. The
///     driving code may require to copy the buffer to its own data structures
///     and preserve it intact until send over I/O link is complete.
typedef void (*MqttsnGwBroadcastReqCb)(void* userData, const unsigned char* buf, unsigned bufLen);

typedef void (*MqttsnSessionTickReqCb)(void* userData, unsigned duration);
typedef unsigned (*MqttsnSessionCancelTickReqCb)(void* userData);
typedef void (*MqttsnSessionSendDataReqCb)(void* userData, const unsigned char* buf, unsigned bufLen);
typedef void (*MqttsnSessionTermReqCb)(void* userData);
typedef void (*MqttsnSessionBrokerReconnectReqCb)(void* userData);
typedef void (*MqttsnSessionClientConnectReportCb)(void* userData, const char* clientId);
typedef void (*MqttsnSessionAuthInfoReqCb)(
    void* userData,
    const char* clientId,
    const char** username,
    const unsigned char** password,
    unsigned* passwordLen);

MqttsnConfigHandle mqttsn_gw_config_alloc(void);
void mqttsn_gw_config_free(MqttsnConfigHandle config);

void mqttsn_gw_config_parse(MqttsnConfigHandle config, const char* str);
bool mqttsn_gw_config_read(MqttsnConfigHandle config, const char* filename);
unsigned char mqttsn_gw_config_id(MqttsnConfigHandle config);
unsigned short mqttsn_gw_config_advertise_period(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_retry_period(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_retry_count(MqttsnConfigHandle config);
const char* mqttsn_gw_config_default_client_id(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_pub_only_keep_alive(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_sleepin_client_msg_limit(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_available_predefined_topics(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_get_predefined_topics(
    MqttsnConfigHandle config,
    MqttsnPredefinedTopicInfo* buf,
    unsigned bufLen);
unsigned mqttsn_gw_config_available_auth_infos(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_get_auth_infos(
    MqttsnConfigHandle config,
    MqttsnAuthInfo* buf,
    unsigned bufLen);
void mqttsn_gw_config_topic_id_alloc_range(
    MqttsnConfigHandle config,
    unsigned short* min,
    unsigned short* max);
const char* mqttsn_gw_config_broker_address(MqttsnConfigHandle config);
unsigned short mqttsn_gw_config_broker_port(MqttsnConfigHandle config);
unsigned mqttsn_gw_config_get_value(
    MqttsnConfigHandle config,
    const char* key,
    char* buf,
    unsigned bufLen);

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
/// @details The @ref Gateway object will invoke the callback to request time
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

/// @brief Notify the @ref Gateway object about requested time expiry.
/// @details Invocation of this function will cause  invocation of the send
///     data request callback as well as new time measurement request.
/// @param[in] gw Handle returned by mqttsn_gw_alloc() function.
void mqttsn_gw_tick(MqttsnGatewayHandle gw);

MqttsnSessionHandle mqttsn_gw_session_alloc(void);
void mqttsn_gw_session_free(MqttsnSessionHandle session);
void mqttsn_gw_session_set_tick_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionTickReqCb cb,
    void* data);
void mqttsn_gw_session_set_cancel_tick_cb(
    MqttsnSessionHandle session,
    MqttsnSessionCancelTickReqCb cb,
    void* data);

void mqttsn_gw_session_set_send_data_to_client_cb(
    MqttsnSessionHandle session,
    MqttsnSessionSendDataReqCb cb,
    void* data);

void mqttsn_gw_session_set_send_data_to_broker_cb(
    MqttsnSessionHandle session,
    MqttsnSessionSendDataReqCb cb,
    void* data);

void mqttsn_gw_session_set_term_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionTermReqCb cb,
    void* data);

void mqttsn_gw_session_set_broker_reconnect_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionBrokerReconnectReqCb cb,
    void* data);

void mqttsn_gw_session_set_client_connect_report_cb(
    MqttsnSessionHandle session,
    MqttsnSessionClientConnectReportCb cb,
    void* data);

void mqttsn_gw_session_set_auth_info_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionAuthInfoReqCb cb,
    void* data);

void mqttsn_gw_session_set_id(MqttsnSessionHandle session, unsigned char id);

void mqttsn_gw_session_set_retry_period(MqttsnSessionHandle session, unsigned value);

void mqttsn_gw_session_set_retry_count(MqttsnSessionHandle session, unsigned value);

void mqttsn_gw_session_set_sleeping_client_msg_limit(
    MqttsnSessionHandle session,
    unsigned value);

void mqttsn_gw_session_set_default_client_id(MqttsnSessionHandle session, const char* clientId);

void mqttsn_gw_session_set_pub_only_keep_alive(MqttsnSessionHandle session, unsigned value);

bool mqttsn_gw_session_start(MqttsnSessionHandle session);
void mqttsn_gw_session_stop(MqttsnSessionHandle session);
void mqttsn_gw_session_tick(MqttsnSessionHandle session, unsigned ms);

unsigned mqttsn_gw_session_data_from_client(
    MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen);

unsigned mqttsn_gw_session_data_from_broker(
    MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen);

void mqttsn_gw_session_broker_connected(MqttsnSessionHandle session, bool connected);

void mqttsn_gw_session_add_predefined_topic(
    MqttsnSessionHandle session,
    const char* topic,
    unsigned short topicId);

void mqttsn_gw_session_set_topic_alloc_range(
    MqttsnSessionHandle session,
    unsigned short minTopicId,
    unsigned short maxTopicId);


#ifdef __cplusplus
}
#endif


