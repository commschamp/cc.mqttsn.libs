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

#include "SessionWrapper.h"

#include <iostream>
#include <cassert>

namespace mqttsn
{

namespace gateway
{

namespace app
{

namespace udp
{

namespace
{

const QString BrokerHost;
const unsigned short BrokerPort = 1883;
const std::string WildcardStr("*");

}  // namespace

SessionWrapper::SessionWrapper(
    const Config& config,
    ClientSocketPtr socket,
    QObject* parent)
  : Base(parent),
    m_config(config),
    m_clientSocket(std::move(socket))
{
    m_session.setNextTickProgramReqCb(
        [this](unsigned ms)
        {
            programNextTick(ms);
        });

    m_session.setCancelTickWaitReqCb(
        [this]() -> unsigned
        {
            return cancelTick();
        });

    m_session.setSendDataClientReqCb(
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            sendDataToClient(buf, bufSize);
        });

    m_session.setSendDataBrokerReqCb(
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            sendDataToBroker(buf, bufSize);
        });

    m_session.setTerminationReqCb(
        [this]()
        {
            termSession();
        });

    m_session.setBrokerReconnectReqCb(
        [this]()
        {
            reconnectBroker();
        });

    m_session.setGatewayId(m_config.gatewayId());
    m_session.setRetryPeriod(m_config.retryPeriod());
    m_session.setRetryCount(m_config.retryCount());
    m_session.setPubOnlyClientId(m_config.pubOnlyClientId());
    m_session.setPubOnlyKeepAlive(m_config.pubOnlyKeepAlive());

    addPredefinedTopicsFor(WildcardStr);

    connect(
        &m_timer, SIGNAL(timeout()),
        this, SLOT(tickTimeout()));

    connect(
        &m_brokerSocket, SIGNAL(connected()),
        this, SLOT(brokerConnected()));
    connect(
        &m_brokerSocket, SIGNAL(disconnected()),
        this, SLOT(brokerDisconnected()));
    connect(
        &m_brokerSocket, SIGNAL(readyRead()),
        this, SLOT(readFromBrokerSocket()));
    connect(
        &m_brokerSocket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(brokerSocketErrorOccurred(QAbstractSocket::SocketError)));

}

SessionWrapper::~SessionWrapper()
{
    std::cout << __FUNCTION__ << std::endl;
}

bool SessionWrapper::start()
{
    if (!m_session.start()) {
        std::cerr << "Failed to start new session" << std::endl;
        return false;
    }

    connectToBroker();
    readFromClientSocket();

    connect(
        m_clientSocket.get(), SIGNAL(readyRead()),
        this, SLOT(readFromClientSocket()));

    connect(
        m_clientSocket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(clientSocketErrorOccurred(QAbstractSocket::SocketError)));

    return true;
}

void SessionWrapper::tickTimeout()
{
    m_session.tick(m_reqTicks);
}

void SessionWrapper::readFromClientSocket()
{
    assert(m_clientSocket);
    DataBuf data;
    QHostAddress senderAddress;
    quint16 senderPort;

    while (m_clientSocket->hasPendingDatagrams()) {
        data.resize(m_clientSocket->pendingDatagramSize());
        auto readBytes = m_clientSocket->readDatagram(
            reinterpret_cast<char*>(&data[0]),
            data.size(),
            &senderAddress,
            &senderPort);
        assert(readBytes == static_cast<decltype(readBytes)>(data.size()));

        if (m_clientSocket->state() != QUdpSocket::ConnectedState) {
            m_clientSocket->connectToHost(senderAddress, senderPort);
            m_clientSocket->waitForConnected();
            assert(m_clientSocket->isOpen());
            assert(m_clientSocket->state() == QUdpSocket::ConnectedState);
        }

        m_session.dataFromClient(&data[0], readBytes);
    }
}

void SessionWrapper::clientSocketErrorOccurred(QAbstractSocket::SocketError err)
{
    static_cast<void>(err);
    assert(m_clientSocket);
    std::cout << "ERROR: UDP Socket: " << m_clientSocket->errorString().toStdString() << std::endl;
}

void SessionWrapper::brokerConnected()
{
    m_session.setBrokerConnected(true);
    m_reconnectRequested = false;
}

void SessionWrapper::brokerDisconnected()
{
    m_session.setBrokerConnected(false);
    if (m_reconnectRequested) {
        connectToBroker();
    }
}

void SessionWrapper::readFromBrokerSocket()
{
    auto data = m_brokerSocket.readAll();
    std::cout << "Received " << data.size() << " bytes from broker" << std::endl;

    auto* buf = reinterpret_cast<const std::uint8_t*>(data.constData());
    std::size_t bufSize = data.size();

    if (!m_brokerData.empty()) {
        m_brokerData.insert(m_brokerData.end(), buf, buf + bufSize);
        buf = &m_brokerData[0];
        bufSize = m_brokerData.size();
    }

    std::size_t consumed = m_session.dataFromBroker(buf, bufSize);
    if (bufSize <= consumed) {
        m_brokerData.clear();
        return;
    }

    if (!m_brokerData.empty()) {
        m_brokerData.erase(m_brokerData.begin(), m_brokerData.begin() + consumed);
        return;
    }

    m_brokerData.assign(buf + consumed, buf + bufSize);
}

void SessionWrapper::brokerSocketErrorOccurred(QAbstractSocket::SocketError err)
{
    static_cast<void>(err);
    std::cout << "ERROR: TCP Socket: " << m_brokerSocket.errorString().toStdString() << std::endl;
}


void SessionWrapper::programNextTick(unsigned ms)
{
    m_reqTicks = ms;
    m_timer.setSingleShot(true);
    m_timer.start(ms);
}

unsigned SessionWrapper::cancelTick()
{
    auto rem = m_timer.remainingTime();
    unsigned result = 0U;
    do {
        if (static_cast<decltype(rem)>(m_reqTicks) <= rem) {
            break;
        }

        result = m_reqTicks - rem;
    } while (false);

    m_timer.stop();
    return result;
}

void SessionWrapper::sendDataToClient(const std::uint8_t* buf, std::size_t bufSize)
{
    assert(m_clientSocket);
    std::cout << "Sending " << bufSize << " bytes to client" << std::endl;
    std::size_t writtenCount = 0;
    while (writtenCount < bufSize) {
        auto remSize = bufSize - writtenCount;
        auto count =
            m_clientSocket->write(
                reinterpret_cast<const char*>(&buf[writtenCount]),
                remSize);
        if (count < 0) {
            std::cerr << "Failed to write to UDP socket" << std::endl;
            return;
        }

        writtenCount += count;
    }
}

void SessionWrapper::sendDataToBroker(const std::uint8_t* buf, std::size_t bufSize)
{
    static_cast<void>(buf);
    static_cast<void>(bufSize);
    std::cout << "Sending " << bufSize << " bytes to broker" << std::endl;

    std::size_t writtenCount = 0;
    while (writtenCount < bufSize) {
        auto remSize = bufSize - writtenCount;
        auto count =
            m_brokerSocket.write(
                reinterpret_cast<const char*>(&buf[writtenCount]),
                remSize);
        if (count < 0) {
            std::cerr << "Failed to write to TCP socket" << std::endl;
            return;
        }

        writtenCount += count;
    }

}

void SessionWrapper::termSession()
{
    std::cout << "INFO: Termination requested:" << std::endl;
    assert(m_clientSocket);
    m_clientSocket->blockSignals(true);
    m_timer.stop();
    deleteLater();
}

void SessionWrapper::reconnectBroker()
{
    m_reconnectRequested = true;
    assert(m_brokerSocket.state() == QTcpSocket::ConnectedState);
    m_brokerSocket.disconnectFromHost();
}

void SessionWrapper::connectToBroker()
{
    QString host = BrokerHost;

    if (host.isEmpty()) {
        host = QHostAddress(QHostAddress::LocalHost).toString();
    }

    m_brokerSocket.connectToHost(host, BrokerPort);
}

void SessionWrapper::addPredefinedTopicsFor(const std::string& clientId)
{
    auto& predefinedTopics = m_config.predefinedTopics();
    for (auto& t : predefinedTopics) {
        if (t.clientId == clientId) {
            m_session.addPredefinedTopic(t.topic, t.topicId);
        }
    }
}

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn


