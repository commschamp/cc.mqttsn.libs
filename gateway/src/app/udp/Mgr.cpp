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

namespace mqttsn
{

namespace gateway
{

namespace app
{

namespace udp
{

bool Mgr::start()
{
    m_port = 20000;

    if (!doListen()) {
        std::cerr << "ERROR: Failed to listen to incomming connections" << std::endl;
        return false;
    }

    m_gw.setLocalPort(m_port);
    m_gw.setBroadcastPort(21000);
    return m_gw.start();
}

void Mgr::newConnection()
{
    std::cout << "New connection" << std::endl;
    disconnect(
        m_socket.get(), SIGNAL(readyRead()),
        this, SLOT(newConnection()));

    std::unique_ptr<SessionWrapper> session(
        new SessionWrapper(m_configParser, std::move(m_socket), this));
    if (session->start()) {
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

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
