//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

#include <QtCore/QObject>
#include <QtCore/QTimer>

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

    typedef mqttsn::gateway::Gateway::SendDataReqCb SendDataReqCb;
    bool start(SendDataReqCb&& sendCb);

private slots:
    void tickTimeout();

private:
    const Config& m_config;
    mqttsn::gateway::Gateway m_gw;
    QTimer m_timer;
};

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
