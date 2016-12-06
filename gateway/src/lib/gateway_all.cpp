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

#include "mqttsn/gateway/gateway_all.h"

#include <sstream>
#include <fstream>
#include <algorithm>
#include <limits>

#include "mqttsn/gateway/gateway_allpp.h"

namespace
{

typedef mqttsn::gateway::Config Config;
typedef mqttsn::gateway::Gateway Gateway;
typedef mqttsn::gateway::Session Session;

}  // namespace

/*===================== Gateway Object ======================*/

MqttsnGatewayHandle mqttsn_gw_alloc(void)
{
    return new Gateway;
}

void mqttsn_gw_free(MqttsnGatewayHandle gw)
{
    std::unique_ptr<Gateway>(reinterpret_cast<Gateway*>(gw));
}

void mqttsn_gw_set_advertise_period(
    MqttsnGatewayHandle gw,
    unsigned short value)
{
    if (gw == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw)->setAdvertisePeriod(value);
}

void mqttsn_gw_set_id(MqttsnGatewayHandle gw, unsigned char id)
{
    if (gw == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw)->setGatewayId(id);
}

void mqttsn_gw_set_tick_req_cb(MqttsnGatewayHandle gw, MqttsnGwTickReqCb cb, void* data)
{
    if ((gw == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Gateway*>(gw)->setNextTickProgramReqCb(
        [cb, data](unsigned duration)
        {
            cb(data, duration);
        });
}

void mqttsn_gw_set_advertise_broadcast_req_cb(
    MqttsnGatewayHandle gw,
    MqttsnGwBroadcastReqCb cb,
    void* data)
{
    if ((gw == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Gateway*>(gw)->setSendDataReqCb(
        [cb, data](const unsigned char* buf, unsigned bufLen)
        {
            cb(data, buf, bufLen);
        });
}

bool mqttsn_gw_start(MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return false;
    }

    return reinterpret_cast<Gateway*>(gw)->start();
}

void mqttsn_gw_stop(MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw)->stop();
}

void mqttsn_gw_tick(MqttsnGatewayHandle gw)
{
    if (gw == nullptr) {
        return;
    }

    reinterpret_cast<Gateway*>(gw)->tick();
}

/*===================== Session Object ======================*/


MqttsnSessionHandle mqttsn_gw_session_alloc(void)
{
    return new Session;
}

void mqttsn_gw_session_free(MqttsnSessionHandle session)
{
    std::unique_ptr<Session>(reinterpret_cast<Session*>(session));
}

void mqttsn_gw_session_set_tick_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionTickReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session)->setNextTickProgramReqCb(
        [cb, data](unsigned duration)
        {
            cb(data, duration);
        });
}

void mqttsn_gw_session_set_cancel_tick_cb(
    MqttsnSessionHandle session,
    MqttsnSessionCancelTickReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session)->setCancelTickWaitReqCb(
        [cb, data]() -> unsigned
        {
            return cb(data);
        });
}

void mqttsn_gw_session_set_send_data_to_client_cb(
    MqttsnSessionHandle session,
    MqttsnSessionSendDataReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session)->setSendDataClientReqCb(
        [cb, data](const std::uint8_t* buf, std::size_t bufLen)
        {
            cb(data, buf, static_cast<unsigned>(bufLen));
        });
}


void mqttsn_gw_session_set_send_data_to_broker_cb(
    MqttsnSessionHandle session,
    MqttsnSessionSendDataReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session)->setSendDataBrokerReqCb(
        [cb, data](const std::uint8_t* buf, std::size_t bufLen)
        {
            cb(data, buf, static_cast<unsigned>(bufLen));
        });
}

void mqttsn_gw_session_set_term_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionTermReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session)->setTerminationReqCb(
        [cb, data]()
        {
            cb(data);
        });
}

void mqttsn_gw_session_set_broker_reconnect_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionBrokerReconnectReqCb cb,
    void* data)
{
    if ((session == nullptr) || (cb == nullptr)) {
        return;
    }

    reinterpret_cast<Session*>(session)->setBrokerReconnectReqCb(
        [cb, data]()
        {
            cb(data);
        });
}

void mqttsn_gw_session_set_client_connect_report_cb(
    MqttsnSessionHandle session,
    MqttsnSessionClientConnectReportCb cb,
    void* data)
{
    if (session == nullptr) {
        return;
    }

    if (cb == nullptr) {
        reinterpret_cast<Session*>(session)->setClientConnectedReportCb(nullptr);
        return;
    }

    reinterpret_cast<Session*>(session)->setClientConnectedReportCb(
        [cb, data](const std::string& clientId)
        {
            cb(data, clientId.c_str());
        });
}

void mqttsn_gw_session_set_auth_info_req_cb(
    MqttsnSessionHandle session,
    MqttsnSessionAuthInfoReqCb cb,
    void* data)
{
    if (session == nullptr) {
        return;
    }

    if (cb == nullptr) {
        reinterpret_cast<Session*>(session)->setAuthInfoReqCb(nullptr);
        return;
    }

    reinterpret_cast<Session*>(session)->setAuthInfoReqCb(
        [cb, data](const std::string& clientId) -> Session::AuthInfo
        {
            const char* username = nullptr;
            const std::uint8_t* password = nullptr;
            unsigned passLen = 0U;
            cb(data, clientId.c_str(), &username, &password, &passLen);

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

void mqttsn_gw_session_set_id(MqttsnSessionHandle session, unsigned char id)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->setGatewayId(id);
}

void mqttsn_gw_session_set_retry_period(MqttsnSessionHandle session, unsigned value)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->setRetryPeriod(value);
}

void mqttsn_gw_session_set_retry_count(MqttsnSessionHandle session, unsigned value)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->setRetryCount(value);
}

void mqttsn_gw_session_set_sleeping_client_msg_limit(
    MqttsnSessionHandle session,
    unsigned value)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->setSleepingClientMsgLimit(value);
}

void mqttsn_gw_session_set_default_client_id(MqttsnSessionHandle session, const char* clientId)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->setDefaultClientId(clientId);
}

void mqttsn_gw_session_set_pub_only_keep_alive(
    MqttsnSessionHandle session,
    unsigned value)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->setPubOnlyKeepAlive(value);
}

bool mqttsn_gw_session_start(MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return false;
    }

    return reinterpret_cast<Session*>(session)->start();
}

void mqttsn_gw_session_stop(MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->stop();
}

void mqttsn_gw_session_tick(MqttsnSessionHandle session)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->tick();
}

unsigned mqttsn_gw_session_data_from_client(
    MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen)
{
    if (session == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<Session*>(session)->dataFromClient(buf, bufLen));

}

unsigned mqttsn_gw_session_data_from_broker(
    MqttsnSessionHandle session,
    const unsigned char* buf,
    unsigned bufLen)
{
    if (session == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<Session*>(session)->dataFromBroker(buf, bufLen));

}

void mqttsn_gw_session_broker_connected(MqttsnSessionHandle session, bool connected)
{
    if (session == nullptr) {
        return;
    }

    reinterpret_cast<Session*>(session)->setBrokerConnected(connected);
}

bool mqttsn_gw_session_add_predefined_topic(
    MqttsnSessionHandle session,
    const char* topic,
    unsigned short topicId)
{
    if (session == nullptr) {
        return false;
    }

    return reinterpret_cast<Session*>(session)->addPredefinedTopic(topic, topicId);
}

bool mqttsn_gw_session_set_topic_id_alloc_range(
    MqttsnSessionHandle session,
    unsigned short minTopicId,
    unsigned short maxTopicId)
{
    if (session == nullptr) {
        return false;
    }

    return reinterpret_cast<Session*>(session)->setTopicIdAllocationRange(minTopicId, maxTopicId);
}

/*===================== Config Object ======================*/

MqttsnConfigHandle mqttsn_gw_config_alloc(void)
{
    return new Config;
}

void mqttsn_gw_config_free(MqttsnConfigHandle config)
{
    std::unique_ptr<Config>(reinterpret_cast<Config*>(config));
}

void mqttsn_gw_config_parse(MqttsnConfigHandle config, const char* str)
{
    if (config == nullptr) {
        return;
    }

    std::stringstream stream;
    stream << str;
    reinterpret_cast<Config*>(config)->read(stream);
}

bool mqttsn_gw_config_read(MqttsnConfigHandle config, const char* filename)
{
    if (config == nullptr) {
        return false;
    }

    std::ifstream stream(filename, std::ios_base::in);
    if (!stream) {
        return false;
    }

    reinterpret_cast<Config*>(config)->read(stream);
    return true;
}

unsigned char mqttsn_gw_config_id(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->gatewayId();
}

unsigned short mqttsn_gw_config_advertise_period(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->advertisePeriod();
}

unsigned mqttsn_gw_config_retry_period(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->retryPeriod();
}

unsigned mqttsn_gw_config_retry_count(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->retryCount();
}

const char* mqttsn_gw_config_default_client_id(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const Config*>(config)->defaultClientId().c_str();
}

unsigned mqttsn_gw_config_pub_only_keep_alive(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->pubOnlyKeepAlive();
}

unsigned mqttsn_gw_config_sleepin_client_msg_limit(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return std::numeric_limits<unsigned>::max();
    }

    return static_cast<unsigned>(
        reinterpret_cast<const Config*>(config)->sleepingClientMsgLimit());
}

unsigned mqttsn_gw_config_available_predefined_topics(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<const Config*>(config)->predefinedTopics().size());
}

unsigned mqttsn_gw_config_get_predefined_topics(
    MqttsnConfigHandle config,
    MqttsnPredefinedTopicInfo* buf,
    unsigned bufLen)
{
    if (config == nullptr) {
        return 0U;
    }

    auto& predefinedTopics = reinterpret_cast<const Config*>(config)->predefinedTopics();
    auto total = std::min(static_cast<unsigned>(predefinedTopics.size()), bufLen);

    std::transform(
        predefinedTopics.begin(), predefinedTopics.begin() + total, buf,
        [](const mqttsn::gateway::Config::PredefinedTopicInfo& info) -> MqttsnPredefinedTopicInfo
        {
            MqttsnPredefinedTopicInfo retInfo;
            retInfo.clientId = info.clientId.c_str();
            retInfo.topic = info.topic.c_str();
            retInfo.topicId = info.topicId;
            return retInfo;
        });
    return total;
}

unsigned mqttsn_gw_config_available_auth_infos(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return static_cast<unsigned>(
        reinterpret_cast<const Config*>(config)->authInfos().size());
}

unsigned mqttsn_gw_config_get_auth_infos(
    MqttsnConfigHandle config,
    MqttsnAuthInfo* buf,
    unsigned bufLen)
{
    if (config == nullptr) {
        return 0U;
    }

    auto& authInfos = reinterpret_cast<const Config*>(config)->authInfos();
    auto total = std::min(static_cast<unsigned>(authInfos.size()), bufLen);

    std::transform(
        authInfos.begin(), authInfos.begin() + total, buf,
        [](const mqttsn::gateway::Config::AuthInfo& info) -> MqttsnAuthInfo
        {
            MqttsnAuthInfo retInfo;
            retInfo.clientId = info.clientId.c_str();
            retInfo.username = info.username.c_str();
            retInfo.password = info.password.c_str();
            return retInfo;
        });
    return total;
}

void mqttsn_gw_config_topic_id_alloc_range(
    MqttsnConfigHandle config,
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

const char* mqttsn_gw_config_broker_address(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<const Config*>(config)->brokerTcpHostAddress().c_str();
}

unsigned short mqttsn_gw_config_broker_port(MqttsnConfigHandle config)
{
    if (config == nullptr) {
        return 0U;
    }

    return reinterpret_cast<const Config*>(config)->brokerTcpHostPort();
}

unsigned mqttsn_gw_config_get_value(
    MqttsnConfigHandle config,
    const char* key,
    char* buf,
    unsigned bufLen)
{
    if (config == nullptr) {
        return 0U;
    }

    auto& map = reinterpret_cast<const Config*>(config)->configMap();
    auto iter = map.find(key);
    if (iter == map.end()) {
        return 0U;
    }

    auto maxLen = std::min(bufLen - 1U, static_cast<unsigned>(iter->second.size()));
    std::copy_n(iter->second.begin(), maxLen, buf);
    buf[maxLen] = '\0';
    return maxLen + 1;
}


