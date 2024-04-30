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

namespace
{

typedef cc_mqttsn_gateway::Config Config;
typedef cc_mqttsn_gateway::Gateway Gateway;
typedef cc_mqttsn_gateway::Session Session;

}  // namespace

/*===================== Gateway Object ======================*/

CC_MqttsnGatewayHandle cc_mqttsn_gw_alloc(void)
{
    CC_MqttsnGatewayHandle gw;
    gw.obj = new Gateway();
    return gw;
}

void cc_mqttsn_gw_free(CC_MqttsnGatewayHandle gw)
{
    std::unique_ptr<Gateway>(reinterpret_cast<Gateway*>(gw.obj));
}

void cc_mqttsn_gw_set_advertise_period(
    CC_MqttsnGatewayHandle gw,
    unsigned short value)
{
    if (gw.obj == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw.obj)->setAdvertisePeriod(value);
}

void cc_mqttsn_gw_set_id(CC_MqttsnGatewayHandle gw, unsigned char id)
{
    if (gw.obj == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw.obj)->setGatewayId(id);
}

void cc_mqttsn_gw_set_tick_req_cb(CC_MqttsnGatewayHandle gw, CC_MqttsnGwTickReqCb cb, void* data)
{
    if ((gw.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Gateway*>(gw.obj)->setNextTickProgramReqCb(
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
    if ((gw.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Gateway*>(gw.obj)->setSendDataReqCb(
        [cb, data](const unsigned char* buf, std::size_t bufLen)
        {
            cb(data, buf, static_cast<unsigned>(bufLen));
        });
}

bool cc_mqttsn_gw_start(CC_MqttsnGatewayHandle gw)
{
    if (gw.obj == nullptr) {
        return false;
    }

    return reinterpret_cast<Gateway*>(gw.obj)->start();
}

void cc_mqttsn_gw_stop(CC_MqttsnGatewayHandle gw)
{
    if (gw.obj == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw.obj)->stop();
}

void cc_mqttsn_gw_tick(CC_MqttsnGatewayHandle gw)
{
    if (gw.obj == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw.obj)->tick();
}

/*===================== Session Object ======================*/


CC_MqttsnSessionHandle cc_mqttsn_gw_session_alloc(void)
{
    CC_MqttsnSessionHandle session;
    session.obj = new Session;
    return session;
}

void cc_mqttsn_gw_session_free(CC_MqttsnSessionHandle session)
{
    std::unique_ptr<Session>(reinterpret_cast<Session*>(session.obj));
}

void cc_mqttsn_gw_session_set_tick_req_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionTickReqCb cb,
    void* data)
{
    if ((session.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setNextTickProgramReqCb(
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
    if ((session.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setCancelTickWaitReqCb(
        [cb, data, session]() -> unsigned
        {
            return cb(data, session);
        });
}

void cc_mqttsn_gw_session_set_send_data_to_client_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionSendDataReqCb cb,
    void* data)
{
    if ((session.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setSendDataClientReqCb(
        [cb, data, session](const std::uint8_t* buf, std::size_t bufLen)
        {
            cb(data, session, buf, static_cast<unsigned>(bufLen));
        });
}


void cc_mqttsn_gw_session_set_send_data_to_broker_cb(
    CC_MqttsnSessionHandle session,
    CC_MqttsnSessionSendDataReqCb cb,
    void* data)
{
    if ((session.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setSendDataBrokerReqCb(
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
    if ((session.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setTerminationReqCb(
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
    if ((session.obj == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setBrokerReconnectReqCb(
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
    if (session.obj == nullptr) {
        return;
    }

    if (cb == nullptr) {
        reinterpret_cast<Session*>(session.obj)->setClientConnectedReportCb(nullptr);
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setClientConnectedReportCb(
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
    if (session.obj == nullptr) {
        return;
    }

    if (cb == nullptr) {
        reinterpret_cast<Session*>(session.obj)->setAuthInfoReqCb(nullptr);
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setAuthInfoReqCb(
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

void cc_mqttsn_gw_session_set_id(CC_MqttsnSessionHandle session, unsigned char id)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setGatewayId(id);
}

void cc_mqttsn_gw_session_set_retry_period(CC_MqttsnSessionHandle session, unsigned value)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setRetryPeriod(value);
}

void cc_mqttsn_gw_session_set_retry_count(CC_MqttsnSessionHandle session, unsigned value)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setRetryCount(value);
}

void cc_mqttsn_gw_session_set_sleeping_client_msg_limit(
    CC_MqttsnSessionHandle session,
    unsigned value)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setSleepingClientMsgLimit(value);
}

void cc_mqttsn_gw_session_set_default_client_id(CC_MqttsnSessionHandle session, const char* clientId)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setDefaultClientId(clientId);
}

void cc_mqttsn_gw_session_set_pub_only_keep_alive(
    CC_MqttsnSessionHandle session,
    unsigned value)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setPubOnlyKeepAlive(static_cast<std::uint16_t>(value));
}

bool cc_mqttsn_gw_session_start(CC_MqttsnSessionHandle session)
{
    if (session.obj == nullptr) {
        return false;
    }

    return reinterpret_cast<Session*>(session.obj)->start();
}

void cc_mqttsn_gw_session_stop(CC_MqttsnSessionHandle session)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->stop();
}

void cc_mqttsn_gw_session_tick(CC_MqttsnSessionHandle session)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->tick();
}

unsigned cc_mqttsn_gw_session_data_from_client(
    CC_MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen)
{
    if (session.obj == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<Session*>(session.obj)->dataFromClient(buf, bufLen));

}

unsigned cc_mqttsn_gw_session_data_from_broker(
    CC_MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen)
{
    if (session.obj == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<Session*>(session.obj)->dataFromBroker(buf, bufLen));

}

void cc_mqttsn_gw_session_broker_connected(CC_MqttsnSessionHandle session, bool connected)
{
    if (session.obj == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session.obj)->setBrokerConnected(connected);
}

bool cc_mqttsn_gw_session_add_predefined_topic(
    CC_MqttsnSessionHandle session,
    const char* topic,
    unsigned short topicId)
{
    if (session.obj == nullptr) {
        return false;
    }

    return reinterpret_cast<Session*>(session.obj)->addPredefinedTopic(topic, topicId);
}

bool cc_mqttsn_gw_session_set_topic_id_alloc_range(
    CC_MqttsnSessionHandle session,
    unsigned short minTopicId,
    unsigned short maxTopicId)
{
    if (session.obj == nullptr) {
        return false;
    }

    return reinterpret_cast<Session*>(session.obj)->setTopicIdAllocationRange(minTopicId, maxTopicId);
}

/*===================== Config Object ======================*/

CC_MqttsnConfigHandle cc_mqttsn_gw_config_alloc(void)
{
    CC_MqttsnConfigHandle config;
    config.obj = new Config;
    return config;
}

void cc_mqttsn_gw_config_free(CC_MqttsnConfigHandle config)
{
    std::unique_ptr<Config>(reinterpret_cast<Config*>(config.obj));
}

void cc_mqttsn_gw_config_parse(CC_MqttsnConfigHandle config, const char* str)
{
    if (config.obj == nullptr) {
        return;
    }

    std::stringstream stream;
    stream << str;
    reinterpret_cast<Config*>(config.obj)->read(stream);
}

bool cc_mqttsn_gw_config_read(CC_MqttsnConfigHandle config, const char* filename)
{
    if (config.obj == nullptr) {
        return false;
    }

    std::ifstream stream(filename, std::ios_base::in);
    if (!stream) {
        return false;
    }

    reinterpret_cast<Config*>(config.obj)->read(stream);
    return true;
}

unsigned char cc_mqttsn_gw_config_id(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config.obj)->gatewayId();
}

unsigned short cc_mqttsn_gw_config_advertise_period(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config.obj)->advertisePeriod();
}

unsigned cc_mqttsn_gw_config_retry_period(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config.obj)->retryPeriod();
}

unsigned cc_mqttsn_gw_config_retry_count(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config.obj)->retryCount();
}

const char* cc_mqttsn_gw_config_default_client_id(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const Config*>(config.obj)->defaultClientId().c_str();
}

unsigned cc_mqttsn_gw_config_pub_only_keep_alive(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config.obj)->pubOnlyKeepAlive();
}

unsigned cc_mqttsn_gw_config_sleeping_client_msg_limit(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return std::numeric_limits<unsigned>::max();
    }

    return static_cast<unsigned>(
        std::max(
            reinterpret_cast<const Config*>(config.obj)->sleepingClientMsgLimit(),
            static_cast<std::size_t>(std::numeric_limits<unsigned>::max())));
}

unsigned cc_mqttsn_gw_config_available_predefined_topics(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<const Config*>(config.obj)->predefinedTopics().size());
}

unsigned cc_mqttsn_gw_config_get_predefined_topics(
    CC_MqttsnConfigHandle config,
    CC_MqttsnPredefinedTopicInfo* buf,
    unsigned bufLen)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    auto& predefinedTopics = reinterpret_cast<const Config*>(config.obj)->predefinedTopics();
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
    if (config.obj == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<const Config*>(config.obj)->authInfos().size());
}

unsigned cc_mqttsn_gw_config_get_auth_infos(
    CC_MqttsnConfigHandle config,
    CC_MqttsnAuthInfo* buf,
    unsigned bufLen)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    auto& authInfos = reinterpret_cast<const Config*>(config.obj)->authInfos();
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
    if (config.obj == nullptr) {
        return;
    }

    auto range = reinterpret_cast<const Config*>(config.obj)->topicIdAllocRange();
    if (min != nullptr) {
        *min = range.first;
    }

    if (max != nullptr) {
        *max = range.second;
    }
}

const char* cc_mqttsn_gw_config_broker_address(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const Config*>(config.obj)->brokerTcpHostAddress().c_str();
}

unsigned short cc_mqttsn_gw_config_broker_port(CC_MqttsnConfigHandle config)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config.obj)->brokerTcpHostPort();
}

unsigned cc_mqttsn_gw_config_values_count(CC_MqttsnConfigHandle config, const char* key)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    auto& map = reinterpret_cast<const Config*>(config.obj)->configMap();
    auto range = map.equal_range(key);
    return static_cast<unsigned>(std::distance(range.first, range.second));
}

const char* cc_mqttsn_gw_config_get_value(CC_MqttsnConfigHandle config, const char* key, unsigned idx)
{
    if (config.obj == nullptr) {
        return 0U;
    }

    auto& map = reinterpret_cast<const Config*>(config.obj)->configMap();
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

