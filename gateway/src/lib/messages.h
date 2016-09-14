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

#include "mqttsn/protocol/Message.h"
#include "mqttsn/protocol/Stack.h"
#include "mqtt/Message.h"
#include "mqtt/Stack.h"

namespace mqttsn
{

namespace gateway
{

class MsgHandler;

typedef mqttsn::protocol::MessageT<
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::Handler<MsgHandler>
> MqttsnMessage;

typedef mqtt::MessageT<
    comms::option::BigEndian,
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::MsgIdType<mqtt::MsgId>,
    comms::option::Handler<MsgHandler>,
    comms::option::RefreshInterface,
    comms::option::ValidCheckInterface,
    comms::option::LengthInfoInterface
> MqttMessage;

typedef mqttsn::protocol::message::Advertise<MqttsnMessage> AdvertiseMsg_SN;
typedef mqttsn::protocol::message::Searchgw<MqttsnMessage> SearchgwMsg_SN;
typedef mqttsn::protocol::message::Gwinfo<MqttsnMessage> GwinfoMsg_SN;
typedef mqttsn::protocol::message::Connect<MqttsnMessage> ConnectMsg_SN;
typedef mqttsn::protocol::message::Connack<MqttsnMessage> ConnackMsg_SN;
typedef mqttsn::protocol::message::Willtopicreq<MqttsnMessage> WilltopicreqMsg_SN;
typedef mqttsn::protocol::message::Willtopic<MqttsnMessage> WilltopicMsg_SN;
typedef mqttsn::protocol::message::Willmsgreq<MqttsnMessage> WillmsgreqMsg_SN;
typedef mqttsn::protocol::message::Willmsg<MqttsnMessage> WillmsgMsg_SN;
typedef mqttsn::protocol::message::Register<MqttsnMessage> RegisterMsg_SN;
typedef mqttsn::protocol::message::Regack<MqttsnMessage> RegackMsg_SN;
typedef mqttsn::protocol::message::Publish<MqttsnMessage> PublishMsg_SN;
typedef mqttsn::protocol::message::Puback<MqttsnMessage> PubackMsg_SN;
typedef mqttsn::protocol::message::Pubrec<MqttsnMessage> PubrecMsg_SN;
typedef mqttsn::protocol::message::Pubrel<MqttsnMessage> PubrelMsg_SN;
typedef mqttsn::protocol::message::Pubcomp<MqttsnMessage> PubcompMsg_SN;
typedef mqttsn::protocol::message::Subscribe<MqttsnMessage> SubscribeMsg_SN;
typedef mqttsn::protocol::message::Suback<MqttsnMessage> SubackMsg_SN;
typedef mqttsn::protocol::message::Unsubscribe<MqttsnMessage> UnsubscribeMsg_SN;
typedef mqttsn::protocol::message::Unsuback<MqttsnMessage> Unsuback_SN;
typedef mqttsn::protocol::message::Pingreq<MqttsnMessage> PingreqMsg_SN;
typedef mqttsn::protocol::message::Pingresp<MqttsnMessage> PingrespMsg_SN;
typedef mqttsn::protocol::message::Disconnect<MqttsnMessage> DisconnectMsg_SN;
typedef mqttsn::protocol::message::Willtopicupd<MqttsnMessage> Willtopicupd_SN;
typedef mqttsn::protocol::message::Willtopicresp<MqttsnMessage> Willtopicresp_SN;
typedef mqttsn::protocol::message::Willmsgupd<MqttsnMessage> Willmsgupd_SN;
typedef mqttsn::protocol::message::Willmsgresp<MqttsnMessage> Willmsgresp_SN;
typedef mqttsn::protocol::Stack<MqttsnMessage> MqttsnProtStack;

typedef mqtt::message::Connect<MqttMessage> ConnectMsg;
typedef mqtt::message::Connack<MqttMessage> ConnackMsg;
typedef mqtt::message::Publish<MqttMessage> PublishMsg;
typedef mqtt::message::Puback<MqttMessage> PubackMsg;
typedef mqtt::message::Pubrec<MqttMessage> PubrecMsg;
typedef mqtt::message::Pubrel<MqttMessage> PubrelMsg;
typedef mqtt::message::Pubcomp<MqttMessage> PubcompMsg;
typedef mqtt::message::Subscribe<MqttMessage> SubscribeMsg;
typedef mqtt::message::Suback<MqttMessage> SubackMsg;
typedef mqtt::message::Unsubscribe<MqttMessage> UnsubscribeMsg;
typedef mqtt::message::Unsuback<MqttMessage> UnsubackMsg;
typedef mqtt::message::Pingreq<MqttMessage> PingreqMsg;
typedef mqtt::message::Pingresp<MqttMessage> PingrespMsg;
typedef mqtt::message::Disconnect<MqttMessage> DisconnectMsg;
typedef mqtt::Stack<MqttMessage> MqttProtStack;

}  // namespace gateway

}  // namespace mqttsn


