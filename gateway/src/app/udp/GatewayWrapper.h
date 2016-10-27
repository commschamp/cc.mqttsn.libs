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

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtNetwork/QUdpSocket>
CC_ENABLE_WARNINGS()

#include "mqttsn/gateway/Config.h"
#include "mqttsn/gateway/Gateway.h"

namespace mqttsn
{

namespace gateway
{

namespace app
{

namespace udp
{

class GatewayWrapper : public QObject
{
    Q_OBJECT
public:
    typedef unsigned short PortType;

    explicit GatewayWrapper(const Config& config);

    void setLocalPort(PortType value)
    {
        m_localPort = value;
    }

    void setBroadcastPort(PortType value)
    {
        m_broadcastPort = value;
    }

    bool start();

private slots:
    void tickTimeout();

private:
    const Config& m_config;
    QUdpSocket m_socket;
    PortType m_localPort = 0;
    PortType m_broadcastPort = 0;
    mqttsn::gateway::Gateway m_gw;
    QTimer m_timer;
};

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
