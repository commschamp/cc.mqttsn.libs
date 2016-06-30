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

#include "TransportMessage.h"

#include <cassert>

#include <QtCore/QVariantMap>

#include "mqttsn/protocol/MsgTypeId.h"
#include "comms/Assert.h"

namespace cc = comms_champion;

namespace mqttsn
{

namespace cc_plugin
{

namespace protocol
{

namespace
{

enum FieldIdx
{
    FieldIdx_Length,
    FieldIdx_Type,
    FieldIdx_Data,
    FieldIdx_NumOfValues
};

typedef Message::Field FieldBase;

QVariantMap createMsgTypeProperties()
{
    return
        cc::property::field::ForField<mqttsn::protocol::field::MsgType<FieldBase> >()
            .name("MsgType")
            .add("ADVERTISE", mqttsn::protocol::MsgTypeId_ADVERTISE)
            .add("SEARCHGW", mqttsn::protocol::MsgTypeId_SEARCHGW)
            .add("GWINFO", mqttsn::protocol::MsgTypeId_GWINFO)
            .add("CONNECT", mqttsn::protocol::MsgTypeId_CONNECT)
            .add("CONNACK", mqttsn::protocol::MsgTypeId_CONNACK)
            .add("WILLTOPICREQ", mqttsn::protocol::MsgTypeId_WILLTOPICREQ)
            .add("WILLTOPIC", mqttsn::protocol::MsgTypeId_WILLTOPIC)
            .add("WILLMSGREQ", mqttsn::protocol::MsgTypeId_WILLMSGREQ)
            .add("WILLMSG", mqttsn::protocol::MsgTypeId_WILLMSG)
            .add("REGISTER", mqttsn::protocol::MsgTypeId_REGISTER)
            .add("REGACK", mqttsn::protocol::MsgTypeId_REGACK)
            .add("PUBLISH", mqttsn::protocol::MsgTypeId_PUBLISH)
            .add("PUBACK", mqttsn::protocol::MsgTypeId_PUBACK)
            .add("PUBCOMP", mqttsn::protocol::MsgTypeId_PUBCOMP)
            .add("PUBREC", mqttsn::protocol::MsgTypeId_PUBREC)
            .add("PUBREL", mqttsn::protocol::MsgTypeId_PUBREL)
            .add("SUBSCRIBE", mqttsn::protocol::MsgTypeId_SUBSCRIBE)
            .add("SUBACK", mqttsn::protocol::MsgTypeId_SUBACK)
            .add("UNSUBSCRIBE", mqttsn::protocol::MsgTypeId_UNSUBSCRIBE)
            .add("UNSUBACK", mqttsn::protocol::MsgTypeId_UNSUBACK)
            .add("PINGREQ", mqttsn::protocol::MsgTypeId_PINGREQ)
            .add("PINGRESP", mqttsn::protocol::MsgTypeId_PINGRESP)
            .add("DISCONNECT", mqttsn::protocol::MsgTypeId_DISCONNECT)
            .add("WILLTOPICUPD", mqttsn::protocol::MsgTypeId_WILLTOPICUPD)
            .add("WILLTOPICRESP", mqttsn::protocol::MsgTypeId_WILLTOPICRESP)
            .add("WILLMSGUPD", mqttsn::protocol::MsgTypeId_WILLMSGUPD)
            .add("WILLMSGRESP", mqttsn::protocol::MsgTypeId_WILLMSGRESP)
            .asMap();
}

QVariantMap createLengthProperties()
{
    return
        cc::property::field::ForField<mqttsn::protocol::LengthField>()
            .add(
                cc::property::field::ForField<mqttsn::protocol::ShortLengthField>()
                    .name("Length")
                    .displayOffset(1)
                    .asMap()
            )
            .add(
                cc::property::field::ForField<mqttsn::protocol::LongLengthField>()
                    .name("Length (ext)")
                    .field(
                        cc::property::field::IntValue()
                            .name("Length (ext)")
                            .displayOffset(3)
                            .asMap()
                    )
                    .uncheckable()
                    .asMap()
            )
            .serialisedHidden()
            .asMap();
}

QVariantMap createDataProperties()
{
    return cc::property::field::ArrayList().name("Data").asMap();
}

QVariantList createFieldsProperties()
{
    QVariantList props;
    props.append(createLengthProperties());
    props.append(createMsgTypeProperties());
    props.append(createDataProperties());

    assert(props.size() == FieldIdx_NumOfValues);
    return props;
}

}  // namespace

const QVariantList& TransportMessage::fieldsPropertiesImpl() const
{
    static const auto Props = createFieldsProperties();
    return Props;
}

bool TransportMessage::refreshImpl()
{
    auto& allFields = fields();
    auto& lengthField = std::get<FieldIdx_Length>(allFields);
    auto& lengthSubFields = lengthField.value();
    auto& lengthShortField = std::get<mqttsn::protocol::LengthFieldIdx_Short>(lengthSubFields);
    auto& lengthLongField = std::get<mqttsn::protocol::LengthFieldIdx_Long>(lengthSubFields);

    auto expectedLongLengthMode = comms::field::OptionalMode::Missing;
    if (lengthShortField.value() == 0) {
        expectedLongLengthMode = comms::field::OptionalMode::Exists;
    }

    if (lengthLongField.getMode() == expectedLongLengthMode) {
        return false;
    }

    lengthLongField.setMode(expectedLongLengthMode);
    return true;
}

}  // namespace protocol

}  // namespace cc_plugin

}  // namespace mqttsn

