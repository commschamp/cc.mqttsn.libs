//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "comms/comms.h"
#include "messages.h"

namespace cc_mqttsn_gateway
{

template<typename TMsgBase>
using MqttsnMsgHandler = comms::GenericHandler<
    TMsgBase,
    cc_mqttsn_gateway::InputMqttsnMessages<TMsgBase>
>;

template<typename TMsgBase>
using MqttMsgHandler = comms::GenericHandler<
    TMsgBase,
    cc_mqttsn_gateway::InputMqtt311Messages<TMsgBase>
>;

class MsgHandler : public MqttsnMsgHandler<MqttsnMessage>, public MqttMsgHandler<MqttMessage>
{
    typedef MqttsnMsgHandler<MqttsnMessage> MqttsnBase;
    typedef MqttMsgHandler<MqttMessage> MqttBase;

public:
    using MqttsnBase::handle;
    using MqttBase::handle;

    virtual ~MsgHandler() = default;
};

}  // namespace cc_mqttsn_gateway
