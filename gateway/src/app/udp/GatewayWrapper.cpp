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

#include "GatewayWrapper.h"

#include <iostream>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace app
{

namespace udp
{

GatewayWrapper::GatewayWrapper(const Config& config)
  : m_config(config)
{
    connect(
        &m_timer, SIGNAL(timeout()),
        this, SLOT(tickTimeout()));
}

bool GatewayWrapper::start(SendDataReqCb&& sendCb)
{
    m_gw.setAdvertisePeriod(m_config.advertisePeriod());
    m_gw.setGatewayId(m_config.gatewayId());
    m_gw.setNextTickProgramReqCb(
        [this](unsigned ms)
        {
            m_timer.setSingleShot(true);
            m_timer.start(ms);
        });

    m_gw.setSendDataReqCb(std::move(sendCb));
    return m_gw.start();
}

void GatewayWrapper::tickTimeout()
{
    m_gw.tick();
}

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn


