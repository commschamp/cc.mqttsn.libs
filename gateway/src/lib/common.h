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

#include <string>
#include <vector>

#include "mqtt/field/QoS.h"
#include "mqttsn/protocol/field.h"


namespace mqttsn
{

namespace gateway
{

typedef std::vector<std::uint8_t> DataBuf;

enum QoS
{
    QoS_AtMostOnceDelivery,
    QoS_AtLeastOnceDelivery,
    QoS_ExactlyOnceDelivery,
    QoS_NumOfValues
};

inline
mqtt::field::QosType translateQosForBroker(QoS val)
{
    return static_cast<mqtt::field::QosType>(val);
}

inline
mqttsn::protocol::field::QosType translateQosForClient(QoS val)
{
    return static_cast<mqttsn::protocol::field::QosType>(val);
}

inline
QoS translateQos(mqtt::field::QosType val)
{
    return static_cast<QoS>(val);
}

inline
QoS translateQos(mqttsn::protocol::field::QosType val)
{
    if (val == mqttsn::protocol::field::QosType::NoGwPublish) {
        return QoS_AtMostOnceDelivery;
    }

    return static_cast<QoS>(val);
}

struct WillInfo
{
    std::string m_topic;
    DataBuf m_msg;
    QoS m_qos = QoS_AtMostOnceDelivery;
    bool m_retain = false;
};

inline
bool operator==(const WillInfo& info1, const WillInfo& info2)
{
    return
        ((info1.m_topic == info2.m_topic) &&
         (info1.m_msg == info2.m_msg) &&
         (info1.m_qos == info2.m_qos) &&
         (info1.m_retain == info2.m_retain));
}

inline
bool operator!=(const WillInfo& info1, const WillInfo& info2)
{
    return !(info1 == info2);
}


//struct ConnectionInfo
//{
//    std::string m_clientId;
//    WillInfo m_will;
//    std::uint16_t m_keepAlive = 0U;
//    bool m_clean = false;
//};

enum class ConnectionStatus
{
    Disconnected,
    Connected,
    Asleep
};

typedef unsigned long long Timestamp;

struct SessionState
{
    static const unsigned DefaultRetryPeriod = 15 * 1000;
    static const unsigned DefaultRetryCount = 3;

    unsigned m_retryPeriod = DefaultRetryPeriod;
    unsigned m_retryCount = DefaultRetryCount;
    bool m_running = false;
    bool m_connecting = false;
    bool m_brokerConnected = false;
    bool m_reconnecgingBroker = false;
    bool m_timerActive = false;
    Timestamp m_timestamp = 0U;
    unsigned m_callStackCount = 0U;

    ConnectionStatus m_connStatus = ConnectionStatus::Disconnected;
    std::string m_clientId;
    WillInfo m_will;
    std::uint8_t m_gwId = 0U;
    std::uint16_t m_keepAlive = 0U;
    std::string m_username;
    DataBuf m_password;
};

}  // namespace gateway

}  // namespace mqttsn

