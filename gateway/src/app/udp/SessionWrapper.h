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
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
CC_ENABLE_WARNINGS()

#include "mqttsn/gateway/Config.h"
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
    typedef unsigned short PortType;
    typedef mqttsn::gateway::Session::AuthInfo AuthInfo;

    SessionWrapper(const Config& config, QObject* parent);
    ~SessionWrapper();


    typedef std::function<void (const SessionWrapper&)> TermNotifyCb;
    template <typename TFunc>
    void setTermNotifyCb(TFunc&& cb)
    {
        m_termNotifyCb = std::forward<TFunc>(cb);
    }

    template <typename TFunc>
    void setSendDataReqCb(TFunc&& cb)
    {
        m_session.setSendDataClientReqCb(std::forward<TFunc>(cb));
    }

    bool start();

    void dataFromClient(const std::uint8_t* buf, const std::size_t bufLen)
    {
        m_session.dataFromClient(buf, bufLen);
    }

    void setClientAddr(const QString& value)
    {
        m_clientAddr = value;
    }

    const QString& getClientAddr() const
    {
        return m_clientAddr;
    }

    void setClientPort(PortType value)
    {
        m_clientPort = value;
    }

    PortType getClientPort() const
    {
        return m_clientPort;
    }

private slots:
    void tickTimeout();
    void brokerConnected();
    void brokerDisconnected();
    void readFromBrokerSocket();
    void brokerSocketErrorOccurred(QAbstractSocket::SocketError err);

private:
    typedef std::vector<std::uint8_t> DataBuf;

    void programNextTick(unsigned ms);
    unsigned cancelTick();
    void sendDataToBroker(const std::uint8_t* buf, std::size_t bufSize);
    void termSession();
    void reconnectBroker();
    void connectToBroker();
    void addPredefinedTopicsFor(const std::string& clientId);
    AuthInfo getAuthInfoFor(const std::string& clientId);

    const Config& m_config;
    QTcpSocket m_brokerSocket;
    mqttsn::gateway::Session m_session;
    QTimer m_timer;
    unsigned m_reqTicks = 0;
    bool m_reconnectRequested = false;
    DataBuf m_brokerData;
    TermNotifyCb m_termNotifyCb;
    QString m_clientAddr;
    PortType m_clientPort = 0;
};

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn
