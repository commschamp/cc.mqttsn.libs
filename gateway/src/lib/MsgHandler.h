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

#include "comms/comms.h"
#include "mqttsn/protocol/AllMessages.h"
#include "mqtt/AllMessages.h"
#include "messages.h"

namespace mqttsn
{

namespace gateway
{

template<typename TMsgBase>
using MqttsnMsgHandler = comms::GenericHandler<
    TMsgBase,
    mqttsn::protocol::AllMessages<TMsgBase>
>;

template<typename TMsgBase>
using MqttMsgHandler = comms::GenericHandler<
    TMsgBase,
    mqtt::AllMessages<TMsgBase>
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


}  // namespace gateway

}  // namespace mqttsn


