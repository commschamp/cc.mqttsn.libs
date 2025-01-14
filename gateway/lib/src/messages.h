//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "cc_mqttsn/Message.h"
#include "cc_mqttsn/frame/Frame.h"
#include "cc_mqttsn/input/AllMessages.h"
#include "cc_mqttsn/input/GwServerInputMessages.h"
#include "cc_mqttsn/options/ServerDefaultOptions.h"
#include "cc_mqttsn/options/DataViewDefaultOptions.h"
#include "cc_mqtt311/Message.h"
#include "cc_mqtt311/frame/Frame.h"
#include "cc_mqtt311/input/AllMessages.h"
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

// Aliases to all MQTT-SN messages:
/// using AdvertiseMsg_SN = cc_mqttsn::message::Advertise<MqttsnMessage, MqttsnGwOptions>;
/// using SearchgwMsg_SN = cc_mqttsn::message::Searchgw<MqttsnMessage, MqttsnGwOptions>;
/// ...
CC_MQTTSN_ALIASES_FOR_ALL_MESSAGES(, Msg_SN, MqttsnMessage, MqttsnGwOptions)    

template <typename TMsgBase>
using InputMqttsnMessages = cc_mqttsn::input::GwServerInputMessages<TMsgBase, MqttsnGwOptions>;

using MqttsnFrame =
    cc_mqttsn::frame::Frame<MqttsnMessage, InputMqttsnMessages<MqttsnMessage>, MqttsnGwOptions>;
    
    
using Mqtt311GwOptions = cc_mqtt311::options::ClientDefaultOptions;

// Aliases to all MQTT-SN messages:
/// using ConnectMsg = cc_mqtt311::message::Connect<MqttsnMessage, MqttsnGwOptions>;
/// using ConnackMsg = cc_mqtt311::message::Connack<MqttsnMessage, MqttsnGwOptions>;
/// ...
CC_MQTT311_ALIASES_FOR_ALL_MESSAGES(, Msg, MqttMessage, Mqtt311GwOptions)   

template <typename TMsgBase>
using InputMqtt311Messages = cc_mqtt311::input::ClientInputMessages<TMsgBase, Mqtt311GwOptions>;

using MqttFrame = 
    cc_mqtt311::frame::Frame<MqttMessage, InputMqtt311Messages<MqttMessage>, Mqtt311GwOptions> ;

}  // namespace cc_mqttsn_gateway
