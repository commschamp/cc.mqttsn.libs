//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Pub.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <cstdint>
#include <iterator>
#include <iomanip>

#include <QtCore/QCoreApplication>

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

Pub::Pub()
  : m_client(cc_mqttsn_client_new())
{
    cc_mqttsn_client_set_next_tick_program_callback(
        m_client, &Pub::nextTickProgramCb, this);

    cc_mqttsn_client_set_cancel_next_tick_wait_callback(
        m_client, &Pub::caneclTickCb, this);

    cc_mqttsn_client_set_send_output_data_callback(
        m_client, &Pub::sendDataCb, this);

    cc_mqttsn_client_set_gw_status_report_callback(
        m_client, &Pub::gwStatusReportCb, this);

    cc_mqttsn_client_set_gw_disconnect_report_callback(
        m_client, &Pub::gwDisconnectReportCb, this);

    cc_mqttsn_client_set_message_report_callback(
        m_client, &Pub::messageReportCb, this);


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

Pub::~Pub()
{
    cc_mqttsn_client_free(m_client);
}

bool Pub::start()
{
    bool result =
        bindLocalPort() &&
        openSocket() &&
        connectToGw() &&
        cc_mqttsn_client_start(m_client) == MqttsnErrorCode_Success;

    if (!result) {
        return false;
    }

    if (m_qos == MqttsnQoS_NoGwPublish) {
        doPublish();
        return true;
    }

    if (m_socket.state() == QUdpSocket::ConnectedState) {
        doConnect();
    }

    return true;
}

void Pub::tick()
{
    m_reqTimeout = 0;
    cc_mqttsn_client_tick(m_client);
}

void Pub::readFromSocket()
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

        if (m_debug) {
            std::cout << "[DEBUG]: --> " << std::hex;
            for (auto byte : data) {
                std::cout << std::setw(2) << std::setfill('0') << static_cast<unsigned>(byte) << ' ';
            }
            std::cout << std::dec << std::endl;
        }

        cc_mqttsn_client_process_data(m_client, &data[0], static_cast<unsigned>(data.size()));
    }
}

void Pub::socketErrorOccurred(QAbstractSocket::SocketError err)
{
    static_cast<void>(err);
    std::cerr << "ERROR: UDP Socket: " << m_socket.errorString().toStdString() << std::endl;
}


void Pub::nextTickProgram(unsigned ms)
{
//    std::cout << "Tick req: " << ms << std::endl;
    m_reqTimeout = ms;
    m_timer.setSingleShot(true);
    m_timer.start(ms);
}

void Pub::nextTickProgramCb(void* obj, unsigned ms)
{
    assert(obj != nullptr);
    reinterpret_cast<Pub*>(obj)->nextTickProgram(ms);
}

unsigned Pub::cancelTick()
{
    auto rem = m_timer.remainingTime();
    m_timer.stop();

    if (m_reqTimeout < static_cast<unsigned>(rem)) {
        rem = static_cast<decltype(rem)>(m_reqTimeout);
    }

    return m_reqTimeout - static_cast<unsigned>(rem);
}

unsigned Pub::caneclTickCb(void* obj)
{
    assert(obj != nullptr);
    return reinterpret_cast<Pub*>(obj)->cancelTick();
}

void Pub::sendData(const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    if (m_debug) {
        std::cout << "[DEBUG]: <-- " << std::hex;
        for (auto idx = 0U; idx < bufLen; ++idx) {
            std::cout << std::setw(2) << std::setfill('0') << static_cast<unsigned>(buf[idx]) << ' ';
        }
        std::cout << std::dec << std::endl;
    }

    if (broadcast) {
        broadcastData(buf, bufLen);
        return;
    }

    if (m_socket.state() != QUdpSocket::ConnectedState) {
        return;
    }

    sendDataConnected(buf, bufLen);
}

void Pub::sendDataCb(void* obj, const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    assert(obj != nullptr);
    reinterpret_cast<Pub*>(obj)->sendData(buf, bufLen, broadcast);
}

void Pub::gwStatusReport(unsigned short gwId, MqttsnGwStatus status)
{
    if (status != MqttsnGwStatus_Available) {
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

void Pub::gwStatusReportCb(void* obj, unsigned char gwId, MqttsnGwStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Pub*>(obj)->gwStatusReport(gwId, status);
}

void Pub::gwDisconnectReport()
{
    if (m_disconnecting) {
        quitApp();
        return;
    }

    std::cerr << "WARNING: Disconnected from GW, reconnecting..." << std::endl;
    doConnect();
}

void Pub::gwDisconnectReportCb(void* obj)
{
    assert(obj != nullptr);
    reinterpret_cast<Pub*>(obj)->gwDisconnectReport();
}

void Pub::messageReportCb(void* obj, const MqttsnMessageInfo* msgInfo)
{
    static_cast<void>(obj);
    static_cast<void>(msgInfo);
}

void Pub::doConnect(bool reconnecting)
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
            &Pub::connectCompleteCb,
            this);
    if (result != MqttsnErrorCode_Success) {
        std::cerr << "ERROR: Failed to connect to the gateway" << std::endl;
    }
}

void Pub::doPublish()
{
    if (!m_topic.empty()) {
        auto result =
            cc_mqttsn_client_publish(
                m_client,
                m_topic.c_str(),
                m_msg.empty() ? nullptr : &m_msg[0],
                static_cast<unsigned>(m_msg.size()),
                m_qos,
                m_retain,
                &Pub::publishCompleteCb,
                this);
        if (result != MqttsnErrorCode_Success) {
            std::cerr << "ERROR: Failed to initiate publish of topic " << m_topic << std::endl;
            quitApp();
        }
        return;
    }

    if (m_topicId != 0) {
        auto result =
            cc_mqttsn_client_publish_id(
                m_client,
                m_topicId,
                m_msg.empty() ? nullptr : &m_msg[0],
                static_cast<unsigned>(m_msg.size()),
                m_qos,
                m_retain,
                &Pub::publishCompleteCb,
                this);

        if (result != MqttsnErrorCode_Success) {
            std::cerr << "ERROR: Failed to initiate subscribe for topic ID " << m_topicId << std::endl;
            quitApp();
        }
        return;
    }

    if (m_qos == MqttsnQoS_NoGwPublish) {
        quitApp();
        return;
    }

    m_disconnecting = true;
    cc_mqttsn_client_disconnect(m_client, &Pub::disconnectCompleteCb, this);
}

void Pub::connectComplete(MqttsnAsyncOpStatus status)
{
    if (m_qos == MqttsnQoS_NoGwPublish) {
        return;
    }

    if (status == MqttsnAsyncOpStatus_Successful) {
        doPublish();
        return;
    }

    if (status == MqttsnAsyncOpStatus_Congestion) {
        std::cerr << "WARNING: Congestion reported, reconnecting..." << std::endl;
        doConnect();
        return;
    }

    std::cerr << "ERROR: Failed to connect..." << std::endl;
    quitApp();
}

void Pub::connectCompleteCb(void* obj, MqttsnAsyncOpStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Pub*>(obj)->connectComplete(status);
}

void Pub::disconnectComplete(MqttsnAsyncOpStatus status)
{
    static_cast<void>(status);
    quitApp();
}

void Pub::disconnectCompleteCb(void* obj, MqttsnAsyncOpStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Pub*>(obj)->disconnectComplete(status);
}

void Pub::publishComplete(MqttsnAsyncOpStatus status)
{
    if (status == MqttsnAsyncOpStatus_Congestion) {
        std::cerr << "WARNING: Failed to publish due to congestion, retrying..." << std::endl;
        doPublish();
        return;
    }

    if (status != MqttsnAsyncOpStatus_Successful) {
        std::cerr << "WARNING: Failed to publish topic ";
        if (!m_topic.empty()) {
            std::cerr << m_topic;
        }
        else if (m_topicId != 0) {
            std::cerr << "ID " << m_topicId;
        }
        std::cerr << std::endl;
    }

    if (m_qos == MqttsnQoS_NoGwPublish) {
        m_socket.flush();
        quitApp();
        return;
    }

    m_disconnecting = true;
    cc_mqttsn_client_disconnect(m_client, &Pub::disconnectCompleteCb, this);
}

void Pub::publishCompleteCb(void* obj, MqttsnAsyncOpStatus status)
{
    assert(obj != nullptr);
    reinterpret_cast<Pub*>(obj)->publishComplete(status);
}

bool Pub::bindLocalPort()
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

bool Pub::openSocket()
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

bool Pub::connectToGw()
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

void Pub::broadcastData(const unsigned char* buf, unsigned bufLen)
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

void Pub::sendDataConnected(const unsigned char* buf, unsigned bufLen)
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

void Pub::quitApp()
{
    QTimer::singleShot(10, qApp, SLOT(quit()));
}

}  // namespace udp

}  // namespace sub

}  // namespace app

}  // namespace cc_mqttsn_client


