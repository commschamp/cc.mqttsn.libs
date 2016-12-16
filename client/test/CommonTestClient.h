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


#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <cstdint>

#include "mqttsn/client/common.h"
#include "mqttsn/protocol/field.h"
#include "client.h"

typedef decltype(&mqttsn_client_new) ClientNewFunc;
typedef decltype(&mqttsn_client_free) ClientFreeFunc;
typedef decltype(&mqttsn_client_set_next_tick_program_callback) NextTickProgramCallbackSetFunc;
typedef decltype(&mqttsn_client_set_cancel_next_tick_wait_callback) CancelNextTickCallbackSetFunc;
typedef decltype(&mqttsn_client_set_send_output_data_callback) SendOutDataCallbackSetFunc;
typedef decltype(&mqttsn_client_set_gw_status_report_callback) GwStatusReportCallbackSetFunc;
typedef decltype(&mqttsn_client_set_gw_disconnect_report_callback) GwDisconnectsReportCallbackSetFunc;
typedef decltype(&mqttsn_client_set_message_report_callback) MessageReportCallbackSetFunc;
typedef decltype(&mqttsn_client_start) StartFunc;
typedef decltype(&mqttsn_client_process_data) ProcessDataFunc;
typedef decltype(&mqttsn_client_tick) TickFunc;
typedef decltype(&mqttsn_client_set_retry_period) SetRetryPeriodFunc;
typedef decltype(&mqttsn_client_set_retry_count) SetRetryCountFunc;
typedef decltype(&mqttsn_client_set_broadcast_radius) SetBroadcastRadiusFunc;
typedef decltype(&mqttsn_client_set_searchgw_enabled) SetSearchgwEnabledFunc;
typedef decltype(&mqttsn_client_cancel) CancelFunc;
typedef decltype(&mqttsn_client_connect) ConnectFunc;
typedef decltype(&mqttsn_client_disconnect) DisconnectFunc;
typedef decltype(&mqttsn_client_publish_id) PublishIdFunc;
typedef decltype(&mqttsn_client_publish) PublishFunc;
typedef decltype(&mqttsn_client_subscribe_id) SubscribeIdFunc;
typedef decltype(&mqttsn_client_subscribe) SubscribeFunc;
typedef decltype(&mqttsn_client_unsubscribe_id) UnsubscribeIdFunc;
typedef decltype(&mqttsn_client_unsubscribe) UnsubscribeFunc;
typedef decltype(&mqttsn_client_will_update) WillUpdateFunc;
typedef decltype(&mqttsn_client_will_topic_update) WillTopicUpdateFunc;
typedef decltype(&mqttsn_client_will_msg_update) WillMsgUpdateFunc;
typedef decltype(&mqttsn_client_sleep) SleepFunc;
typedef decltype(&mqttsn_client_reconnect) ReconnectFunc;
typedef decltype(&mqttsn_client_check_messages) CheckMessagesFunc;


struct ClientLibFuncs
{
    ClientNewFunc m_newFunc = nullptr;
    ClientFreeFunc m_freeFunc = nullptr;
    NextTickProgramCallbackSetFunc m_nextTickProgramCallbackSetFunc = nullptr;
    CancelNextTickCallbackSetFunc m_cancelNextTickCallbackSetFunc = nullptr;
    SendOutDataCallbackSetFunc m_sentOutDataCallbackSetFunc = nullptr;
    GwStatusReportCallbackSetFunc m_gwStatusReportCallbackSetFunc = nullptr;
    GwDisconnectsReportCallbackSetFunc m_gwDisconnectReportCallbackSetFunc = nullptr;
    MessageReportCallbackSetFunc m_msgReportCallbackSetFunc = nullptr;
    StartFunc m_startFunc = nullptr;
    ProcessDataFunc m_processDataFunc = nullptr;
    TickFunc m_tickFunc = nullptr;
    SetRetryPeriodFunc m_setRetryPeriodFunc = nullptr;
    SetRetryCountFunc m_setRetryCountFunc = nullptr;
    SetBroadcastRadiusFunc m_setBroadcastRadius = nullptr;
    SetSearchgwEnabledFunc m_setSearchgwEnabledFunc = nullptr;
    CancelFunc m_cancelFunc = nullptr;
    ConnectFunc m_connectFunc = nullptr;
    DisconnectFunc m_disconnectFunc = nullptr;
    PublishIdFunc m_publishIdFunc = nullptr;
    PublishFunc m_publishFunc = nullptr;
    SubscribeIdFunc m_subscribeIdFunc = nullptr;
    SubscribeFunc m_subscribeFunc = nullptr;
    UnsubscribeIdFunc m_unsubscribeIdFunc = nullptr;
    UnsubscribeFunc m_unsubscribeFunc = nullptr;
    WillUpdateFunc m_willUpdateFunc = nullptr;
    WillTopicUpdateFunc m_willTopicUpdateFunc = nullptr;
    WillMsgUpdateFunc m_willMsgUpdateFunc = nullptr;
    SleepFunc m_sleepFunc = nullptr;
    ReconnectFunc m_reconnectFunc = nullptr;
    CheckMessagesFunc m_checkMessagesFunc = nullptr;
};

class CommonTestClient
{
public:
    typedef std::unique_ptr<CommonTestClient> Ptr;
    typedef std::function<void (unsigned)> ProgramNextTickCallback;
    typedef std::function<unsigned ()> CancelNextTickCallback;
    typedef std::function<void (const std::uint8_t* buf, unsigned bufLen, bool broadcast)> SendDataCallback;
    typedef std::function<void (unsigned short gwId, MqttsnGwStatus status)> GwStatusReportCallback;
    typedef std::function<void ()> GwDisconnectReportCallback;
    typedef std::function<void (const MqttsnMessageInfo& msgInfo)> MessageReportCallback;
    typedef std::function<void (MqttsnAsyncOpStatus status)> AsyncOpCompleteCallback;
    typedef std::function<void (MqttsnAsyncOpStatus status, MqttsnQoS qos)> SubscribeCompleteCallback;

    ~CommonTestClient();

    ProgramNextTickCallback setProgramNextTickCallback(ProgramNextTickCallback&& func);
    CancelNextTickCallback setCancelNextTickCallback(CancelNextTickCallback&& func);
    SendDataCallback setSendDataCallback(SendDataCallback&& func);
    GwStatusReportCallback setGwStatusReportCallback(GwStatusReportCallback&& func);
    GwDisconnectReportCallback setGwDisconnectReportCallback(GwDisconnectReportCallback&& func);
    MessageReportCallback setMessageReportCallback(MessageReportCallback&& func);
    AsyncOpCompleteCallback setConnectCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setDisconnectCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setPublishCompleteCallback(AsyncOpCompleteCallback&& func);
    SubscribeCompleteCallback setSubsribeCompleteCallback(SubscribeCompleteCallback&& func);
    AsyncOpCompleteCallback setUnsubsribeCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setWillUpdateCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setWillTopicUpdateCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setWillMsgUpdateCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setSleepCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setReconnectCompleteCallback(AsyncOpCompleteCallback&& func);
    AsyncOpCompleteCallback setCheckMessagesCompleteCallback(AsyncOpCompleteCallback&& func);

    static Ptr alloc(const ClientLibFuncs& libFuncs = DefaultFuncs);
    MqttsnErrorCode start();
    void inputData(const std::uint8_t* buf, std::size_t bufLen);
    void tick();
    void setRetryPeriod(unsigned ms);
    void setRetryCount(unsigned value);
    void setBroadcastRadius(unsigned char val);
    void setSearchgwEnabled(bool value);

    bool cancel();
    MqttsnErrorCode connect(
        const char* clientId,
        unsigned short keepAliveSeconds,
        bool cleanSession,
        const MqttsnWillInfo* willInfo);

    MqttsnErrorCode disconnect();

    MqttsnErrorCode publishId(
        MqttsnTopicId topicId,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain);

    MqttsnErrorCode publish(
        const std::string& topic,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain);

    MqttsnErrorCode subscribe(
        const std::string& topic,
        MqttsnQoS qos);

    MqttsnErrorCode subscribe(
        MqttsnTopicId topicId,
        MqttsnQoS qos);

    MqttsnErrorCode unsubscribe(const std::string& topic);

    MqttsnErrorCode unsubscribe(MqttsnTopicId topicId);

    MqttsnErrorCode willUpdate(
        const MqttsnWillInfo* willInfo);

    MqttsnErrorCode willTopicUpdate(
        const std::string& topic,
        MqttsnQoS qos,
        bool retain);

    MqttsnErrorCode willMsgUpdate(
        const std::uint8_t* msg,
        std::size_t msgLen);

    MqttsnErrorCode sleep(std::uint16_t duration);

    MqttsnErrorCode reconnect();

    MqttsnErrorCode checkMessages();

    static MqttsnQoS transformQos(mqttsn::protocol::field::QosType val);
    static mqttsn::protocol::field::QosType transformQos(MqttsnQoS val);

private:
    typedef std::vector<std::uint8_t> InputData;

    CommonTestClient(const ClientLibFuncs& libFuncs);

    void programNextTick(unsigned duration);
    unsigned cancelNextTick();
    void sendOutputData(const unsigned char* buf, unsigned bufLen, bool broadcast);
    void reportGwStatus(unsigned short gwId, MqttsnGwStatus status);
    void reportGwDisconnect();
    void reportMessage(const MqttsnMessageInfo* msgInfo);
    void reportConnectComplete(MqttsnAsyncOpStatus status);
    void reportDisconnectComplete(MqttsnAsyncOpStatus status);
    void reportPublishComplete(MqttsnAsyncOpStatus status);
    void reportSubsribeComplete(MqttsnAsyncOpStatus status, MqttsnQoS qos);
    void reportUnsubsribeComplete(MqttsnAsyncOpStatus status);
    void reportWillUpdateComplete(MqttsnAsyncOpStatus status);
    void reportWillTopicUpdateComplete(MqttsnAsyncOpStatus status);
    void reportWillMsgUpdateComplete(MqttsnAsyncOpStatus status);
    void reportSleepComplete(MqttsnAsyncOpStatus status);
    void reportReconnectComplete(MqttsnAsyncOpStatus status);
    void reportCheckMessagesComplete(MqttsnAsyncOpStatus status);

    static void nextTickProgramCallback(void* data, unsigned duration);
    static unsigned cancelNextTickCallback(void* data);
    static void sendOutputDataCallback(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);
    static void gwStatusReportCallback(void* data, unsigned char gwId, MqttsnGwStatus status);
    static void gwDisconnectReportCallback(void* data);
    static void msgReportCallback(void* data, const MqttsnMessageInfo* msgInfo);
    static void connectCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void disconnectCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void publishCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void subsribeCompleteCallback(void* data, MqttsnAsyncOpStatus status, MqttsnQoS qos);
    static void unsubsribeCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void willUpdateCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void willTopicUpdateCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void willMsgUpdateCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void sleepCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void reconnectCompleteCallback(void* data, MqttsnAsyncOpStatus status);
    static void checkMessagesCompleteCallback(void* data, MqttsnAsyncOpStatus status);

    ClientLibFuncs m_libFuncs;
    MqttsnClientHandle m_client = nullptr;
    InputData m_inData;

    ProgramNextTickCallback m_programNextTickCallback;
    CancelNextTickCallback m_cancelNextTickCallback;
    SendDataCallback m_sendDataCallback;
    GwStatusReportCallback m_gwStatusReportCallback;
    GwDisconnectReportCallback m_gwDisconnectReportCallback;
    MessageReportCallback m_msgReportCallback;
    AsyncOpCompleteCallback m_connectCompleteCallback;
    AsyncOpCompleteCallback m_disconnectCompleteCallback;
    AsyncOpCompleteCallback m_publishCompleteCallback;
    SubscribeCompleteCallback m_subscribeCompleteCallback;
    AsyncOpCompleteCallback m_unsubscribeCompleteCallback;
    AsyncOpCompleteCallback m_willUpdateCompleteCallback;
    AsyncOpCompleteCallback m_willTopicUpdateCompleteCallback;
    AsyncOpCompleteCallback m_willMsgUpdateCompleteCallback;
    AsyncOpCompleteCallback m_sleepCompleteCallback;
    AsyncOpCompleteCallback m_reconnectCompleteCallback;
    AsyncOpCompleteCallback m_checkMessagesCompleteCallback;

    static const ClientLibFuncs DefaultFuncs;
};
