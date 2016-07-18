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

#include "mqttsn/protocol/field.h"

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
    typedef mqttsn::protocol::field::GwId<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("GwId").asMap();
}

QVariantMap createProps_duration()
{
    typedef mqttsn::protocol::field::Duration<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("Duration").asMap();
}

QVariantMap createProps_durationOpt()
{
    return
        cc::property::field::Optional()
            .name("Duration")
            .field(createProps_duration())
            .asMap();
}

QVariantMap createProps_radius()
{
    typedef mqttsn::protocol::field::Radius<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("Radius").asMap();
}

QVariantMap createProps_gwAdd()
{
    typedef mqttsn::protocol::field::GwAdd<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("GwAdd").asMap();
}

QVariantMap createProps_flags()
{
    typedef mqttsn::protocol::field::TopicIdType<FieldBase> TopicIdTypeField;
    cc::property::field::ForField<TopicIdTypeField> topicIdProps;
    topicIdProps
        .name("TopicIdType")
        .serialisedHidden()
        .add("Normal")
        .add("Pre-defined")
        .add("Topic-name");
    assert(topicIdProps.values().size() == (int)mqttsn::protocol::field::TopicIdTypeVal::NumOfValues);

    typedef mqttsn::protocol::field::MidFlags<FieldBase> MidFlagsField;
    cc::property::field::ForField<MidFlagsField> midFlagsProps;
    midFlagsProps
        .serialisedHidden()
        .add("Clean Session")
        .add("Will")
        .add("Retain");
    assert(midFlagsProps.bits().size() == mqttsn::protocol::field::MidFlagsBits_numOfValues);

    typedef mqttsn::protocol::field::QoS<FieldBase> QoSField;
    cc::property::field::ForField<QoSField> qosProps;
    qosProps
        .name("QoS")
        .serialisedHidden()
        .add("No Gateway Publish", (int)mqttsn::protocol::field::QosType::NoGwPublish)
        .add("At most once delivery", (int)mqttsn::protocol::field::QosType::AtMostOnceDelivery)
        .add("At least once delivery", (int)mqttsn::protocol::field::QosType::AtLeastOnceDelivery)
        .add("Exactly once delivery", (int)mqttsn::protocol::field::QosType::ExactlyOnceDelivery);

    typedef mqttsn::protocol::field::DupFlags<FieldBase> DupFlagsField;
    cc::property::field::ForField<DupFlagsField> dupFlagsProps;
    dupFlagsProps
        .serialisedHidden()
        .add("DUP");
    assert(dupFlagsProps.bits().size() == mqttsn::protocol::field::DupFlagsBits_numOfValues);

    typedef mqttsn::protocol::field::Flags<FieldBase> FlagsField;
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
    typedef mqttsn::protocol::field::ProtocolId<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("ProtocolId").asMap();
}

QVariantMap createProps_clientId()
{
    typedef mqttsn::protocol::field::ClientId<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("ClientId").asMap();
}

QVariantMap createProps_returnCode()
{
    typedef mqttsn::protocol::field::ReturnCode<FieldBase> Field;
    cc::property::field::ForField<Field> props;
    props.name("ReturnCode")
         .add("Accepted")
         .add("Conjestion")
         .add("Invalid Topic ID")
         .add("Not Supported");
    assert(props.values().size() == (int)mqttsn::protocol::field::ReturnCodeVal_NumOfValues);
    return props.asMap();
}

QVariantMap createProps_willTopic()
{
    typedef mqttsn::protocol::field::WillTopic<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("WillTopic").asMap();
}

QVariantMap createProps_willMsg()
{
    typedef mqttsn::protocol::field::WillMsg<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("WillMsg").asMap();
}

QVariantMap createProps_topicId()
{
    typedef mqttsn::protocol::field::TopicId<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("TopicId").asMap();
}

QVariantMap createProps_topicIdOpt()
{
    return
        cc::property::field::Optional()
            .name("TopicId")
            .field(createProps_topicId())
            .uncheckable()
            .asMap();
}

QVariantMap createProps_msgId()
{
    typedef mqttsn::protocol::field::MsgId<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("MsgId").asMap();
}

QVariantMap createProps_topicName()
{
    typedef mqttsn::protocol::field::TopicName<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("TopicName").asMap();
}

QVariantMap createProps_topicNameOpt()
{
    return
        cc::property::field::Optional()
            .name("TopicName")
            .field(createProps_topicName())
            .uncheckable()
            .asMap();
}

QVariantMap createProps_data()
{
    typedef mqttsn::protocol::field::Data<FieldBase> Field;
    return cc::property::field::ForField<Field>().name("Data").asMap();
}

}  // namespace field

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn

