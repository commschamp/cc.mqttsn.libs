//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <cstdint>
#include <string>

#include "cc_mqttsn_client/common.h"
#include "cc_mqttsn/field/QosCommon.h"
#include "client.h"

typedef decltype(&cc_mqttsn_client_new) ClientNewFunc;
typedef decltype(&cc_mqttsn_client_free) ClientFreeFunc;
typedef decltype(&cc_mqttsn_client_set_next_tick_program_callback) NextTickProgramCallbackSetFunc;
typedef decltype(&cc_mqttsn_client_set_cancel_next_tick_wait_callback) CancelNextTickCallbackSetFunc;
typedef decltype(&cc_mqttsn_client_set_send_output_data_callback) SendOutDataCallbackSetFunc;
typedef decltype(&cc_mqttsn_client_set_gw_status_report_callback) GwStatusReportCallbackSetFunc;
typedef decltype(&cc_mqttsn_client_set_gw_disconnect_report_callback) GwDisconnectsReportCallbackSetFunc;
typedef decltype(&cc_mqttsn_client_set_message_report_callback) MessageReportCallbackSetFunc;
typedef decltype(&cc_mqttsn_client_start) StartFunc;
typedef decltype(&cc_mqttsn_client_process_data) ProcessDataFunc;
typedef decltype(&cc_mqttsn_client_tick) TickFunc;
typedef decltype(&cc_mqttsn_client_set_retry_period) SetRetryPeriodFunc;
typedef decltype(&cc_mqttsn_client_set_retry_count) SetRetryCountFunc;
typedef decltype(&cc_mqttsn_client_set_broadcast_radius) SetBroadcastRadiusFunc;
typedef decltype(&cc_mqttsn_client_set_searchgw_enabled) SetSearchgwEnabledFunc;
typedef decltype(&cc_mqttsn_client_cancel) CancelFunc;
typedef decltype(&cc_mqttsn_client_connect) ConnectFunc;
typedef decltype(&cc_mqttsn_client_disconnect) DisconnectFunc;
typedef decltype(&cc_mqttsn_client_publish_id) PublishIdFunc;
typedef decltype(&cc_mqttsn_client_publish) PublishFunc;
typedef decltype(&cc_mqttsn_client_subscribe_id) SubscribeIdFunc;
typedef decltype(&cc_mqttsn_client_subscribe) SubscribeFunc;
typedef decltype(&cc_mqttsn_client_unsubscribe_id) UnsubscribeIdFunc;
typedef decltype(&cc_mqttsn_client_unsubscribe) UnsubscribeFunc;
typedef decltype(&cc_mqttsn_client_will_update) WillUpdateFunc;
typedef decltype(&cc_mqttsn_client_will_topic_update) WillTopicUpdateFunc;
typedef decltype(&cc_mqttsn_client_will_msg_update) WillMsgUpdateFunc;
typedef decltype(&cc_mqttsn_client_sleep) SleepFunc;
typedef decltype(&cc_mqttsn_client_reconnect) ReconnectFunc;
typedef decltype(&cc_mqttsn_client_check_messages) CheckMessagesFunc;


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
    typedef std::function<void (unsigned short gwId, CC_MqttsnGwStatus status)> GwStatusReportCallback;
    typedef std::function<void ()> GwDisconnectReportCallback;
    typedef std::function<void (const CC_MqttsnMessageInfo& msgInfo)> MessageReportCallback;
    typedef std::function<void (CC_MqttsnAsyncOpStatus status)> AsyncOpCompleteCallback;
    typedef std::function<void (CC_MqttsnAsyncOpStatus status, CC_MqttsnQoS qos)> SubscribeCompleteCallback;

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
    CC_MqttsnErrorCode start();
    void inputData(const std::uint8_t* buf, std::size_t bufLen);
    void tick();
    void setRetryPeriod(unsigned ms);
    void setRetryCount(unsigned value);
    void setBroadcastRadius(unsigned char val);
    void setSearchgwEnabled(bool value);

    bool cancel();
    CC_MqttsnErrorCode connect(
        const char* clientId,
        unsigned short keepAliveSeconds,
        bool cleanSession,
        const CC_MqttsnWillInfo* willInfo);

    CC_MqttsnErrorCode disconnect();

    CC_MqttsnErrorCode publishId(
        CC_MqttsnTopicId topicId,
        const std::uint8_t* msg,
        std::size_t msgLen,
        CC_MqttsnQoS qos,
        bool retain);

    CC_MqttsnErrorCode publish(
        const std::string& topic,
        const std::uint8_t* msg,
        std::size_t msgLen,
        CC_MqttsnQoS qos,
        bool retain);

    CC_MqttsnErrorCode subscribe(
        const std::string& topic,
        CC_MqttsnQoS qos);

    CC_MqttsnErrorCode subscribe(
        CC_MqttsnTopicId topicId,
        CC_MqttsnQoS qos);

    CC_MqttsnErrorCode unsubscribe(const std::string& topic);

    CC_MqttsnErrorCode unsubscribe(CC_MqttsnTopicId topicId);

    CC_MqttsnErrorCode willUpdate(
        const CC_MqttsnWillInfo* willInfo);

    CC_MqttsnErrorCode willTopicUpdate(
        const std::string& topic,
        CC_MqttsnQoS qos,
        bool retain);

    CC_MqttsnErrorCode willMsgUpdate(
        const std::uint8_t* msg,
        std::size_t msgLen);

    CC_MqttsnErrorCode sleep(std::uint16_t duration);

    CC_MqttsnErrorCode reconnect();

    CC_MqttsnErrorCode checkMessages();

    static CC_MqttsnQoS transformQos(cc_mqttsn::field::QosVal val);
    static cc_mqttsn::field::QosVal transformQos(CC_MqttsnQoS val);

private:
    typedef std::vector<std::uint8_t> InputData;

    CommonTestClient(const ClientLibFuncs& libFuncs);

    void programNextTick(unsigned duration);
    unsigned cancelNextTick();
    void sendOutputData(const unsigned char* buf, unsigned bufLen, bool broadcast);
    void reportGwStatus(unsigned short gwId, CC_MqttsnGwStatus status);
    void reportGwDisconnect();
    void reportMessage(const CC_MqttsnMessageInfo* msgInfo);
    void reportConnectComplete(CC_MqttsnAsyncOpStatus status);
    void reportDisconnectComplete(CC_MqttsnAsyncOpStatus status);
    void reportPublishComplete(CC_MqttsnAsyncOpStatus status);
    void reportSubsribeComplete(CC_MqttsnAsyncOpStatus status, CC_MqttsnQoS qos);
    void reportUnsubsribeComplete(CC_MqttsnAsyncOpStatus status);
    void reportWillUpdateComplete(CC_MqttsnAsyncOpStatus status);
    void reportWillTopicUpdateComplete(CC_MqttsnAsyncOpStatus status);
    void reportWillMsgUpdateComplete(CC_MqttsnAsyncOpStatus status);
    void reportSleepComplete(CC_MqttsnAsyncOpStatus status);
    void reportReconnectComplete(CC_MqttsnAsyncOpStatus status);
    void reportCheckMessagesComplete(CC_MqttsnAsyncOpStatus status);

    static void nextTickProgramCallback(void* data, unsigned duration);
    static unsigned cancelNextTickCallback(void* data);
    static void sendOutputDataCallback(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);
    static void gwStatusReportCallback(void* data, unsigned char gwId, CC_MqttsnGwStatus status);
    static void gwDisconnectReportCallback(void* data);
    static void msgReportCallback(void* data, const CC_MqttsnMessageInfo* msgInfo);
    static void connectCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void disconnectCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void publishCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void subsribeCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status, CC_MqttsnQoS qos);
    static void unsubsribeCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void willUpdateCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void willTopicUpdateCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void willMsgUpdateCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void sleepCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void reconnectCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);
    static void checkMessagesCompleteCallback(void* data, CC_MqttsnAsyncOpStatus status);

    ClientLibFuncs m_libFuncs;
    CC_MqttsnClientHandle m_client;
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
