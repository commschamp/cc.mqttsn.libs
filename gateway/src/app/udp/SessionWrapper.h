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
#include <vector>
#include <cstdint>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>
CC_ENABLE_WARNINGS()


#include "mqttsn/gateway/Session.h"

namespace mqttsn
{

namespace gateway
{

namespace app
{

namespace udp
{

class SessionWrapper : public QObject
{
    Q_OBJECT
    typedef QObject Base;
public:
    typedef std::unique_ptr<QUdpSocket> SocketPtr;
    typedef unsigned short PortType;

    SessionWrapper(SocketPtr socket, QObject* parent);
    ~SessionWrapper();

    bool start();

private slots:
    void tickTimeout();
    void doRead();
    void socketErrorOccurred(QAbstractSocket::SocketError err);

private:
    typedef std::vector<std::uint8_t> DataBuf;

    void programNextTick(unsigned ms);
    unsigned cancelTick();
    void sendDataToClient(const std::uint8_t* buf, std::size_t bufSize);
    void sendDataToBroker(const std::uint8_t* buf, std::size_t bufSize);
    void termSession();
    void reconnectBroker();

    SocketPtr m_socket;
    mqttsn::gateway::Session m_session;
    QTimer m_timer;
    unsigned m_reqTicks = 0;
};

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
