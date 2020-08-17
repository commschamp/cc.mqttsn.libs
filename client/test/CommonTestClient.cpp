//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "CommonTestClient.h"

#include <cassert>
#include <iostream>

namespace
{

ClientLibFuncs createDefaultLibFuncs()
{
    ClientLibFuncs funcs;
    funcs.m_newFunc = &mqttsn_client_new;
    funcs.m_freeFunc = &mqttsn_client_free;
    funcs.m_nextTickProgramCallbackSetFunc = &mqttsn_client_set_next_tick_program_callback;
    funcs.m_cancelNextTickCallbackSetFunc = &mqttsn_client_set_cancel_next_tick_wait_callback;
    funcs.m_sentOutDataCallbackSetFunc = &mqttsn_client_set_send_output_data_callback;
    funcs.m_gwStatusReportCallbackSetFunc = &mqttsn_client_set_gw_status_report_callback;
    funcs.m_gwDisconnectReportCallbackSetFunc = &mqttsn_client_set_gw_disconnect_report_callback;
    funcs.m_msgReportCallbackSetFunc = &mqttsn_client_set_message_report_callback;
    funcs.m_startFunc = &mqttsn_client_start;
    funcs.m_processDataFunc = &mqttsn_client_process_data;
    funcs.m_tickFunc = &mqttsn_client_tick;
    funcs.m_setRetryPeriodFunc = &mqttsn_client_set_retry_period;
    funcs.m_setRetryCountFunc = &mqttsn_client_set_retry_count;
    funcs.m_setBroadcastRadius = &mqttsn_client_set_broadcast_radius;
    funcs.m_setSearchgwEnabledFunc = &mqttsn_client_set_searchgw_enabled;
    funcs.m_cancelFunc = &mqttsn_client_cancel;
    funcs.m_connectFunc = &mqttsn_client_connect;
    funcs.m_disconnectFunc = &mqttsn_client_disconnect;
    funcs.m_publishIdFunc = &mqttsn_client_publish_id;
    funcs.m_publishFunc = &mqttsn_client_publish;
    funcs.m_subscribeIdFunc = &mqttsn_client_subscribe_id;
    funcs.m_subscribeFunc = &mqttsn_client_subscribe;
    funcs.m_unsubscribeIdFunc = &mqttsn_client_unsubscribe_id;
    funcs.m_unsubscribeFunc = &mqttsn_client_unsubscribe;
    funcs.m_willUpdateFunc = &mqttsn_client_will_update;
    funcs.m_willTopicUpdateFunc = &mqttsn_client_will_topic_update;
    funcs.m_willMsgUpdateFunc = &mqttsn_client_will_msg_update;
    funcs.m_sleepFunc = &mqttsn_client_sleep;
    funcs.m_reconnectFunc = &mqttsn_client_reconnect;
    funcs.m_checkMessagesFunc = &mqttsn_client_check_messages;
    return funcs;
}

}  // namespace

const ClientLibFuncs CommonTestClient::DefaultFuncs = createDefaultLibFuncs();

CommonTestClient::~CommonTestClient()
{
    assert(m_libFuncs.m_freeFunc != nullptr);
    m_libFuncs.m_freeFunc(m_client);
}

CommonTestClient::ProgramNextTickCallback CommonTestClient::setProgramNextTickCallback(
    ProgramNextTickCallback&& func)
{
    ProgramNextTickCallback old(std::move(m_programNextTickCallback));
    m_programNextTickCallback = std::move(func);
    return old;
}

CommonTestClient::CancelNextTickCallback CommonTestClient::setCancelNextTickCallback(
    CancelNextTickCallback&& func)
{
    CancelNextTickCallback old(std::move(m_cancelNextTickCallback));
    m_cancelNextTickCallback = std::move(func);
    return old;
}

CommonTestClient::SendDataCallback CommonTestClient::setSendDataCallback(
    SendDataCallback&& func)
{
    SendDataCallback old(std::move(m_sendDataCallback));
    m_sendDataCallback = std::move(func);
    return old;
}

CommonTestClient::GwStatusReportCallback CommonTestClient::setGwStatusReportCallback(
    GwStatusReportCallback&& func)
{
    GwStatusReportCallback old(std::move(m_gwStatusReportCallback));
    m_gwStatusReportCallback = std::move(func);
    return old;
}

CommonTestClient::GwDisconnectReportCallback CommonTestClient::setGwDisconnectReportCallback(
    GwDisconnectReportCallback&& func)
{
    GwDisconnectReportCallback old(std::move(m_gwDisconnectReportCallback));
    m_gwDisconnectReportCallback = std::move(func);
    return old;
}

CommonTestClient::MessageReportCallback CommonTestClient::setMessageReportCallback(
    MessageReportCallback&& func)
{
    MessageReportCallback old(std::move(m_msgReportCallback));
    m_msgReportCallback = std::move(func);
    return old;
}
CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setConnectCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_connectCompleteCallback));
    m_connectCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setDisconnectCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_disconnectCompleteCallback));
    m_disconnectCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setPublishCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_publishCompleteCallback));
    m_publishCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::SubscribeCompleteCallback CommonTestClient::setSubsribeCompleteCallback(
    SubscribeCompleteCallback&& func)
{
    SubscribeCompleteCallback old(std::move(m_subscribeCompleteCallback));
    m_subscribeCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setUnsubsribeCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_unsubscribeCompleteCallback));
    m_unsubscribeCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setWillUpdateCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_willUpdateCompleteCallback));
    m_willUpdateCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setWillTopicUpdateCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_willTopicUpdateCompleteCallback));
    m_willTopicUpdateCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setWillMsgUpdateCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_willMsgUpdateCompleteCallback));
    m_willMsgUpdateCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setSleepCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_sleepCompleteCallback));
    m_sleepCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setReconnectCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_reconnectCompleteCallback));
    m_reconnectCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::AsyncOpCompleteCallback CommonTestClient::setCheckMessagesCompleteCallback(
    AsyncOpCompleteCallback&& func)
{
    AsyncOpCompleteCallback old(std::move(m_checkMessagesCompleteCallback));
    m_checkMessagesCompleteCallback = std::move(func);
    return old;
}

CommonTestClient::Ptr CommonTestClient::alloc(const ClientLibFuncs& libFuncs)
{
    return Ptr(new CommonTestClient(libFuncs));
}

MqttsnErrorCode CommonTestClient::start()
{
    assert(m_libFuncs.m_startFunc != nullptr);
    return (m_libFuncs.m_startFunc)(m_client);
}

void CommonTestClient::inputData(const std::uint8_t* buf, std::size_t bufLen)
{
    assert(m_inData.empty());
    m_inData.insert(m_inData.end(), buf, buf + bufLen);
    assert(m_libFuncs.m_processDataFunc != nullptr);
    assert(!m_inData.empty());
    unsigned count = (m_libFuncs.m_processDataFunc)(m_client, &m_inData[0], m_inData.size());
    if (m_inData.size() < count) {
        std::cout << "Processed " << count << " bytes, while having only " << m_inData.size() << std::endl;
    }
    assert(count <= m_inData.size());
    m_inData.erase(m_inData.begin(), m_inData.begin() + count);
}

void CommonTestClient::tick()
{
    assert(m_libFuncs.m_tickFunc != nullptr);
    (m_libFuncs.m_tickFunc)(m_client);
}

void CommonTestClient::setRetryPeriod(unsigned ms)
{
    assert(m_libFuncs.m_setRetryPeriodFunc != nullptr);
    (m_libFuncs.m_setRetryPeriodFunc)(m_client, ms);
}

void CommonTestClient::setRetryCount(unsigned value)
{
    assert(m_libFuncs.m_setRetryCountFunc != nullptr);
    (m_libFuncs.m_setRetryCountFunc)(m_client, value);
}

void CommonTestClient::setBroadcastRadius(unsigned char val)
{
    assert(m_libFuncs.m_setBroadcastRadius != nullptr);
    (m_libFuncs.m_setBroadcastRadius)(m_client, val);
}

void CommonTestClient::setSearchgwEnabled(bool value)
{
    assert(m_libFuncs.m_setSearchgwEnabledFunc != nullptr);
    (m_libFuncs.m_setSearchgwEnabledFunc)(m_client, value);
}

MqttsnErrorCode CommonTestClient::connect(
    const char* clientId,
    unsigned short keepAliveSeconds,
    bool cleanSession,
    const MqttsnWillInfo* willInfo)
{
    assert(m_libFuncs.m_connectFunc != nullptr);

    return
        (m_libFuncs.m_connectFunc)(
            m_client,
            clientId,
            keepAliveSeconds,
            cleanSession,
            willInfo,
            &CommonTestClient::connectCompleteCallback,
            this);
}

bool CommonTestClient::cancel()
{
    assert(m_libFuncs.m_cancelFunc != nullptr);
    return (m_libFuncs.m_cancelFunc)(m_client);
}

MqttsnErrorCode CommonTestClient::disconnect()
{
    assert(m_libFuncs.m_disconnectFunc != nullptr);
    return (m_libFuncs.m_disconnectFunc)(
        m_client,
        &CommonTestClient::disconnectCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::publishId(
    MqttsnTopicId topicId,
    const std::uint8_t* msg,
    std::size_t msgLen,
    MqttsnQoS qos,
    bool retain)
{
    assert(m_libFuncs.m_publishIdFunc != nullptr);
    return (m_libFuncs.m_publishIdFunc)(
        m_client,
        topicId,
        msg,
        msgLen,
        qos,
        retain,
        &CommonTestClient::publishCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::publish(
    const std::string& topic,
    const std::uint8_t* msg,
    std::size_t msgLen,
    MqttsnQoS qos,
    bool retain)
{
    assert(m_libFuncs.m_publishFunc != nullptr);
    return (m_libFuncs.m_publishFunc)(
        m_client,
        topic.c_str(),
        msg,
        msgLen,
        qos,
        retain,
        &CommonTestClient::publishCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::subscribe(
    const std::string& topic,
    MqttsnQoS qos)
{
    assert(m_libFuncs.m_subscribeFunc != nullptr);
    return (m_libFuncs.m_subscribeFunc)(
        m_client,
        topic.c_str(),
        qos,
        &CommonTestClient::subsribeCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::subscribe(
    MqttsnTopicId topicId,
    MqttsnQoS qos)
{
    assert(m_libFuncs.m_subscribeIdFunc != nullptr);
    return (m_libFuncs.m_subscribeIdFunc)(
        m_client,
        topicId,
        qos,
        &CommonTestClient::subsribeCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::unsubscribe(const std::string& topic)
{
    assert(m_libFuncs.m_unsubscribeFunc != nullptr);
    return (m_libFuncs.m_unsubscribeFunc)(
        m_client,
        topic.c_str(),
        &CommonTestClient::unsubsribeCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::unsubscribe(MqttsnTopicId topicId)
{
    assert(m_libFuncs.m_unsubscribeIdFunc != nullptr);
    return (m_libFuncs.m_unsubscribeIdFunc)(
        m_client,
        topicId,
        &CommonTestClient::unsubsribeCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::willUpdate(
    const MqttsnWillInfo* willInfo)
{
    assert(m_libFuncs.m_willUpdateFunc != nullptr);
    return (m_libFuncs.m_willUpdateFunc)(
        m_client,
        willInfo,
        &CommonTestClient::willUpdateCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::willTopicUpdate(
    const std::string& topic,
    MqttsnQoS qos,
    bool retain)
{
    assert(m_libFuncs.m_willTopicUpdateFunc != nullptr);
    return (m_libFuncs.m_willTopicUpdateFunc)(
        m_client,
        topic.c_str(),
        qos,
        retain,
        &CommonTestClient::willTopicUpdateCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::willMsgUpdate(
    const std::uint8_t* msg,
    std::size_t msgLen)
{
    assert(m_libFuncs.m_willMsgUpdateFunc != nullptr);
    return (m_libFuncs.m_willMsgUpdateFunc)(
        m_client,
        msg,
        msgLen,
        &CommonTestClient::willMsgUpdateCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::sleep(std::uint16_t duration)
{
    assert(m_libFuncs.m_sleepFunc != nullptr);
    return (m_libFuncs.m_sleepFunc)(
        m_client,
        duration,
        &CommonTestClient::sleepCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::reconnect()
{
    assert(m_libFuncs.m_reconnectFunc != nullptr);
    return (m_libFuncs.m_reconnectFunc)(
        m_client,
        &CommonTestClient::reconnectCompleteCallback,
        this);
}

MqttsnErrorCode CommonTestClient::checkMessages()
{
    assert(m_libFuncs.m_checkMessagesFunc != nullptr);
    return (m_libFuncs.m_checkMessagesFunc)(
        m_client,
        &CommonTestClient::checkMessagesCompleteCallback,
        this);
}

MqttsnQoS CommonTestClient::transformQos(mqttsn::field::QosVal val)
{
    static_assert(
        (int)mqttsn::field::QosVal::AtMostOnceDelivery == MqttsnQoS_AtMostOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::field::QosVal::AtLeastOnceDelivery == MqttsnQoS_AtLeastOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::field::QosVal::ExactlyOnceDelivery == MqttsnQoS_ExactlyOnceDelivery,
        "Invalid mapping");

    if (val == mqttsn::field::QosVal::NoGwPublish) {
        return MqttsnQoS_NoGwPublish;
    }

    return static_cast<MqttsnQoS>(val);
}

mqttsn::field::QosVal CommonTestClient::transformQos(MqttsnQoS val)
{

    if (val == MqttsnQoS_NoGwPublish) {
        return mqttsn::field::QosVal::NoGwPublish;
    }

    return static_cast<mqttsn::field::QosVal>(val);
}

CommonTestClient::CommonTestClient(const ClientLibFuncs& libFuncs)
  : m_libFuncs(libFuncs),
    m_client((libFuncs.m_newFunc)())
{
    assert(m_libFuncs.m_nextTickProgramCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_cancelNextTickCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_sentOutDataCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_gwStatusReportCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_gwDisconnectReportCallbackSetFunc != nullptr);
    assert(m_libFuncs.m_msgReportCallbackSetFunc != nullptr);

    (m_libFuncs.m_nextTickProgramCallbackSetFunc)(m_client, &CommonTestClient::nextTickProgramCallback, this);
    (m_libFuncs.m_cancelNextTickCallbackSetFunc)(m_client, &CommonTestClient::cancelNextTickCallback, this);
    (m_libFuncs.m_sentOutDataCallbackSetFunc)(m_client, &CommonTestClient::sendOutputDataCallback, this);
    (m_libFuncs.m_gwStatusReportCallbackSetFunc)(m_client, &CommonTestClient::gwStatusReportCallback, this);
    (m_libFuncs.m_gwDisconnectReportCallbackSetFunc)(m_client, &CommonTestClient::gwDisconnectReportCallback, this);
    (m_libFuncs.m_msgReportCallbackSetFunc)(m_client, &CommonTestClient::msgReportCallback, this);
    // TODO: callbacks
}

void CommonTestClient::programNextTick(unsigned duration)
{
    if (m_programNextTickCallback) {
        ProgramNextTickCallback tmp(m_programNextTickCallback);
        tmp(duration);
    }
}

unsigned CommonTestClient::cancelNextTick()
{
    if (m_cancelNextTickCallback) {
        CancelNextTickCallback tmp(m_cancelNextTickCallback);
        return tmp();
    }

    assert(!"Should not happen");
    return 0;
}

void CommonTestClient::sendOutputData(const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    if (m_sendDataCallback) {
        SendDataCallback tmp(m_sendDataCallback);
        tmp(buf, bufLen, broadcast);
    }
}

void CommonTestClient::reportGwStatus(unsigned short gwId, MqttsnGwStatus status)
{
    if (m_gwStatusReportCallback) {
        GwStatusReportCallback tmp(m_gwStatusReportCallback);
        tmp(gwId, status);
    }
}

void CommonTestClient::reportGwDisconnect()
{
    if (m_gwDisconnectReportCallback) {
        GwDisconnectReportCallback tmp(m_gwDisconnectReportCallback);
        tmp();
    }
}

void CommonTestClient::reportMessage(const MqttsnMessageInfo* msgInfo)
{
    if (m_msgReportCallback) {
        assert(msgInfo != nullptr);
        MessageReportCallback tmp(m_msgReportCallback);
        tmp(*msgInfo);
    }
}

void CommonTestClient::reportConnectComplete(MqttsnAsyncOpStatus status)
{
    if (m_connectCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_connectCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportDisconnectComplete(MqttsnAsyncOpStatus status)
{
    if (m_disconnectCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_disconnectCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportPublishComplete(MqttsnAsyncOpStatus status)
{
    if (m_publishCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_publishCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportSubsribeComplete(MqttsnAsyncOpStatus status, MqttsnQoS qos)
{
    if (m_subscribeCompleteCallback) {
        SubscribeCompleteCallback tmp(m_subscribeCompleteCallback);
        tmp(status, qos);
    }
}

void CommonTestClient::reportUnsubsribeComplete(MqttsnAsyncOpStatus status)
{
    if (m_unsubscribeCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_unsubscribeCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportWillUpdateComplete(MqttsnAsyncOpStatus status)
{
    if (m_willUpdateCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_willUpdateCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportWillTopicUpdateComplete(MqttsnAsyncOpStatus status)
{
    if (m_willTopicUpdateCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_willTopicUpdateCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportWillMsgUpdateComplete(MqttsnAsyncOpStatus status)
{
    if (m_willMsgUpdateCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_willMsgUpdateCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportSleepComplete(MqttsnAsyncOpStatus status)
{
    if (m_sleepCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_sleepCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportReconnectComplete(MqttsnAsyncOpStatus status)
{
    if (m_reconnectCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_reconnectCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::reportCheckMessagesComplete(MqttsnAsyncOpStatus status)
{
    if (m_checkMessagesCompleteCallback) {
        AsyncOpCompleteCallback tmp(m_checkMessagesCompleteCallback);
        tmp(status);
    }
}

void CommonTestClient::nextTickProgramCallback(void* data, unsigned duration)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->programNextTick(duration);
}

unsigned CommonTestClient::cancelNextTickCallback(void* data)
{
    assert(data != nullptr);
    return reinterpret_cast<CommonTestClient*>(data)->cancelNextTick();
}

void CommonTestClient::sendOutputDataCallback(
    void* data,
    const unsigned char* buf,
    unsigned bufLen,
    bool broadcast)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->sendOutputData(buf, bufLen, broadcast);
}

void CommonTestClient::gwStatusReportCallback(
    void* data,
    unsigned char gwId,
    MqttsnGwStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportGwStatus(gwId, status);
}

void CommonTestClient::gwDisconnectReportCallback(void* data)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportGwDisconnect();
}

void CommonTestClient::msgReportCallback(void* data, const MqttsnMessageInfo* msgInfo)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportMessage(msgInfo);
}

void CommonTestClient::publishCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportPublishComplete(status);
}

void CommonTestClient::connectCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportConnectComplete(status);
}

void CommonTestClient::disconnectCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportDisconnectComplete(status);
}

void CommonTestClient::subsribeCompleteCallback(void* data, MqttsnAsyncOpStatus status, MqttsnQoS qos)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportSubsribeComplete(status, qos);
}

void CommonTestClient::unsubsribeCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportUnsubsribeComplete(status);
}

void CommonTestClient::willUpdateCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportWillUpdateComplete(status);
}

void CommonTestClient::willTopicUpdateCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportWillTopicUpdateComplete(status);
}

void CommonTestClient::willMsgUpdateCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportWillMsgUpdateComplete(status);
}

void CommonTestClient::sleepCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportSleepComplete(status);
}

void CommonTestClient::reconnectCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportReconnectComplete(status);
}

void CommonTestClient::checkMessagesCompleteCallback(void* data, MqttsnAsyncOpStatus status)
{
    assert(data != nullptr);
    reinterpret_cast<CommonTestClient*>(data)->reportCheckMessagesComplete(status);
}
