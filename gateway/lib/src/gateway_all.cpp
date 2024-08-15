//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "cc_mqttsn_gateway/gateway_all.h"

#include <sstream>
#include <fstream>
#include <algorithm>
#include <limits>

#include "cc_mqttsn_gateway/gateway_allpp.h"

struct CC_MqttsnGateway {};
struct CC_MqttsnSession {};
struct CC_MqttsnConfig {};

namespace
{

using Config = cc_mqttsn_gateway::Config;
using Gateway = cc_mqttsn_gateway::Gateway;
using Session = cc_mqttsn_gateway::Session;

Gateway* asGateway(CC_MqttsnGatewayHandle handle)
{
    return reinterpret_cast<Gateway*>(handle);
}

CC_MqttsnGatewayHandle fromGateway(Gateway* obj)
{
    return reinterpret_cast<CC_MqttsnGatewayHandle>(obj);
}

Session* asSession(CC_MqttsnSessionHandle handle)
{
    return reinterpret_cast<Session*>(handle);
}

CC_MqttsnSessionHandle fromSession(Session* obj)
{
    return reinterpret_cast<CC_MqttsnSessionHandle>(obj);
}

Config* asConfig(CC_MqttsnConfigHandle handle)
{
    return reinterpret_cast<Config*>(handle);
}

CC_MqttsnConfigHandle fromConfig(Config* obj)
{
    return reinterpret_cast<CC_MqttsnConfigHandle>(obj);
}

}  // namespace

/*===================== Gateway Object ======================*/

CC_MqttsnGatewayHandle cc_mqttsn_gw_alloc(void)
{
    return fromGateway(new Gateway());
}

void cc_mqttsn_gw_free(CC_MqttsnGatewayHandle gw)
{
    std::unique_ptr<Gateway>(asGateway(gw));
}

void cc_mqttsn_gw_set_advertise_period(
    CC_MqttsnGatewayHandle gw,
    unsigned short value)
{
    if (gw == nullptr) {
        return;
    }

    asGateway(gw)->setAdvertisePeriod(value);
}

unsigned short cc_mqttsn_gw_get_advertise_period(CC_MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return 0U;
    }

    return asGateway(gw)->getAdvertisePeriod();    
}

void cc_mqttsn_gw_set_id(CC_MqttsnGatewayHandle gw, unsigned char id)
{
    if (gw == nullptr) {
        return;
    }

    asGateway(gw)->setGatewayId(id);
}

unsigned char cc_mqttsn_gw_get_id(CC_MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return 0U;
    }

    return asGateway(gw)->getGatewayId();    
}

void cc_mqttsn_gw_set_tick_req_cb(CC_MqttsnGatewayHandle gw, CC_MqttsnGwTickReqCb cb, void* data)
{
    if ((gw == nullptr) || (cb == nullptr)) {
        return;
    }

    asGateway(gw)->setNextTickProgramReqCb(
        [cb, data](unsigned duration)
        {
            cb(data, duration);
        });
}

void cc_mqttsn_gw_set_advertise_broadcast_req_cb(
    CC_MqttsnGatewayHandle gw,
    CC_MqttsnGwBroadcastReqCb cb,
    void* data)
{
    if ((gw == nullptr) || (cb == nullptr)) {
        return;
    }

    asGateway(gw)->setSendDataReqCb(
        [cb, data](const unsigned char* buf, std::size_t bufLen)
        {
            cb(data, buf, static_cast<unsigned>(bufLen));
        });
}

bool cc_mqttsn_gw_start(CC_MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return false;
    }

    return asGateway(gw)->start();
}

void cc_mqttsn_gw_stop(CC_MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return;
    }

    asGateway(gw)->stop();
}

void cc_mqttsn_gw_tick(CC_MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return;
    }

    asGateway(gw)->tick();
}

/*===================== Session Object ======================*/


CC_MqttsnSessionHandle cc_mqttsn_gw_session_alloc(void)
{
    return fromSession(new Session);
}

void cc_mqttsn_gw_session_free(CC_MqttsnSessionHandle session)
{
    std::unique_ptr<Session>(asSession(session));
}

void cc_mqttsn_gw_session_set_tick_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionTickReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    asSession(session)->setNextTickProgramReqCb(
        [cb, data, session](unsigned duration)
        {
            cb(data, session, duration);
        });
}

void cc_mqttsn_gw_session_set_cancel_tick_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionCancelTickReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    asSession(session)->setCancelTickWaitReqCb(
        [cb, data, session]() -> unsigned
        {
            return cb(data, session);
        });
}

void cc_mqttsn_gw_session_set_send_data_to_client_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionClientSendDataReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    asSession(session)->setSendDataClientReqCb(
        [cb, data, session](const std::uint8_t* buf, std::size_t bufLen, unsigned broadcastRadius)
        {
            cb(data, session, buf, static_cast<unsigned>(bufLen), broadcastRadius);
        });
}


void cc_mqttsn_gw_session_set_send_data_to_broker_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionBrokerSendDataReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    asSession(session)->setSendDataBrokerReqCb(
        [cb, data, session](const std::uint8_t* buf, std::size_t bufLen)
        {
            cb(data, session, buf, static_cast<unsigned>(bufLen));
        });
}

void cc_mqttsn_gw_session_set_term_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionTermReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    asSession(session)->setTerminationReqCb(
        [cb, data, session]()
        {
            cb(data, session);
        });
}

void cc_mqttsn_gw_session_set_broker_reconnect_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionBrokerReconnectReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    asSession(session)->setBrokerReconnectReqCb(
        [cb, data, session]()
        {
            cb(data, session);
        });
}

void cc_mqttsn_gw_session_set_client_connect_report_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionClientConnectReportCb cb,
    void* data)
{
    if (session == nullptr) {
        return;
    }

    if (cb == nullptr) {
        asSession(session)->setClientConnectedReportCb(nullptr);
        return;
    }

    asSession(session)->setClientConnectedReportCb(
        [cb, data, session](const std::string& clientId)
        {
            cb(data, session, clientId.c_str());
        });
}

void cc_mqttsn_gw_session_set_auth_info_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionAuthInfoReqCb cb,
    void* data)
{
    if (session == nullptr) {
        return;
    }

    if (cb == nullptr) {
        asSession(session)->setAuthInfoReqCb(nullptr);
        return;
    }

    asSession(session)->setAuthInfoReqCb(
        [cb, data, session](const std::string& clientId) -> Session::AuthInfo
        {
            const char* username = nullptr;
            const std::uint8_t* password = nullptr;
            unsigned passLen = 0U;
            cb(data, session, clientId.c_str(), &username, &password, &passLen);

            Session::AuthInfo info;
            if (username != nullptr) {
                info.first = username;
            }

            if ((password != nullptr) && (0U < passLen)) {
                info.second.assign(password, password + passLen);
            }

            return info;
        });
}

void cc_mqttsn_gw_session_set_error_report_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionErrorReportCb cb,
    void* data)
{
    if (session == nullptr) {
        return;
    }    

    asSession(session)->setErrorReportCb(
        [cb, data, session](const char* msg)
        {
            cb(data, session, msg);
        });    
}

void cc_mqttsn_gw_session_set_fwd_enc_session_created_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionFwdEncSessionCreatedCb cb,
    void* data)
{
    if (session == nullptr) {
        return;
    }

    if (cb == nullptr) {
        asSession(session)->setFwdEncSessionCreatedReportCb(nullptr);
        return;
    }    

    asSession(session)->setFwdEncSessionCreatedReportCb(
        [cb, data](cc_mqttsn_gateway::Session* sessionPtr)
        {
            return cb(data, fromSession(sessionPtr));
        });    
}

void cc_mqttsn_gw_session_set_fwd_enc_session_deleted_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionFwdEncSessionDeletedCb cb,
    void* data)
{
    if (session == nullptr) {
        return;
    }

    if (cb == nullptr) {
        asSession(session)->setFwdEncSessionDeletedReportCb(nullptr);
        return;
    }    

    asSession(session)->setFwdEncSessionDeletedReportCb(
        [cb, data](cc_mqttsn_gateway::Session* sessionPtr)
        {
            cb(data, fromSession(sessionPtr));
        });    
}

void cc_mqttsn_gw_session_set_id(CC_MqttsnSessionHandle session, unsigned char id)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->setGatewayId(id);
}

unsigned char cc_mqttsn_gw_session_get_id(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return 0;
    }

    return asSession(session)->getGatewayId();    
}

void cc_mqttsn_gw_session_set_retry_period(CC_MqttsnSessionHandle session, unsigned value)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->setRetryPeriod(value);
}

unsigned cc_mqttsn_gw_session_get_retry_period(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return 0U;
    }

    return asSession(session)->getRetryPeriod();    
}

void cc_mqttsn_gw_session_set_retry_count(CC_MqttsnSessionHandle session, unsigned value)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->setRetryCount(value);
}

unsigned cc_mqttsn_gw_session_get_retry_count(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return 0U;
    }

    return asSession(session)->getRetryCount();    
}

void cc_mqttsn_gw_session_set_sleeping_client_msg_limit(
    CC_MqttsnSessionHandle session,
    unsigned value)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->setSleepingClientMsgLimit(static_cast<std::size_t>(value));
}

unsigned long long cc_mqttsn_gw_session_get_sleeping_client_msg_limit(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return 0U;
    }

    return static_cast<unsigned long long>(asSession(session)->getSleepingClientMsgLimit());    
}

void cc_mqttsn_gw_session_set_default_client_id(CC_MqttsnSessionHandle session, const char* clientId)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->setDefaultClientId(clientId);
}

const char* cc_mqttsn_gw_session_get_default_client_id(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return nullptr;
    }

    return asSession(session)->getDefaultClientId().c_str();    
}

void cc_mqttsn_gw_session_set_pub_only_keep_alive(
    CC_MqttsnSessionHandle session,
    unsigned value)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->setPubOnlyKeepAlive(static_cast<std::uint16_t>(value));
}

unsigned cc_mqttsn_gw_session_get_pub_only_keep_alive(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return 0U;
    }

    return asSession(session)->getPubOnlyKeepAlive();    
}

bool cc_mqttsn_gw_session_start(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return false;
    }

    return asSession(session)->start();
}

void cc_mqttsn_gw_session_stop(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->stop();
}

void cc_mqttsn_gw_session_tick(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->tick();
}

unsigned cc_mqttsn_gw_session_data_from_client(
    CC_MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen)
{
    if (session == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        asSession(session)->dataFromClient(buf, bufLen));

}

unsigned cc_mqttsn_gw_session_data_from_broker(
    CC_MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen)
{
    if (session == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        asSession(session)->dataFromBroker(buf, bufLen));

}

void cc_mqttsn_gw_session_set_broker_connected(CC_MqttsnSessionHandle session, bool connected)
{
    if (session == nullptr) {
        return;
    }

    asSession(session)->setBrokerConnected(connected);
}

bool cc_mqttsn_gw_session_get_broker_connected(CC_MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return false;
    }

    return asSession(session)->getBrokerConnected();    
}

bool cc_mqttsn_gw_session_add_predefined_topic(
    CC_MqttsnSessionHandle session,
    const char* topic,
    unsigned short topicId)
{
    if (session == nullptr) {
        return false;
    }

    return asSession(session)->addPredefinedTopic(topic, topicId);
}

bool cc_mqttsn_gw_session_set_topic_id_alloc_range(
    CC_MqttsnSessionHandle session,
    unsigned short minTopicId,
    unsigned short maxTopicId)
{
    if (session == nullptr) {
        return false;
    }

    return asSession(session)->setTopicIdAllocationRange(minTopicId, maxTopicId);
}

/*===================== Config Object ======================*/

CC_MqttsnConfigHandle cc_mqttsn_gw_config_alloc(void)
{
    return fromConfig(new Config);
}

void cc_mqttsn_gw_config_free(CC_MqttsnConfigHandle config)
{
    std::unique_ptr<Config>(asConfig(config));
}

void cc_mqttsn_gw_config_parse(CC_MqttsnConfigHandle config, const char* str)
{
    if (config == nullptr) {
        return;
    }

    std::stringstream stream;
    stream << str;
    asConfig(config)->read(stream);
}

bool cc_mqttsn_gw_config_read(CC_MqttsnConfigHandle config, const char* filename)
{
    if (config == nullptr) {
        return false;
    }

    std::ifstream stream(filename, std::ios_base::in);
    if (!stream) {
        return false;
    }

    asConfig(config)->read(stream);
    return true;
}

unsigned char cc_mqttsn_gw_config_id(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->gatewayId();
}

unsigned short cc_mqttsn_gw_config_advertise_period(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->advertisePeriod();
}

unsigned cc_mqttsn_gw_config_retry_period(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->retryPeriod();
}

unsigned cc_mqttsn_gw_config_retry_count(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->retryCount();
}

const char* cc_mqttsn_gw_config_default_client_id(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const Config*>(config)->defaultClientId().c_str();
}

unsigned cc_mqttsn_gw_config_pub_only_keep_alive(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->pubOnlyKeepAlive();
}

unsigned cc_mqttsn_gw_config_sleeping_client_msg_limit(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return std::numeric_limits<unsigned>::max();
    }

    return static_cast<unsigned>(
        std::max(
            reinterpret_cast<const Config*>(config)->sleepingClientMsgLimit(),
            static_cast<std::size_t>(std::numeric_limits<unsigned>::max())));
}

unsigned cc_mqttsn_gw_config_available_predefined_topics(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<const Config*>(config)->predefinedTopics().size());
}

unsigned cc_mqttsn_gw_config_get_predefined_topics(
    CC_MqttsnConfigHandle config,
    CC_MqttsnPredefinedTopicInfo* buf,
    unsigned bufLen)
{
    if (config == nullptr) {
        return 0U;
    }

    auto& predefinedTopics = reinterpret_cast<const Config*>(config)->predefinedTopics();
    auto total = std::min(static_cast<unsigned>(predefinedTopics.size()), bufLen);

    std::transform(
        predefinedTopics.begin(), predefinedTopics.begin() + total, buf,
        [](const cc_mqttsn_gateway::Config::PredefinedTopicInfo& info) -> CC_MqttsnPredefinedTopicInfo
        {
            CC_MqttsnPredefinedTopicInfo retInfo;
            retInfo.clientId = info.clientId.c_str();
            retInfo.topic = info.topic.c_str();
            retInfo.topicId = info.topicId;
            return retInfo;
        });
    return total;
}

unsigned cc_mqttsn_gw_config_available_auth_infos(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<const Config*>(config)->authInfos().size());
}

unsigned cc_mqttsn_gw_config_get_auth_infos(
    CC_MqttsnConfigHandle config,
    CC_MqttsnAuthInfo* buf,
    unsigned bufLen)
{
    if (config == nullptr) {
        return 0U;
    }

    auto& authInfos = reinterpret_cast<const Config*>(config)->authInfos();
    auto total = std::min(static_cast<unsigned>(authInfos.size()), bufLen);

    std::transform(
        authInfos.begin(), authInfos.begin() + total, buf,
        [](const cc_mqttsn_gateway::Config::AuthInfo& info) -> CC_MqttsnAuthInfo
        {
            CC_MqttsnAuthInfo retInfo;
            retInfo.clientId = info.clientId.c_str();
            retInfo.username = info.username.c_str();
            retInfo.password = info.password.c_str();
            return retInfo;
        });
    return total;
}

void cc_mqttsn_gw_config_topic_id_alloc_range(
    CC_MqttsnConfigHandle config,
    unsigned short* min,
    unsigned short* max)
{
    if (config == nullptr) {
        return;
    }

    auto range = reinterpret_cast<const Config*>(config)->topicIdAllocRange();
    if (min != nullptr) {
        *min = range.first;
    }

    if (max != nullptr) {
        *max = range.second;
    }
}

const char* cc_mqttsn_gw_config_broker_address(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const Config*>(config)->brokerTcpHostAddress().c_str();
}

unsigned short cc_mqttsn_gw_config_broker_port(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->brokerTcpHostPort();
}

const char* cc_mqttsn_gw_config_log_file(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const Config*>(config)->logFile().c_str();
}

CC_MqttsnClientConnectionType cc_mqttsn_gw_config_client_connection_type(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return CC_MqttsnClientConnectionType_ValuesLimit;
    }    

    return static_cast<CC_MqttsnClientConnectionType>(reinterpret_cast<const Config*>(config)->clientConnectionType());
}

CC_MqttsnBrokerConnectionType cc_mqttsn_gw_config_broker_connection_type(CC_MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return CC_MqttsnBrokerConnectionType_ValuesLimit;
    }    

    return static_cast<CC_MqttsnBrokerConnectionType>(reinterpret_cast<const Config*>(config)->brokerConnectionType());
}

unsigned cc_mqttsn_gw_config_values_count(CC_MqttsnConfigHandle config, const char* key)
{
    if (config == nullptr) {
        return 0U;
    }

    auto& map = reinterpret_cast<const Config*>(config)->configMap();
    auto range = map.equal_range(key);
    return static_cast<unsigned>(std::distance(range.first, range.second));
}

const char* cc_mqttsn_gw_config_get_value(CC_MqttsnConfigHandle config, const char* key, unsigned idx)
{
    if (config == nullptr) {
        return 0U;
    }

    auto& map = reinterpret_cast<const Config*>(config)->configMap();
    auto range = map.equal_range(key);

    unsigned count = 0;
    for (auto iter = range.first; iter != range.second; ++iter) {
        if (count == idx) {
            return iter->second.c_str();
        }

        ++count;
    }
    return nullptr;
}

