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

mqtt::field::QosType translateQosForBroker(QoS val)
{
    return static_cast<mqtt::field::QosType>(val);
}

mqttsn::protocol::field::QosType translateQosForClient(QoS val)
{
    return static_cast<mqttsn::protocol::field::QosType>(val);
}

QoS translateQos(mqtt::field::QosType val)
{
    return static_cast<QoS>(val);
}

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

struct ConnectionInfo
{
    std::string m_clientId;
    std::string m_username;
    DataBuf m_password;
    WillInfo m_will;
    std::uint16_t m_keepAlive = 0U;
    bool m_clean = false;
};


}  // namespace gateway

}  // namespace mqttsn

