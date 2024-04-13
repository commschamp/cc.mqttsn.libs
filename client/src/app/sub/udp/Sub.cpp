//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Sub.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <cstdint>
#include <iterator>
#include <iomanip>

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

namespace cc_mqttsn_client
{

namespace app
{

namespace sub
{

namespace udp
{

namespace
{

typedef std::vector<std::uint8_t> DataBuf;

}  // namespace

Sub::Sub()
  : m_client(cc_mqttsn_client_new())
{
    cc_mqttsn_client_set_next_tick_program_callback(
        m_client, &Sub::nextTickProgramCb, this);

    cc_mqttsn_client_set_cancel_next_tick_wait_callback(
        m_client, &Sub::caneclTickCb, this);

    cc_mqttsn_client_set_send_output_data_callback(
        m_client, &Sub::sendDataCb, this);

    cc_mqttsn_client_set_gw_status_report_callback(
        m_client, &Sub::gwStatusReportCb, this);

    cc_mqttsn_client_set_gw_disconnect_report_callback(
        m_client, &Sub::gwDisconnectReportCb, this);

    cc_mqttsn_client_set_message_report_callback(
        m_client, &Sub::messageReportCb, this);


    connect(
        &m_timer, SIGNAL(timeout()),
        this, SLOT(tick()));

    connect(
        &m_socket, SIGNAL(readyRead()),
        this, SLOT(readFromSocket()));

    connect(
        &m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(socketErrorOccurred(QAbstractSocket::SocketError)));

}

Sub::~Sub()
{
    cc_mqttsn_client_free(m_client);
}

bool Sub::start()
{
    bool result =
        bindLocalPort() &&
        openSocket() &&
        connectToGw() &&
        cc_mqttsn_client_start(m_client) == CC_MqttsnErrorCode_Success;


    if (result && (m_socket.state() == QUdpSocket::ConnectedState)) {
        doConnect();
    }
    return result;
}

void Sub::tick()
{
    m_reqTimeout = 0;
    cc_mqttsn_client_tick(m_client);
}

void Sub::readFromSocket()
{
    DataBuf data;

    while (m_socket.hasPendingDatagrams()) {
        data.resize(m_socket.pendingDatagramSize());
        auto readBytes = m_socket.readDatagram(
            reinterpret_cast<char*>(&data[0]),
            data.size(),
            &m_lastSenderAddress,
            &m_lastSenderPort);
        assert(readBytes == static_cast<decltype(readBytes)>(data.size()));
        static_cast<void>(readBytes);

//        std::cout << "--> " << std::hex;
//        std::copy_n(&data[0], data.size(), std::ostream_iterator<unsigned>(std::cout, " "));
//        std::cout << std::dec << std::endl;

        cc_mqttsn_client_process_data(m_client, &data[0], static_cast<unsigned>(data.size()));
    }
}

void Sub::socketErrorOccurred(QAbstractSocket::SocketError err)
{
    static_cast<void>(err);
    std::cerr << "ERROR: UDP Socket: " << m_socket.errorString().toStdString() << std::endl;
}


void Sub::nextTickProgram(unsigned ms)
{
//    std::cout << "Tick req: " << ms << std::endl;
    m_reqTimeout = ms;
    m_timer.setSingleShot(true);
    m_timer.start(ms);
}

void Sub::nextTickProgramCb(void* obj, unsigned ms)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->nextTickProgram(ms);
}

unsigned Sub::cancelTick()
{
    auto rem = m_timer.remainingTime();
    m_timer.stop();

    if (m_reqTimeout < static_cast<unsigned>(rem)) {
        rem = static_cast<decltype(rem)>(m_reqTimeout);
    }

    return m_reqTimeout - static_cast<unsigned>(rem);
}

unsigned Sub::caneclTickCb(void* obj)
{
    assert(obj != nullptr);
    return reinterpret_cast<Sub*>(obj)->cancelTick();
}

void Sub::sendData(const unsigned char* buf, unsigned bufLen, bool broadcast)
{
//    std::cout << "<-- " << std::hex;
//    std::copy_n(buf, bufLen, std::ostream_iterator<unsigned>(std::cout, " "));
//    std::cout << std::dec << std::endl;

    if (broadcast) {
        broadcastData(buf, bufLen);
        return;
    }

    if (m_socket.state() != QUdpSocket::ConnectedState) {
        return;
    }

    sendDataConnected(buf, bufLen);
}

void Sub::sendDataCb(void* obj, const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->sendData(buf, bufLen, broadcast);
}

void Sub::gwStatusReport(unsigned short gwId, CC_MqttsnGwStatus status)
{
    if (status != CC_MqttsnGwStatus_Available) {
        return;
    }

    if ((0 <= m_gwId) && (gwId != m_gwId)) {
        return;
    }

    if (m_socket.state() != QUdpSocket::ConnectedState) {
        m_socket.connectToHost(m_lastSenderAddress, m_lastSenderPort);
        if (!m_socket.waitForConnected(2000)) {
            std::cerr << "ERROR: Failed to connect UDP socket" << std::endl;
            return;
        }

        assert(m_socket.isOpen());
        assert(m_socket.state() == QUdpSocket::ConnectedState);
    }

    doConnect();
}

void Sub::gwStatusReportCb(void* obj, unsigned char gwId, CC_MqttsnGwStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->gwStatusReport(gwId, status);
}

void Sub::gwDisconnectReport()
{
    std::cerr << "WARNING: Disconnected from GW, reconnecting..." << std::endl;
    doConnect(true);
}

void Sub::gwDisconnectReportCb(void* obj)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->gwDisconnectReport();
}

void Sub::messageReport(const CC_MqttsnMessageInfo* msgInfo)
{
    assert(msgInfo != nullptr);
    if (msgInfo->retain && m_noRetain) {
        return;
    }

    if (m_verbose) {
        if (msgInfo->topic != nullptr) {
            std::cout << msgInfo->topic;
        }
        else {
            std::cout << msgInfo->topicId;
        }
        std::cout << " (qos=" << static_cast<unsigned>(msgInfo->qos);
        if (msgInfo->retain) {
            std::cout << " retained";
        }
        std::cout << ") : ";
    }

    if (!m_hexOutput) {
        std::copy_n(msgInfo->msg, msgInfo->msgLen, std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;
        return;
    }

    std::cout << std::hex;
    for (auto idx = 0U; idx < msgInfo->msgLen; ++idx) {
        if (0U < idx) {
            std::cout << ' ';
        }
        std::cout << std::setw(2) << std::setfill('0') << static_cast<unsigned>(msgInfo->msg[idx]);
    }
    std::cout << std::dec << std::endl;
}

void Sub::messageReportCb(void* obj, const CC_MqttsnMessageInfo* msgInfo)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->messageReport(msgInfo);
}

void Sub::doConnect(bool reconnecting)
{
    bool cleanSession = m_cleanSession;
    if (reconnecting) {
        cleanSession = false;
    }

    auto result =
        cc_mqttsn_client_connect(
            m_client,
            m_clientId.c_str(),
            m_keepAlive,
            cleanSession,
            nullptr,
            &Sub::connectCompleteCb,
            this);
    if (result != CC_MqttsnErrorCode_Success) {
        std::cerr << "ERROR: Failed to connect to the gateway" << std::endl;
    }
}

void Sub::doSubscribe()
{
    if (!m_topics.empty()) {
        auto result =
            cc_mqttsn_client_subscribe(
                m_client,
                m_topics.front().c_str(),
                m_qos,
                &Sub::subscribeCompleteCb,
                this);
        if (result != CC_MqttsnErrorCode_Success) {
            std::cerr << "ERROR: Failed to initiate subscribe for topic " << m_topics.front() << std::endl;
            m_topics.pop_front();
            doSubscribe();
        }
        return;
    }

    if (!m_topicIds.empty()) {
        auto result =
            cc_mqttsn_client_subscribe_id(
                m_client,
                m_topicIds.front(),
                m_qos,
                &Sub::subscribeCompleteCb,
                this);

        if (result != CC_MqttsnErrorCode_Success) {
            std::cerr << "ERROR: Failed to initiate subscribe for topic ID " << m_topicIds.front() << std::endl;
            m_topicIds.pop_front();
            doSubscribe();
        }
        return;
    }
}

void Sub::connectComplete(CC_MqttsnAsyncOpStatus status)
{
    if (status == CC_MqttsnAsyncOpStatus_Successful) {
        doSubscribe();
        return;
    }

    if (status == CC_MqttsnAsyncOpStatus_Congestion) {
        std::cerr << "WARNING: Congestion reported, reconnecting..." << std::endl;
        doConnect();
        return;
    }

    std::cerr << "ERROR: Failed to connect..." << std::endl;
    QTimer::singleShot(10, qApp, SLOT(quit()));
}

void Sub::connectCompleteCb(void* obj, CC_MqttsnAsyncOpStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->connectComplete(status);
}


void Sub::subscribeComplete(CC_MqttsnAsyncOpStatus status)
{
    if (status == CC_MqttsnAsyncOpStatus_Congestion) {
        std::cerr << "WARNING: Failed to subscribe due to congestion, retrying..." << std::endl;
        doSubscribe();
        return;
    }

    if (status != CC_MqttsnAsyncOpStatus_Successful) {
        std::cerr << "WARNING: Failed to subscribe to topic ";
        if (!m_topics.empty()) {
            std::cerr << m_topics.front();
        }
        else if (!m_topicIds.empty()) {
            std::cerr << "ID " << m_topicIds.front();
        }
        std::cerr << std::endl;
    }

    if (!m_topics.empty()) {
        m_topics.pop_front();
    }
    else if (!m_topicIds.empty()) {
        m_topicIds.pop_front();
    }

    doSubscribe();
}

void Sub::subscribeCompleteCb(void* obj, CC_MqttsnAsyncOpStatus status, CC_MqttsnQoS qos)
{
    static_cast<void>(qos);
    assert(obj != nullptr);
    reinterpret_cast<Sub*>(obj)->subscribeComplete(status);
}

bool Sub::bindLocalPort()
{
    if (m_localPort == 0) {
        return true;
    }

    bool result =
        m_socket.bind(QHostAddress::AnyIPv4, m_localPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!result) {
        std::cerr << "ERROR: Failed to bind UDP socket to local port " << m_localPort << std::endl;
    }
    return result;
}

bool Sub::openSocket()
{
    if (m_socket.isOpen()) {
        return true;
    }

    bool result = m_socket.open(QUdpSocket::ReadWrite);
    if (!result) {
        std::cerr << "ERROR: Failed to open UDP socket" << std::endl;
    }
    return result;
}

bool Sub::connectToGw()
{
    if ((m_gwAddr.isEmpty()) || (m_gwPort == 0)) {
        return true;
    }

    m_socket.connectToHost(m_gwAddr, m_gwPort);
    if (!m_socket.waitForConnected(2000)) {
        std::cerr << "ERROR: Failed to connect UDP socket" << std::endl;
        return false;
    }

    assert(m_socket.isOpen());
    assert(m_socket.state() == QUdpSocket::ConnectedState);

    cc_mqttsn_client_set_searchgw_enabled(m_client, false);
    return true;
}

void Sub::broadcastData(const unsigned char* buf, unsigned bufLen)
{
    if (m_gwPort == 0) {
        return;
    }

    std::size_t writtenCount = 0;
    while (writtenCount < bufLen) {
        auto remSize = bufLen - writtenCount;
        auto count =
            m_socket.writeDatagram(
                reinterpret_cast<const char*>(&buf[writtenCount]),
                remSize,
                QHostAddress::Broadcast,
                m_gwPort);

        if (count < 0) {
            std::cerr << "ERROR: Failed to broadcast data" << std::endl;
            return;
        }

        writtenCount += count;
    }

}

void Sub::sendDataConnected(const unsigned char* buf, unsigned bufLen)
{
    std::size_t writtenCount = 0;
    while (writtenCount < bufLen) {
        auto remSize = bufLen - writtenCount;
        auto count =
            m_socket.write(
                reinterpret_cast<const char*>(&buf[writtenCount]),
                remSize);
        if (count < 0) {
            std::cerr << "ERROR: Failed to write to UDP socket" << std::endl;
            return;
        }

        writtenCount += count;
    }
}


}  // namespace udp

}  // namespace sub

}  // namespace app

}  // namespace cc_mqttsn_client


