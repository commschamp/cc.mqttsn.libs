//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <list>
#include <vector>
#include <cstdint>

#include <QtCore/QObject>
#include <QtNetwork/QUdpSocket>

#include "cc_mqttsn/gateway/Config.h"
#include "GatewayWrapper.h"
#include "SessionWrapper.h"

namespace cc_mqttsn
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
    ~Mgr();
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

}  // namespace cc_mqttsn
