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

#include "Filter.h"

#include <cassert>
#include <iostream>
#include <iterator>

#include "comms/util/ScopeGuard.h"

namespace cc = comms_champion;

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

Filter::Filter()
{
    connect(
        &m_tickTimer, SIGNAL(timeout()),
        this, SLOT(tick()));
}
Filter::~Filter() = default;

bool Filter::startImpl()
{
    m_client.reset(mqttsn_client_new());
    mqttsn_client_set_retry_period(m_client.get(), m_retryPeriod);
    mqttsn_client_set_retry_count(m_client.get(), m_retryCount);
    mqttsn_client_set_message_report_callback(m_client.get(), &Filter::messageArrivedCb, this);
    mqttsn_client_set_send_output_data_callback(m_client.get(), &Filter::sendMessageCb, this);
    mqttsn_client_set_next_tick_program_callback(m_client.get(), &Filter::programNextTickCb, this);
    mqttsn_client_set_cancel_next_tick_wait_callback(m_client.get(), &Filter::cancelTickCb, this);
    mqttsn_client_set_gw_status_report_callback(m_client.get(), &Filter::gwStatusReportCb, this);
    mqttsn_client_set_connection_status_report_callback(m_client.get(), &Filter::connectionStatusReportCb, this);
    QTimer::singleShot(0, this, SLOT(startClient()));
    return true;
}

void Filter::stopImpl()
{
    mqttsn_client_stop(m_client.get());
    m_client.reset();
}

QList<Filter::DataInfoPtr> Filter::recvDataImpl(DataInfoPtr dataPtr)
{
    if (!dataPtr) {
        return std::move(m_readData);
    }

    m_readSendInProgress = true;
    auto guard = comms::util::makeScopeGuard(
        [this]()
        {
            m_readSendInProgress = false;
            sendAccumulatedMessages();
        });

    do {
        if (!m_input) {
            m_input = std::move(dataPtr);
            break;
        }

        m_input->m_data.insert(m_input->m_data.end(), dataPtr->m_data.begin(), dataPtr->m_data.end());
        m_input->m_timestamp = dataPtr->m_timestamp;
        m_input->m_extraProperties = std::move(dataPtr->m_extraProperties);
    } while (false);

    assert(m_input);
    auto iter = m_input->m_data.begin();
    while (iter != m_input->m_data.end()) {
        auto remLen = static_cast<unsigned>(std::distance(iter, m_input->m_data.end()));
        auto consumed = mqttsn_client_process_data(m_client.get(), &(*iter), remLen);
        if (consumed == 0U) {
            break;
        }

        iter += consumed;
    }

    if (iter == m_input->m_data.end()) {
        m_input.reset();
    }
    else {
        m_input->m_data.erase(m_input->m_data.begin(), iter);
    }

    return std::move(m_readData);
}

QList<Filter::DataInfoPtr> Filter::sendDataImpl(DataInfoPtr dataPtr)
{
    assert(!m_readSendInProgress);
    m_readSendInProgress = true;
    auto guard = comms::util::makeScopeGuard(
        [this]()
        {
            m_readSendInProgress = false;
        });

    bool firstPub = m_pendingPubs.isEmpty();
    m_pendingPubs.append(std::move(dataPtr));

    if (firstPub && m_connected && (m_subs.size() <= m_completedSubsCount)) {
        doPublish();
    }

    return std::move(m_sendData);
};

void Filter::tick()
{
    mqttsn_client_tick(m_client.get(), m_tickDuration);
}

void Filter::startClient()
{
    if (m_clientId.empty()) {
        std::cerr << "ERROR: MQTT-SN: Cannot start with empty client ID" << std::endl;
        return;
    }

    if (mqttsn_client_start(m_client.get()) != MqttsnErrorCode_Success) {
        reportError("MQTT-SN client failed to start");
    }
}

void Filter::doPublish()
{
    if (m_pendingPubs.isEmpty()) {
        return;
    }

    auto& pubInfo = m_pendingPubs.front();
    assert(pubInfo);

    MqttsnQoS qos = m_pub.m_qos;
    auto qosVar = pubInfo->m_extraProperties.value(m_qosPropertyName);
    if (qosVar.isValid() && qosVar.canConvert<int>()) {
        qos = static_cast<MqttsnQoS>(qosVar.value<int>());
    }

    bool retain = m_pub.m_retain;
    auto retainVar = pubInfo->m_extraProperties.value(m_retainPropertyName);
    if (retainVar.isValid() && retainVar.canConvert<bool>()) {
        retain = retainVar.value<bool>();
    }

    do {
        auto topicId = m_pub.m_topicId;
        auto topicIdVar = pubInfo->m_extraProperties.value(m_topicIdPropertyName);
        if (topicIdVar.isValid() && topicIdVar.canConvert<MqttsnTopicId>()) {
            topicId = topicIdVar.value<MqttsnTopicId>();
        }

        if (topicId == 0U) {
            break;
        }

        auto result =
            mqttsn_client_publish_id(
                m_client.get(),
                topicId,
                &pubInfo->m_data[0],
                pubInfo->m_data.size(),
                qos,
                retain,
                &Filter::publishCompleteCb,
                this);

        if (result != MqttsnErrorCode_Success) {
            std::cerr << "ERROR: MQTT-SN: publish operation cannot be performed" << std::endl;
        }
        return;
    } while (false);

    QString topic = m_pub.m_topic;
    auto topicVar = pubInfo->m_extraProperties.value(m_topicPropertyName);
    if (topicVar.isValid() && topicVar.canConvert<QString>()) {
        auto topicValue = topicVar.toString();
        if (!topicValue.isEmpty()) {
            topic = topicValue;
        }
    }

    if (topic.isEmpty()) {
        std::cerr << "ERROR: MQTT-SN: Cannot publish with empty topic" << std::endl;
        m_pendingPubs.pop_front();
        doPublish();
        return;
    }

    m_topicHolder = topic.toStdString();
    auto result =
        mqttsn_client_publish(
            m_client.get(),
            m_topicHolder.c_str(),
            &pubInfo->m_data[0],
            pubInfo->m_data.size(),
            qos,
            retain,
            &Filter::publishCompleteCb,
            this);

    if (result != MqttsnErrorCode_Success) {
        std::cerr << "ERROR: MQTT-SN: publish operation cannot be performed" << std::endl;
        m_pendingPubs.pop_back();
        doPublish();
        return;
    }
}

void Filter::publishComplete(MqttsnAsyncOpStatus status)
{
    assert(!m_pendingPubs.isEmpty());

    if (status == MqttsnAsyncOpStatus_Congestion) {
        doPublish();
        return;
    }

    if (status != MqttsnAsyncOpStatus_Successful) {
        std::cerr << "ERROR: MQTT-SN: Publish has failed with status: " << (unsigned)status << std::endl;
    }

    m_pendingPubs.pop_front();
    doPublish();
}

void Filter::doConnect()
{
    if ((m_connected) || (m_gateways.isEmpty()) || (m_clientId.empty())) {
        return;
    }

    assert(!m_clientId.empty());
    auto result =
        mqttsn_client_connect(
            m_client.get(), m_clientId.c_str(), m_keepAlivePeriod, true, nullptr);

    if (result != MqttsnErrorCode_Success) {
        std::cerr << "ERROR: MQTT-SN: Failed to initiate connection to the GW" << std::endl;
    }
}

void Filter::sendAccumulatedMessages()
{
    decltype(m_sendData) dataToSend;
    dataToSend.swap(m_sendData);
    assert(m_sendData.isEmpty());

    for (auto& s : dataToSend) {
        reportDataToSend(std::move(s));
    }
}

void Filter::doSubscribe()
{
    if (m_subs.size() <= m_completedSubsCount) {
        doPublish();
        return;
    }

    auto& sub = m_subs.front();
    if (!sub.m_topic.isEmpty()) {
        m_topicHolder = sub.m_topic.toStdString();
        auto result =
            mqttsn_client_subscribe(
                m_client.get(),
                m_topicHolder.c_str(),
                sub.m_qos,
                &Filter::subscribeCompleteCb,
                this);

        assert(result == MqttsnErrorCode_Success);
        static_cast<void>(result);
        return;
    }

    if (sub.m_topicId == 0) {
        std::cerr << "ERROR: MQTT-SN: cannont subscribe to topic 0" << std::endl;
        ++m_completedSubsCount;
        doSubscribe();
        return;
    }

    auto result =
        mqttsn_client_subscribe_id(
            m_client.get(),
            sub.m_topicId,
            sub.m_qos,
            &Filter::subscribeCompleteCb,
            this);

    assert(result == MqttsnErrorCode_Success);
    static_cast<void>(result);
}

void Filter::subscribeComplete(MqttsnAsyncOpStatus status, MqttsnQoS qos)
{
    static_cast<void>(qos);
    assert(!m_subs.isEmpty());

    if (status == MqttsnAsyncOpStatus_Congestion) {
        doSubscribe();
        return;
    }

    if (status != MqttsnAsyncOpStatus_Successful) {
        std::cerr << "ERROR: MQTT-SN: Subscribe has failed with status: " << (unsigned)status << std::endl;
    }

    ++m_completedSubsCount;
    doSubscribe();
}

void Filter::reportReceivedMessage(const MqttsnMessageInfo& msgInfo)
{
    auto data = cc::makeDataInfo();
    data->m_timestamp = cc::DataInfo::TimestampClock::now();
    if ((msgInfo.msg != nullptr) && (0 < msgInfo.msgLen)) {
        data->m_data.assign(msgInfo.msg, msgInfo.msg + msgInfo.msgLen);
    }

    if ((msgInfo.topic != nullptr) && (!m_topicPropertyName.isEmpty())) {
        data->m_extraProperties.insert(m_topicPropertyName, msgInfo.topic);
    }
    else if ((msgInfo.topicId != 0) && (!m_topicIdPropertyName.isEmpty())) {
        data->m_extraProperties.insert(m_topicIdPropertyName, msgInfo.topicId);
    }

    if (!m_qosPropertyName.isEmpty()) {
        data->m_extraProperties.insert(m_qosPropertyName, (int)msgInfo.qos);
    }

    if (!m_retainPropertyName.isEmpty()) {
        data->m_extraProperties.insert(m_retainPropertyName, msgInfo.retain);
    }

    m_readData.append(std::move(data));
}

void Filter::sendMessage(const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    assert(buf != nullptr);
    assert(0 < bufLen);

    auto data = cc::makeDataInfo();
    data->m_timestamp = cc::DataInfo::TimestampClock::now();
    data->m_data.insert(data->m_data.end(), buf, buf + bufLen);
    if (broadcast) {
        data->m_extraProperties.insert(m_broadcastPropertyName, true);
    }

    m_sendData.append(std::move(data));
    if (m_readSendInProgress) {
        return;
    }

    sendAccumulatedMessages();
}

void Filter::programNextTick(unsigned duration)
{
    m_tickDuration = duration;
    m_tickTimer.stop();
    m_tickTimer.setSingleShot(true);
    m_tickTimer.setInterval(duration);
    m_tickTimer.start();
}

unsigned Filter::cancelTick()
{
    if (!m_tickTimer.isActive()) {
        return 0U;
    }

    auto remainingTime = m_tickTimer.remainingTime();
    assert(0 <= remainingTime);
    assert(static_cast<unsigned>(remainingTime) <= m_tickDuration);
    auto ticks = m_tickDuration - remainingTime;
    m_tickTimer.stop();
    return ticks;
}

void Filter::gwStatusReport(unsigned short gwId, MqttsnGwStatus status)
{
    auto iter = std::find(m_gateways.begin(), m_gateways.end(), gwId);

    if ((status == MqttsnGwStatus_TimedOut) && (iter != m_gateways.end())) {
        m_gateways.erase(iter);
        return;
    }

    if ((status == MqttsnGwStatus_Available) && (iter == m_gateways.end())) {
        m_gateways.append(gwId);
        if ((!m_connected) && (m_gateways.front() == gwId)) {
            doConnect();
        }
        return;
    }
}

void Filter::connectionStatusReport(MqttsnConnectionStatus status)
{
    if (status != MqttsnConnectionStatus_Connected) {
        std::cerr << "ERROR: MQTT-SN: Not connected to gateway: status=" << (int)status << std::endl;
        m_connected = false;
        doConnect();
        return;
    }

    m_connected = true;
    m_completedSubsCount = 0;

    doSubscribe();
}

void Filter::messageArrivedCb(void* data, const MqttsnMessageInfo* msgInfo)
{
    if ((data == nullptr) || (msgInfo == nullptr)) {
        return;
    }
    reinterpret_cast<Filter*>(data)->reportReceivedMessage(*msgInfo);
}

void Filter::sendMessageCb(
    void* data,
    const unsigned char* buf,
    unsigned bufLen,
    bool broadcast)
{
    if ((data == nullptr) || (buf == nullptr) || (bufLen == 0)) {
        return;
    }

    reinterpret_cast<Filter*>(data)->sendMessage(buf, bufLen, broadcast);
}

void Filter::programNextTickCb(void* data, unsigned duration)
{
    if (data == nullptr) {
        return;
    }

    reinterpret_cast<Filter*>(data)->programNextTick(duration);
}

unsigned Filter::cancelTickCb(void* data)
{
    if (data == nullptr) {
        return 0U;
    }

    return reinterpret_cast<Filter*>(data)->cancelTick();
}

void Filter::gwStatusReportCb(
    void* data,
    unsigned short gwId,
    MqttsnGwStatus status)
{
    if (data == nullptr) {
        return;
    }

    reinterpret_cast<Filter*>(data)->gwStatusReport(gwId, status);
}

void Filter::connectionStatusReportCb(void* data, MqttsnConnectionStatus status)
{
    if (data == nullptr) {
        return;
    }

    reinterpret_cast<Filter*>(data)->connectionStatusReport(status);
}

void Filter::publishCompleteCb(void* data, MqttsnAsyncOpStatus status)
{
    if (data == nullptr) {
        return;
    }

    reinterpret_cast<Filter*>(data)->publishComplete(status);
}

void Filter::subscribeCompleteCb(void* data, MqttsnAsyncOpStatus status, MqttsnQoS qos)
{
    if (data == nullptr) {
        return;
    }

    reinterpret_cast<Filter*>(data)->subscribeComplete(status, qos);
}

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn




