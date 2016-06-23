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

#include "mqttsn/MsgTypeId.h"
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
        cc::property::field::ForField<mqttsn::field::MsgType<FieldBase> >()
            .name("MsgType")
            .add("ADVERTISE", mqttsn::MsgTypeId_ADVERTISE)
            .add("SEARCHGW", mqttsn::MsgTypeId_SEARCHGW)
            .add("GWINFO", mqttsn::MsgTypeId_GWINFO)
            .add("CONNECT", mqttsn::MsgTypeId_CONNECT)
            .add("CONNACK", mqttsn::MsgTypeId_CONNACK)
            .add("WILLTOPICREQ", mqttsn::MsgTypeId_WILLTOPICREQ)
            .add("WILLTOPIC", mqttsn::MsgTypeId_WILLTOPIC)
            .add("WILLMSGREQ", mqttsn::MsgTypeId_WILLMSGREQ)
            .add("WILLMSG", mqttsn::MsgTypeId_WILLMSG)
            .add("REGISTER", mqttsn::MsgTypeId_REGISTER)
            .add("REGACK", mqttsn::MsgTypeId_REGACK)
            .add("PUBLISH", mqttsn::MsgTypeId_PUBLISH)
            .add("PUBACK", mqttsn::MsgTypeId_PUBACK)
            .add("PUBCOMP", mqttsn::MsgTypeId_PUBCOMP)
            .add("PUBREC", mqttsn::MsgTypeId_PUBREC)
            .add("PUBREL", mqttsn::MsgTypeId_PUBREL)
            .add("SUBSCRIBE", mqttsn::MsgTypeId_SUBSCRIBE)
            .add("SUBACK", mqttsn::MsgTypeId_SUBACK)
            .add("UNSUBSCRIBE", mqttsn::MsgTypeId_UNSUBSCRIBE)
            .add("UNSUBACK", mqttsn::MsgTypeId_UNSUBACK)
            .add("PINGREQ", mqttsn::MsgTypeId_PINGREQ)
            .add("PINGRESP", mqttsn::MsgTypeId_PINGRESP)
            .add("DISCONNECT", mqttsn::MsgTypeId_DISCONNECT)
            .add("WILLTOPICUPD", mqttsn::MsgTypeId_WILLTOPICUPD)
            .add("WILLTOPICRESP", mqttsn::MsgTypeId_WILLTOPICRESP)
            .add("WILLMSGUPD", mqttsn::MsgTypeId_WILLMSGUPD)
            .add("WILLMSGRESP", mqttsn::MsgTypeId_WILLMSGRESP)
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

TransportMessage::TransportMessage()
{
    auto& allFields = fields();
    auto& lengthField = std::get<FieldIdx_Length>(allFields);
    auto& lengthSubFields = lengthField.value();
    auto& lengthLongField = std::get<mqttsn::protocol::LengthFieldIdx_Long>(lengthSubFields);
    lengthLongField.setMode(comms::field::OptionalMode::Missing);
}

const QVariantList& TransportMessage::fieldsPropertiesImpl() const
{
    static const auto Props = createFieldsProperties();
    return Props;
}

comms::ErrorStatus TransportMessage::readImpl(ReadIterator& iter, std::size_t size)
{
    auto& allFields = fields();
    auto& lengthField = std::get<FieldIdx_Length>(allFields);
    auto& lengthSubFields = lengthField.value();
    auto& lengthShortField = std::get<mqttsn::protocol::LengthFieldIdx_Short>(lengthSubFields);
    auto& lengthLongField = std::get<mqttsn::protocol::LengthFieldIdx_Long>(lengthSubFields);

    auto es = lengthShortField.read(iter, size);
    if (es != comms::ErrorStatus::Success) {
        return es;
    }

    if (0 < lengthShortField.value()) {
        lengthLongField.setMode(comms::field::OptionalMode::Missing);
    }
    else {
        lengthLongField.setMode(comms::field::OptionalMode::Exists);
    }

    es = lengthLongField.read(iter, size - lengthShortField.length());
    if (es != comms::ErrorStatus::Success) {
        return es;
    }

    std::size_t remainingSize = size - lengthField.length();
    return readFieldsFrom<FieldIdx_Type>(iter, remainingSize);
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

