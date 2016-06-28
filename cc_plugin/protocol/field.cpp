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

#include <cassert>

#include "field.h"

#include "cc_plugin/protocol/Message.h"

#include "mqttsn/field.h"

namespace cc = comms_champion;

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
    return cc::property::field::ForField<Field>().name("GwId").asMap();
}

QVariantMap createProps_duration()
{
    typedef mqttsn::field::Duration<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("Duration").asMap();
}

QVariantMap createProps_radius()
{
    typedef mqttsn::field::Radius<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("Radius").asMap();
}

QVariantMap createProps_gwAdd()
{
    typedef mqttsn::field::GwAdd<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("GwAdd").asMap();
}

QVariantMap createProps_flags()
{
    typedef mqttsn::field::TopicIdType<FieldBase> TopicIdTypeField;
    cc::property::field::ForField<TopicIdTypeField> topicIdProps;
    topicIdProps
        .name("TopicIdType")
        .serialisedHidden()
        .add("Normal")
        .add("Pre-defined")
        .add("Topic-name");
    assert(topicIdProps.values().size() == (int)mqttsn::field::TopicIdTypeVal::NumOfValues);

    typedef mqttsn::field::MidFlags<FieldBase> MidFlagsField;
    cc::property::field::ForField<MidFlagsField> midFlagsProps;
    midFlagsProps
        .serialisedHidden()
        .add("Clean Session")
        .add("Will")
        .add("Retain");
    assert(midFlagsProps.bits().size() == mqttsn::field::MidFlagsBits_numOfValues);

    typedef mqttsn::field::QoS<FieldBase> QoSField;
    cc::property::field::ForField<QoSField> qosProps;
    qosProps
        .name("QoS")
        .serialisedHidden()
        .add("At most once delivery")
        .add("At least once delivery")
        .add("Exactly once delivery");
    assert(qosProps.values().size() == (int)mqttsn::field::QosType::NumOfValues);

    typedef mqttsn::field::DupFlags<FieldBase> DupFlagsField;
    cc::property::field::ForField<DupFlagsField> dupFlagsProps;
    dupFlagsProps
        .serialisedHidden()
        .add("DUP");
    assert(dupFlagsProps.bits().size() == mqttsn::field::DupFlagsBits_numOfValues);

    typedef mqttsn::field::Flags<FieldBase> FlagsField;
    return
        cc::property::field::ForField<FlagsField>()
            .name("Flags")
            .add(topicIdProps.asMap())
            .add(midFlagsProps.asMap())
            .add(qosProps.asMap())
            .add(dupFlagsProps.asMap())
            .asMap();
}

QVariantMap createProps_protocolId()
{
    typedef mqttsn::field::ProtocolId<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("ProtocolId").asMap();
}

QVariantMap createProps_clientId()
{
    typedef mqttsn::field::ClientId<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("ClientId").asMap();
}

QVariantMap createProps_returnCode()
{
    typedef mqttsn::field::ReturnCode<FieldBase> Field;
    cc::property::field::ForField<Field> props;
    props.name("ReturnCode")
         .add("Accepted")
         .add("Conjestion")
         .add("Invalid Topic ID")
         .add("Not Supported");
    assert(props.values().size() == (int)mqttsn::field::ReturnCodeVal::NumOfValues);
    return props.asMap();
}


}  // namespace field

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn

