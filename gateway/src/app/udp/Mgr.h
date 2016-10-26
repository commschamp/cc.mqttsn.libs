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

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QObject>
#include <QtNetwork/QUdpSocket>
CC_ENABLE_WARNINGS()

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
    Mgr() = default;

    bool start();

private slots:
    void newConnection();

private:
    typedef SessionWrapper::ClientSocketPtr SocketPtr;
    typedef unsigned short PortType;

    bool doListen();

    PortType m_port = 0;
    SocketPtr m_socket;
    GatewayWrapper m_gw;
};

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
