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

#include "Mgr.h"

#include <cassert>
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

namespace
{

const std::string UdpListenPortKey("udp_listen_port");
const std::string UdpBroadcastPortKey("udp_broadcast_port");
const std::string SpaceChars(" \t");
const std::uint16_t DefaultListenPort = 1883;
const std::uint16_t DefaultBroadcastPort = 1883;

std::uint16_t getPortInfo(
    const Config& config,
    const std::string& key,
    std::uint16_t defaultValue)
{
    auto& map = config.configMap();
    auto iter = map.find(key);
    if ((iter == map.end()) ||
        (iter->second.empty())) {
        return defaultValue;
    }

    try {
        auto& valStr = iter->second;
        auto spacePos = valStr.find_first_of(SpaceChars);
        if (spacePos == std::string::npos) {
            return static_cast<std::uint16_t>(std::stoul(valStr));
        }


        std::string portStr(valStr.begin(), valStr.begin() + spacePos);
        return static_cast<std::uint16_t>(std::stoul(portStr));
    }
    catch (...) {
        // nothing to do
    }

    return defaultValue;
}

}  // namespace

bool Mgr::start()
{
    m_port = getPortInfo(m_config, UdpListenPortKey, DefaultListenPort);
    if (!doListen()) {
        std::cerr << "ERROR: Failed to listen to incomming connections" << std::endl;
        return false;
    }

    if (m_config.advertisePeriod() == 0) {
        return true;
    }

    m_broadcastPort = getPortInfo(m_config, UdpBroadcastPortKey, DefaultBroadcastPort);
    auto broadcastFunc =
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            broadcastAdvertise(buf, bufSize);
        };
    return m_gw.start(std::move(broadcastFunc));
}

void Mgr::newConnection()
{
    disconnect(
        m_socket.get(), SIGNAL(readyRead()),
        this, SLOT(newConnection()));

    auto selfAdvertiseCheck =
        [this](const std::uint8_t* buf, std::size_t bufLen) -> bool
        {
            return
                ((m_lastAdvertise.size() == bufLen) &&
                 (std::equal(m_lastAdvertise.begin(), m_lastAdvertise.end(), buf)));
        };

    std::unique_ptr<SessionWrapper> session(
        new SessionWrapper(m_config, std::move(m_socket), this));
    if (session->start(selfAdvertiseCheck)) {
        session.release();
    }

    doListen();
}

bool Mgr::doListen()
{
    assert(!m_socket);

    if (m_port == 0) {
        return false;
    }

    m_socket.reset(new QUdpSocket());
    if (!m_socket->bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        std::cerr << "ERROR: Failed to bind UDP socket to local port " << m_port << std::endl;
        return false;
    }

    if (!m_socket->open(QUdpSocket::ReadWrite)) {
        std::cerr << "ERROR: Failed to open UDP socket" << std::endl;
        return false;
    }

    connect(
        m_socket.get(), SIGNAL(readyRead()),
        this, SLOT(newConnection()));
    return true;
}

void Mgr::broadcastAdvertise(const std::uint8_t* buf, std::size_t bufSize)
{
    m_lastAdvertise.assign(buf, buf + bufSize);
    std::size_t writtenCount = 0;
    while (writtenCount < bufSize) {
        auto remSize = bufSize - writtenCount;
        auto count =
            m_socket->writeDatagram(
                reinterpret_cast<const char*>(&buf[writtenCount]),
                remSize,
                QHostAddress::Broadcast,
                m_broadcastPort);

        if (count < 0) {
            return;
        }

        writtenCount += count;
    }
}

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
