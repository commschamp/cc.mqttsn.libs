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


#include "field.h"

#include "cc_plugin/protocol/Message.h"

#include "mqttsn/field.h"

namespace mqttsn
{

namespace cc_plugin
{

namespace protocol
{

namespace field
{

namespace
{

typedef cc_plugin::protocol::Message::Field FieldBase;

}  // namespace

QVariantMap createProps_gwId()
{
    typedef mqttsn::field::GwId<FieldBase> Field;
    return comms_champion::property::field::ForField<Field>().name("GwId").asMap();
}

QVariantMap createProps_duration()
{
    typedef mqttsn::field::Duration<FieldBase> Field;
    return comms_champion::property::field::ForField<Field>().name("Duration").asMap();
}

QVariantMap createProps_radius()
{
    typedef mqttsn::field::Radius<FieldBase> Field;
    return comms_champion::property::field::ForField<Field>().name("Radius").asMap();
}

QVariantMap createProps_gwAdd()
{
    typedef mqttsn::field::GwAdd<FieldBase> Field;
    return comms_champion::property::field::ForField<Field>().name("GwAdd").asMap();
}

}  // namespace field

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn

