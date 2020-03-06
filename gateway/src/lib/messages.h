//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "mqttsn/Message.h"
#include "mqttsn/frame/Frame.h"
#include "mqttsn/input/ServerInputMessages.h"
#include "mqtt311/Message.h"
#include "mqtt311/frame/Frame.h"

namespace mqttsn
{

namespace gateway
{

class MsgHandler;

using MqttsnMessage =
    mqttsn::Message<
        comms::option::IdInfoInterface,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>,
        comms::option::Handler<MsgHandler>,
        comms::option::LengthInfoInterface
    >;

using MqttMessage =
    mqtt311::Message<
        comms::option::IdInfoInterface,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>,
        comms::option::Handler<MsgHandler>,
        comms::option::LengthInfoInterface
    >;

//typedef protocol::ParsedOptions<
//    protocol::option::GatewayOnlyVariant,
//    protocol::option::UseOrigDataView
//> GwOptions;

using GwOptions = mqttsn::options::DefaultOptions;

typedef mqttsn::message::Advertise<MqttsnMessage, GwOptions> AdvertiseMsg_SN;
typedef mqttsn::message::Searchgw<MqttsnMessage, GwOptions> SearchgwMsg_SN;
typedef mqttsn::message::Gwinfo<MqttsnMessage, GwOptions> GwinfoMsg_SN;
typedef mqttsn::message::Connect<MqttsnMessage, GwOptions> ConnectMsg_SN;
typedef mqttsn::message::Connack<MqttsnMessage, GwOptions> ConnackMsg_SN;
typedef mqttsn::message::Willtopicreq<MqttsnMessage, GwOptions> WilltopicreqMsg_SN;
typedef mqttsn::message::Willtopic<MqttsnMessage, GwOptions> WilltopicMsg_SN;
typedef mqttsn::message::Willmsgreq<MqttsnMessage, GwOptions> WillmsgreqMsg_SN;
typedef mqttsn::message::Willmsg<MqttsnMessage, GwOptions> WillmsgMsg_SN;
typedef mqttsn::message::Register<MqttsnMessage, GwOptions> RegisterMsg_SN;
typedef mqttsn::message::Regack<MqttsnMessage, GwOptions> RegackMsg_SN;
typedef mqttsn::message::Publish<MqttsnMessage, GwOptions> PublishMsg_SN;
typedef mqttsn::message::Puback<MqttsnMessage, GwOptions> PubackMsg_SN;
typedef mqttsn::message::Pubrec<MqttsnMessage, GwOptions> PubrecMsg_SN;
typedef mqttsn::message::Pubrel<MqttsnMessage, GwOptions> PubrelMsg_SN;
typedef mqttsn::message::Pubcomp<MqttsnMessage, GwOptions> PubcompMsg_SN;
typedef mqttsn::message::Subscribe<MqttsnMessage, GwOptions> SubscribeMsg_SN;
typedef mqttsn::message::Suback<MqttsnMessage, GwOptions> SubackMsg_SN;
typedef mqttsn::message::Unsubscribe<MqttsnMessage, GwOptions> UnsubscribeMsg_SN;
typedef mqttsn::message::Unsuback<MqttsnMessage, GwOptions> UnsubackMsg_SN;
typedef mqttsn::message::Pingreq<MqttsnMessage, GwOptions> PingreqMsg_SN;
typedef mqttsn::message::Pingresp<MqttsnMessage, GwOptions> PingrespMsg_SN;
typedef mqttsn::message::Disconnect<MqttsnMessage, GwOptions> DisconnectMsg_SN;
typedef mqttsn::message::Willtopicupd<MqttsnMessage, GwOptions> WilltopicupdMsg_SN;
typedef mqttsn::message::Willtopicresp<MqttsnMessage, GwOptions> WilltopicrespMsg_SN;
typedef mqttsn::message::Willmsgupd<MqttsnMessage, GwOptions> WillmsgupdMsg_SN;
typedef mqttsn::message::Willmsgresp<MqttsnMessage, GwOptions> WillmsgrespMsg_SN;

template <typename TMsgBase>
using InputMqttsnMessages = mqttsn::input::ServerInputMessages<TMsgBase, GwOptions>;

using MqttsnProtStack =
    mqttsn::frame::Frame<MqttsnMessage, InputMqttsnMessages<MqttsnMessage> >;

typedef mqtt311::message::Connect<MqttMessage> ConnectMsg;
typedef mqtt311::message::Connack<MqttMessage> ConnackMsg;
typedef mqtt311::message::Publish<MqttMessage> PublishMsg;
typedef mqtt311::message::Puback<MqttMessage> PubackMsg;
typedef mqtt311::message::Pubrec<MqttMessage> PubrecMsg;
typedef mqtt311::message::Pubrel<MqttMessage> PubrelMsg;
typedef mqtt311::message::Pubcomp<MqttMessage> PubcompMsg;
typedef mqtt311::message::Subscribe<MqttMessage> SubscribeMsg;
typedef mqtt311::message::Suback<MqttMessage> SubackMsg;
typedef mqtt311::message::Unsubscribe<MqttMessage> UnsubscribeMsg;
typedef mqtt311::message::Unsuback<MqttMessage> UnsubackMsg;
typedef mqtt311::message::Pingreq<MqttMessage> PingreqMsg;
typedef mqtt311::message::Pingresp<MqttMessage> PingrespMsg;
typedef mqtt311::message::Disconnect<MqttMessage> DisconnectMsg;
typedef mqtt311::frame::Frame<MqttMessage> MqttProtStack;

}  // namespace gateway

}  // namespace mqttsn


