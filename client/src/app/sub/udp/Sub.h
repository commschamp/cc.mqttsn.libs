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
#include <type_traits>

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "client.h"

namespace mqttsn
{

namespace client
{

namespace app
{

namespace sub
{

namespace udp
{

class Sub : public QObject
{
    Q_OBJECT
public:
    Sub();

private slots:
    void tick();

private:
    struct ClientDeleter
    {
        void operator()(MqttsnClientHandle client)
        {
            mqttsn_client_free(client);
        }
    };

    typedef std::unique_ptr<
        typename std::remove_pointer<MqttsnClientHandle>::type,
        ClientDeleter
    > ClientPtr;

    void nextTickProgram(unsigned ms);
    static void nextTickProgramCb(void* obj, unsigned ms);

    unsigned cancelTick();
    static unsigned caneclTickCb(void* obj);

    void sendData(const unsigned char* buf, unsigned bufLen, bool broadcast);
    static void sendDataCb(void* obj, const unsigned char* buf, unsigned bufLen, bool broadcast);

    void gwStatusReport(unsigned short gwId, MqttsnGwStatus status);
    static void gwStatusReportCb(void* obj, unsigned short gwId, MqttsnGwStatus status);

    void connectionStatusReport(MqttsnConnectionStatus status);
    static void connectionStatusReportCb(void* obj, MqttsnConnectionStatus status);

    void messageReport(const MqttsnMessageInfo* msgInfo);
    static void messageReportCb(void* obj, const MqttsnMessageInfo* msgInfo);

    ClientPtr m_client;
    QTimer m_timer;
    unsigned m_reqTimeout = 0;
};

}  // namespace udp

}  // namespace sub

}  // namespace app

}  // namespace client

}  // namespace mqttsn


