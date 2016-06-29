//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Unsublic License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Unsublic License for more details.
//
// You should have received a copy of the GNU General Unsublic License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "comms_champion/comms_champion.h"
#include "mqttsn/message/Unsuback.h"
#include "cc_plugin/protocol/Message.h"

namespace mqttsn
{

namespace cc_plugin
{

namespace protocol
{

namespace message
{

class Unsuback : public
    comms_champion::ProtocolMessageBase<
        mqttsn::message::Unsuback<mqttsn::cc_plugin::protocol::Message>,
        Unsuback>
{
protected:
    virtual const char* nameImpl() const override;
    virtual const QVariantList& fieldsPropertiesImpl() const override;
};

}  // namespace message

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn


