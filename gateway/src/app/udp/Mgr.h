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
#include <list>
#include <vector>
#include <cstdint>


#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QObject>
#include <QtNetwork/QUdpSocket>
CC_ENABLE_WARNINGS()

#include "mqttsn/gateway/Config.h"
#include "GatewayWrapper.h"
#include "SessionWrapper.h"

namespace mqttsn
{

namespace gateway
{

namespace app
{

namespace udp
{

class Mgr : public QObject
{
    Q_OBJECT
public:
    explicit Mgr(const Config& config);
    bool start();

private slots:
    void readClientData();
    void socketErrorOccurred(QAbstractSocket::SocketError err);

private:
    typedef unsigned short PortType;
    typedef std::map<QString, SessionWrapper*> SessionMap;

    bool doListen();
    void sendToClient(
        const SessionWrapper& session,
        const std::uint8_t* buf,
        std::size_t bufSize);
    void broadcastAdvertise(const std::uint8_t* buf, std::size_t bufSize);

    const Config& m_config;
    PortType m_port = 0;
    PortType m_broadcastPort = 0;
    QUdpSocket m_socket;
    GatewayWrapper m_gw;
    std::vector<std::uint8_t> m_lastAdvertise;
    SessionMap m_sessions;
};

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
