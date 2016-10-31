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

bool GatewayWrapper::isSelfAdvertise(const std::uint8_t* buf, std::size_t bufLen) const
{
    return
        ((m_lastAdvertise.size() == bufLen) &&
         (std::equal(m_lastAdvertise.begin(), m_lastAdvertise.end(), buf)));
}

bool GatewayWrapper::start()
{
    if ((m_localPort != 0) &&
        (!m_socket.bind(QHostAddress::AnyIPv4, m_localPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))) {
        std::cerr << "ERROR: Failed to bind UDP socket to local port " << m_localPort << std::endl;
        return false;
    }

    if (!m_socket.open(QUdpSocket::WriteOnly)) {
        std::cerr << "ERROR: Failed to open UDP socket" << std::endl;
        return false;
    }

    m_gw.setAdvertisePeriod(m_config.advertisePeriod());
    m_gw.setGatewayId(m_config.gatewayId());
    m_gw.setNextTickProgramReqCb(
        [this](unsigned ms)
        {
            m_timer.setSingleShot(true);
            m_timer.start(ms);
        });

    m_gw.setSendDataReqCb(
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            m_lastAdvertise.assign(buf, buf + bufSize);
            PortType broadcastPort = m_broadcastPort;
            if (broadcastPort == 0) {
                broadcastPort = m_localPort;
            }
            std::size_t writtenCount = 0;
            while (writtenCount < bufSize) {
                auto remSize = bufSize - writtenCount;
                auto count =
                    m_socket.writeDatagram(
                        reinterpret_cast<const char*>(&buf[writtenCount]),
                        remSize,
                        QHostAddress::Broadcast,
                        broadcastPort);

                std::cout << "Broadcasted " << count << " bytes" << std::endl;
                if (count < 0) {
                    return;
                }

                writtenCount += count;
            }

        });

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


