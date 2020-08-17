//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "mqttsn/Message.h"
#include "mqttsn/frame/Frame.h"
#include "mqttsn/input/ServerInputMessages.h"
#include "mqttsn/options/ServerDefaultOptions.h"
#include "mqttsn/options/DataViewDefaultOptions.h"
#include "mqtt311/Message.h"
#include "mqtt311/frame/Frame.h"
#include "mqtt311/input/ClientInputMessages.h"
#include "mqtt311/options/ClientDefaultOptions.h"
#include "mqtt311/options/DataViewDefaultOptions.h"

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

using MqttsnGwOptions = 
    mqttsn::options::DataViewDefaultOptionsT<
        mqttsn::options::ServerDefaultOptions
    >;
    
typedef mqttsn::message::Advertise<MqttsnMessage, MqttsnGwOptions> AdvertiseMsg_SN;
typedef mqttsn::message::Searchgw<MqttsnMessage, MqttsnGwOptions> SearchgwMsg_SN;
typedef mqttsn::message::Gwinfo<MqttsnMessage, MqttsnGwOptions> GwinfoMsg_SN;
typedef mqttsn::message::Connect<MqttsnMessage, MqttsnGwOptions> ConnectMsg_SN;
typedef mqttsn::message::Connack<MqttsnMessage, MqttsnGwOptions> ConnackMsg_SN;
typedef mqttsn::message::Willtopicreq<MqttsnMessage, MqttsnGwOptions> WilltopicreqMsg_SN;
typedef mqttsn::message::Willtopic<MqttsnMessage, MqttsnGwOptions> WilltopicMsg_SN;
typedef mqttsn::message::Willmsgreq<MqttsnMessage, MqttsnGwOptions> WillmsgreqMsg_SN;
typedef mqttsn::message::Willmsg<MqttsnMessage, MqttsnGwOptions> WillmsgMsg_SN;
typedef mqttsn::message::Register<MqttsnMessage, MqttsnGwOptions> RegisterMsg_SN;
typedef mqttsn::message::Regack<MqttsnMessage, MqttsnGwOptions> RegackMsg_SN;
typedef mqttsn::message::Publish<MqttsnMessage, MqttsnGwOptions> PublishMsg_SN;
typedef mqttsn::message::Puback<MqttsnMessage, MqttsnGwOptions> PubackMsg_SN;
typedef mqttsn::message::Pubrec<MqttsnMessage, MqttsnGwOptions> PubrecMsg_SN;
typedef mqttsn::message::Pubrel<MqttsnMessage, MqttsnGwOptions> PubrelMsg_SN;
typedef mqttsn::message::Pubcomp<MqttsnMessage, MqttsnGwOptions> PubcompMsg_SN;
typedef mqttsn::message::Subscribe<MqttsnMessage, MqttsnGwOptions> SubscribeMsg_SN;
typedef mqttsn::message::Suback<MqttsnMessage, MqttsnGwOptions> SubackMsg_SN;
typedef mqttsn::message::Unsubscribe<MqttsnMessage, MqttsnGwOptions> UnsubscribeMsg_SN;
typedef mqttsn::message::Unsuback<MqttsnMessage, MqttsnGwOptions> UnsubackMsg_SN;
typedef mqttsn::message::Pingreq<MqttsnMessage, MqttsnGwOptions> PingreqMsg_SN;
typedef mqttsn::message::Pingresp<MqttsnMessage, MqttsnGwOptions> PingrespMsg_SN;
typedef mqttsn::message::Disconnect<MqttsnMessage, MqttsnGwOptions> DisconnectMsg_SN;
typedef mqttsn::message::Willtopicupd<MqttsnMessage, MqttsnGwOptions> WilltopicupdMsg_SN;
typedef mqttsn::message::Willtopicresp<MqttsnMessage, MqttsnGwOptions> WilltopicrespMsg_SN;
typedef mqttsn::message::Willmsgupd<MqttsnMessage, MqttsnGwOptions> WillmsgupdMsg_SN;
typedef mqttsn::message::Willmsgresp<MqttsnMessage, MqttsnGwOptions> WillmsgrespMsg_SN;

template <typename TMsgBase>
using InputMqttsnMessages = mqttsn::input::ServerInputMessages<TMsgBase, MqttsnGwOptions>;

using MqttsnProtStack =
    mqttsn::frame::Frame<MqttsnMessage, InputMqttsnMessages<MqttsnMessage> >;
    
    
//using Mqtt311GwOptions = mqtt311::options::ClientDefaultOptions;

// TODO: Currently gateway is implemented to receive PING and DISCONNECT from server,
// According to protocol spec these are client only messages. Consider using ClientDefaultOptions in the future
using Mqtt311GwOptions = mqtt311::options::DefaultOptions; 

typedef mqtt311::message::Connect<MqttMessage, Mqtt311GwOptions> ConnectMsg;
typedef mqtt311::message::Connack<MqttMessage, Mqtt311GwOptions> ConnackMsg;
typedef mqtt311::message::Publish<MqttMessage, Mqtt311GwOptions> PublishMsg;
typedef mqtt311::message::Puback<MqttMessage, Mqtt311GwOptions> PubackMsg;
typedef mqtt311::message::Pubrec<MqttMessage, Mqtt311GwOptions> PubrecMsg;
typedef mqtt311::message::Pubrel<MqttMessage, Mqtt311GwOptions> PubrelMsg;
typedef mqtt311::message::Pubcomp<MqttMessage, Mqtt311GwOptions> PubcompMsg;
typedef mqtt311::message::Subscribe<MqttMessage, Mqtt311GwOptions> SubscribeMsg;
typedef mqtt311::message::Suback<MqttMessage, Mqtt311GwOptions> SubackMsg;
typedef mqtt311::message::Unsubscribe<MqttMessage, Mqtt311GwOptions> UnsubscribeMsg;
typedef mqtt311::message::Unsuback<MqttMessage, Mqtt311GwOptions> UnsubackMsg;
typedef mqtt311::message::Pingreq<MqttMessage, Mqtt311GwOptions> PingreqMsg;
typedef mqtt311::message::Pingresp<MqttMessage, Mqtt311GwOptions> PingrespMsg;
typedef mqtt311::message::Disconnect<MqttMessage, Mqtt311GwOptions> DisconnectMsg;

// template <typename TMsgBase>
// using InputMqtt311Messages = mqtt311::input::ClientInputMessages<TMsgBase, Mqtt311GwOptions>;

// TODO: Currently gateway is implemented to receive PING and DISCONNECT from server,
// According to protocol spec these are client only messages. Consider using ClientInputMessages in the future
template <typename TMsgBase>
using InputMqtt311Messages = mqtt311::input::AllMessages<TMsgBase, Mqtt311GwOptions>;

using MqttProtStack = 
    mqtt311::frame::Frame<MqttMessage, InputMqtt311Messages<MqttMessage> > ;

}  // namespace gateway

}  // namespace mqttsn


