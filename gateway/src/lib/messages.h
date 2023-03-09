//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "cc_mqttsn/Message.h"
#include "cc_mqttsn/frame/Frame.h"
#include "cc_mqttsn/input/ServerInputMessages.h"
#include "cc_mqttsn/options/ServerDefaultOptions.h"
#include "cc_mqttsn/options/DataViewDefaultOptions.h"
#include "cc_mqtt311/Message.h"
#include "cc_mqtt311/frame/Frame.h"
#include "cc_mqtt311/input/ClientInputMessages.h"
#include "cc_mqtt311/options/ClientDefaultOptions.h"
#include "cc_mqtt311/options/DataViewDefaultOptions.h"

namespace cc_mqttsn_gateway
{

class MsgHandler;

using MqttsnMessage =
    cc_mqttsn::Message<
        comms::option::IdInfoInterface,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>,
        comms::option::Handler<MsgHandler>,
        comms::option::LengthInfoInterface
    >;

using MqttMessage =
    cc_mqtt311::Message<
        comms::option::IdInfoInterface,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>,
        comms::option::Handler<MsgHandler>,
        comms::option::LengthInfoInterface
    >;

using MqttsnGwOptions = 
    cc_mqttsn::options::DataViewDefaultOptionsT<
        cc_mqttsn::options::ServerDefaultOptions
    >;
    
typedef cc_mqttsn::message::Advertise<MqttsnMessage, MqttsnGwOptions> AdvertiseMsg_SN;
typedef cc_mqttsn::message::Searchgw<MqttsnMessage, MqttsnGwOptions> SearchgwMsg_SN;
typedef cc_mqttsn::message::Gwinfo<MqttsnMessage, MqttsnGwOptions> GwinfoMsg_SN;
typedef cc_mqttsn::message::Connect<MqttsnMessage, MqttsnGwOptions> ConnectMsg_SN;
typedef cc_mqttsn::message::Connack<MqttsnMessage, MqttsnGwOptions> ConnackMsg_SN;
typedef cc_mqttsn::message::Willtopicreq<MqttsnMessage, MqttsnGwOptions> WilltopicreqMsg_SN;
typedef cc_mqttsn::message::Willtopic<MqttsnMessage, MqttsnGwOptions> WilltopicMsg_SN;
typedef cc_mqttsn::message::Willmsgreq<MqttsnMessage, MqttsnGwOptions> WillmsgreqMsg_SN;
typedef cc_mqttsn::message::Willmsg<MqttsnMessage, MqttsnGwOptions> WillmsgMsg_SN;
typedef cc_mqttsn::message::Register<MqttsnMessage, MqttsnGwOptions> RegisterMsg_SN;
typedef cc_mqttsn::message::Regack<MqttsnMessage, MqttsnGwOptions> RegackMsg_SN;
typedef cc_mqttsn::message::Publish<MqttsnMessage, MqttsnGwOptions> PublishMsg_SN;
typedef cc_mqttsn::message::Puback<MqttsnMessage, MqttsnGwOptions> PubackMsg_SN;
typedef cc_mqttsn::message::Pubrec<MqttsnMessage, MqttsnGwOptions> PubrecMsg_SN;
typedef cc_mqttsn::message::Pubrel<MqttsnMessage, MqttsnGwOptions> PubrelMsg_SN;
typedef cc_mqttsn::message::Pubcomp<MqttsnMessage, MqttsnGwOptions> PubcompMsg_SN;
typedef cc_mqttsn::message::Subscribe<MqttsnMessage, MqttsnGwOptions> SubscribeMsg_SN;
typedef cc_mqttsn::message::Suback<MqttsnMessage, MqttsnGwOptions> SubackMsg_SN;
typedef cc_mqttsn::message::Unsubscribe<MqttsnMessage, MqttsnGwOptions> UnsubscribeMsg_SN;
typedef cc_mqttsn::message::Unsuback<MqttsnMessage, MqttsnGwOptions> UnsubackMsg_SN;
typedef cc_mqttsn::message::Pingreq<MqttsnMessage, MqttsnGwOptions> PingreqMsg_SN;
typedef cc_mqttsn::message::Pingresp<MqttsnMessage, MqttsnGwOptions> PingrespMsg_SN;
typedef cc_mqttsn::message::Disconnect<MqttsnMessage, MqttsnGwOptions> DisconnectMsg_SN;
typedef cc_mqttsn::message::Willtopicupd<MqttsnMessage, MqttsnGwOptions> WilltopicupdMsg_SN;
typedef cc_mqttsn::message::Willtopicresp<MqttsnMessage, MqttsnGwOptions> WilltopicrespMsg_SN;
typedef cc_mqttsn::message::Willmsgupd<MqttsnMessage, MqttsnGwOptions> WillmsgupdMsg_SN;
typedef cc_mqttsn::message::Willmsgresp<MqttsnMessage, MqttsnGwOptions> WillmsgrespMsg_SN;

template <typename TMsgBase>
using InputMqttsnMessages = cc_mqttsn::input::ServerInputMessages<TMsgBase, MqttsnGwOptions>;

using MqttsnProtStack =
    cc_mqttsn::frame::Frame<MqttsnMessage, InputMqttsnMessages<MqttsnMessage> >;
    
    
//using Mqtt311GwOptions = cc_mqtt311::options::ClientDefaultOptions;

// TODO: Currently gateway is implemented to receive PING and DISCONNECT from server,
// According to protocol spec these are client only messages. Consider using ClientDefaultOptions in the future
using Mqtt311GwOptions = cc_mqtt311::options::DefaultOptions; 

typedef cc_mqtt311::message::Connect<MqttMessage, Mqtt311GwOptions> ConnectMsg;
typedef cc_mqtt311::message::Connack<MqttMessage, Mqtt311GwOptions> ConnackMsg;
typedef cc_mqtt311::message::Publish<MqttMessage, Mqtt311GwOptions> PublishMsg;
typedef cc_mqtt311::message::Puback<MqttMessage, Mqtt311GwOptions> PubackMsg;
typedef cc_mqtt311::message::Pubrec<MqttMessage, Mqtt311GwOptions> PubrecMsg;
typedef cc_mqtt311::message::Pubrel<MqttMessage, Mqtt311GwOptions> PubrelMsg;
typedef cc_mqtt311::message::Pubcomp<MqttMessage, Mqtt311GwOptions> PubcompMsg;
typedef cc_mqtt311::message::Subscribe<MqttMessage, Mqtt311GwOptions> SubscribeMsg;
typedef cc_mqtt311::message::Suback<MqttMessage, Mqtt311GwOptions> SubackMsg;
typedef cc_mqtt311::message::Unsubscribe<MqttMessage, Mqtt311GwOptions> UnsubscribeMsg;
typedef cc_mqtt311::message::Unsuback<MqttMessage, Mqtt311GwOptions> UnsubackMsg;
typedef cc_mqtt311::message::Pingreq<MqttMessage, Mqtt311GwOptions> PingreqMsg;
typedef cc_mqtt311::message::Pingresp<MqttMessage, Mqtt311GwOptions> PingrespMsg;
typedef cc_mqtt311::message::Disconnect<MqttMessage, Mqtt311GwOptions> DisconnectMsg;

// template <typename TMsgBase>
// using InputMqtt311Messages = cc_mqtt311::input::ClientInputMessages<TMsgBase, Mqtt311GwOptions>;

// TODO: Currently gateway is implemented to receive PING and DISCONNECT from server,
// According to protocol spec these are client only messages. Consider using ClientInputMessages in the future
template <typename TMsgBase>
using InputMqtt311Messages = cc_mqtt311::input::AllMessages<TMsgBase, Mqtt311GwOptions>;

using MqttProtStack = 
    cc_mqtt311::frame::Frame<MqttMessage, InputMqtt311Messages<MqttMessage> > ;

}  // namespace cc_mqttsn_gateway
