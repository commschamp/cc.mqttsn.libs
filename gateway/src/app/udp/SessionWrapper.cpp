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

SessionWrapper::SessionWrapper(SocketPtr socket, QObject* parent)
  : Base(parent),
    m_socket(std::move(socket))
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

    m_session.setGatewayId(1U);

    connect(
        &m_timer, SIGNAL(timeout()),
        this, SLOT(tickTimeout()));
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

    // TODO: connect to broker

    doRead();

    connect(
        m_socket.get(), SIGNAL(readyRead()),
        this, SLOT(doRead()));

    connect(
        m_socket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(socketErrorOccurred(QAbstractSocket::SocketError)));

    return true;
}

void SessionWrapper::tickTimeout()
{
    m_session.tick(m_reqTicks);
}

void SessionWrapper::doRead()
{
    assert(m_socket);
    DataBuf data;
    QHostAddress senderAddress;
    quint16 senderPort;

    while (m_socket->hasPendingDatagrams()) {
        data.resize(m_socket->pendingDatagramSize());
        auto readBytes = m_socket->readDatagram(
            reinterpret_cast<char*>(&data[0]),
            data.size(),
            &senderAddress,
            &senderPort);
        assert(readBytes == static_cast<decltype(readBytes)>(data.size()));

        if (m_socket->state() != QUdpSocket::ConnectedState) {
            m_socket->connectToHost(senderAddress, senderPort);
            m_socket->waitForConnected();
            assert(m_socket->isOpen());
            assert(m_socket->state() == QUdpSocket::ConnectedState);
        }

        m_session.dataFromClient(&data[0], readBytes);
    }
}

void SessionWrapper::socketErrorOccurred(QAbstractSocket::SocketError err)
{
    static_cast<void>(err);
    assert(m_socket);
    std::cout << "ERROR: UDP Socket: " << m_socket->errorString().toStdString() << std::endl;
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
    assert(m_socket);
    std::cout << "Sending " << bufSize << " bytes to client" << std::endl;
    std::size_t writtenCount = 0;
    while (writtenCount < bufSize) {
        auto remSize = bufSize - writtenCount;
        auto count =
            m_socket->write(
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
}

void SessionWrapper::termSession()
{
    std::cout << "INFO: Termination requested:" << std::endl;
    assert(m_socket);
    m_socket->blockSignals(true);
    m_timer.stop();
    deleteLater();
}

void SessionWrapper::reconnectBroker()
{
    assert(!"NYI");
}

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn


