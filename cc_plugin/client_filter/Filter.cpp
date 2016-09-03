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

namespace cc = comms_champion;

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

Filter::Filter()
{
    assert(m_client);
    std::cout << __FUNCTION__ << ": " << this << std::endl;

    connect(
        &m_tickTimer, SIGNAL(timeout()),
        this, SLOT(tick()));
}
Filter::~Filter()
{
    std::cout << __FUNCTION__ << ": " << this << std::endl;
}

bool Filter::startImpl()
{
    std::cout << "FILTER started: " << this << std::endl;
    m_client.reset(mqttsn_client_new());
    mqttsn_client_set_gw_advertise_period(m_client.get(), m_advertisePeriod);
    mqttsn_client_set_retry_period(m_client.get(), m_retryPeriod);
    mqttsn_client_set_retry_count(m_client.get(), m_retryCount);
    mqttsn_client_set_message_report_callback(m_client.get(), &Filter::messageArrivedCb, this);
    mqttsn_client_set_send_output_data_callback(m_client.get(), &Filter::sendMessageCb, this);
    mqttsn_client_set_next_tick_program_callback(m_client.get(), &Filter::programNextTickCb, this);

    return true;
}

void Filter::stopImpl()
{
    std::cout << "FILTER stopped: " << this << std::endl;
    m_client.reset();
}

Filter::DataInfoPtr Filter::recvDataImpl(DataInfoPtr dataPtr)
{
    std::cout << "Data received" << std::endl;

    static_cast<void>(dataPtr);
    assert(!"NYI");
    // TODO: report data
    return dataPtr;
}

Filter::DataInfoPtr Filter::sendDataImpl(DataInfoPtr dataPtr)
{
    std::cout << "Data sent" << std::endl;
    assert(!"NYI");
    // TODO: publish
    return dataPtr;
};

void Filter::tick()
{
    mqttsn_client_tick(m_client.get(), m_tickDuration);
}

//void mqttsn_client_set_cancel_next_tick_wait_callback(
//    MqttsnClientHandle client,
//    MqttsnCancelNextTickWaitFn fn,
//    void* data);
//void mqttsn_client_set_gw_status_report_callback(
//    MqttsnClientHandle client,
//    MqttsnGwStatusReportFn fn,
//    void* data);
//void mqttsn_client_set_connection_status_report_callback(
//    MqttsnClientHandle client,
//    MqttsnConnectionStatusReportFn fn,
//    void* data);
//

void Filter::reportReceivedMessage(const MqttsnMessageInfo& msgInfo)
{
    // TODO: process multiple messages from the same input;
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
    // TODO: send
}

void Filter::programNextTick(unsigned duration)
{
    m_tickDuration = duration;
    m_tickTimer.stop();
    m_tickTimer.setSingleShot(true);
    m_tickTimer.setInterval(duration);
    m_tickTimer.start();
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

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn




