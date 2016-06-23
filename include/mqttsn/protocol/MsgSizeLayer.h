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

namespace mqttsn
{

namespace protocol
{

using ShortLengthField =
    comms::field::IntValue<
        comms::Field<comms::option::BigEndian>,
        std::uint8_t,
        comms::option::NumValueSerOffset<sizeof(std::uint8_t)>
    >;

using LongLengthField =
    comms::field::Optional<
        comms::field::IntValue<
            comms::Field<comms::option::BigEndian>,
            std::uint16_t,
            comms::option::NumValueSerOffset<sizeof(std::uint8_t) + sizeof(std::uint16_t)>
        >
    >;

using LengthField =
    comms::field::Bundle<
        std::tuple<
            ShortLengthField,
            LongLengthField
        >
    >;

enum LengthFieldIdx
{
    LengthFieldIdx_Short,
    LengthFieldIdx_Long
};

template <typename TNextLayer>
class MsgSizeLayer : public comms::protocol::ProtocolLayerBase<LengthField, TNextLayer>
{
    typedef comms::protocol::ProtocolLayerBase<LengthField, TNextLayer> Base;
public:
    typedef typename Base::MsgPtr MsgPtr;
    typedef typename Base::Message Message;
    typedef typename Base::ReadIterator ReadIterator;
    typedef typename Base::WriteIterator WriteIterator;
    typedef typename Base::Field Field;

    template <typename TMsgPtr>
    comms::ErrorStatus read(
        TMsgPtr& msgPtr,
        ReadIterator& iter,
        std::size_t size,
        std::size_t* missingSize = nullptr)
    {
        Field field;
        return
            readInternal(
                field,
                msgPtr,
                iter,
                size,
                missingSize,
                Base::createNextLayerReader());
    }

    template <std::size_t TIdx, typename TAllFields, typename TMsgPtr>
    comms::ErrorStatus readFieldsCached(
        TAllFields& allFields,
        TMsgPtr& msgPtr,
        ReadIterator& iter,
        std::size_t size,
        std::size_t* missingSize = nullptr)
    {
        auto& field = Base::template getField<TIdx>(allFields);

        return
            readInternal(
                field,
                msgPtr,
                iter,
                size,
                missingSize,
                Base::template createNextLayerCachedFieldsReader<TIdx>(allFields));
    }

    comms::ErrorStatus write(
            const Message& msg,
            WriteIterator& iter,
            std::size_t size) const
    {
        Field field;
        return writeInternal(field, msg, iter, size, Base::createNextLayerWriter());
    }

    template <std::size_t TIdx, typename TAllFields>
    comms::ErrorStatus writeFieldsCached(
        TAllFields& allFields,
        const Message& msg,
        WriteIterator& iter,
        std::size_t size) const
    {
        auto& field = Base::template getField<TIdx>(allFields);
        return
            writeInternal(
                field,
                msg,
                iter,
                size,
                Base::template createNextLayerCachedFieldsWriter<TIdx>(allFields));
    }

private:

    template <typename TMsgPtr, typename TReader>
    comms::ErrorStatus readInternal(
        Field& field,
        TMsgPtr& msgPtr,
        ReadIterator& iter,
        std::size_t size,
        std::size_t* missingSize,
        TReader&& reader)
    {
        typedef typename std::iterator_traits<ReadIterator>::iterator_category IterTag;
        static_assert(
            std::is_base_of<std::random_access_iterator_tag, IterTag>::value,
            "Current implementation of MsgSizeLayer requires ReadIterator to be random-access one.");

        auto& members = field.value();
        auto& shortLengthField = std::get<LengthFieldIdx_Short>(members);
        auto& longLengthField = std::get<LengthFieldIdx_Long>(members);
        longLengthField.setMode(comms::field::OptionalMode::Missing);

        auto es = shortLengthField.read(iter, size);
        if (es == comms::ErrorStatus::NotEnoughData) {
            Base::updateMissingSize(field, size, missingSize);
        }

        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto fromIter = iter;
        auto actualRemainingSize = (size - shortLengthField.length());
        auto requiredRemainingSize = static_cast<std::size_t>(shortLengthField.value());

        do {
            if (0U < requiredRemainingSize) {
                break;
            }

            longLengthField.setMode(comms::field::OptionalMode::Exists);
            GASSERT(0U < longLengthField.length());

            es = longLengthField.read(iter, size);
            if (es == comms::ErrorStatus::NotEnoughData) {
                Base::updateMissingSize(field, size, missingSize);
            }

            if (es != comms::ErrorStatus::Success) {
                return es;
            }

            fromIter = iter;
            actualRemainingSize = (size - longLengthField.length());
            requiredRemainingSize = static_cast<std::size_t>(longLengthField.field().value());
        } while (false);

        if (actualRemainingSize < requiredRemainingSize) {
            if (missingSize != nullptr) {
                *missingSize = requiredRemainingSize - actualRemainingSize;
            }
            return comms::ErrorStatus::NotEnoughData;
        }

        // not passing missingSize farther on purpose
        es = reader.read(msgPtr, iter, requiredRemainingSize, nullptr);
        if (es == comms::ErrorStatus::NotEnoughData) {
            return comms::ErrorStatus::ProtocolError;
        }

        auto consumed =
            static_cast<std::size_t>(std::distance(fromIter, iter));
        if (consumed < requiredRemainingSize) {
            auto diff = requiredRemainingSize - consumed;
            std::advance(iter, diff);
        }
        return es;
    }

    template <typename TWriter>
    comms::ErrorStatus writeInternal(
        Field& field,
        const Message& msg,
        WriteIterator& iter,
        std::size_t size,
        TWriter&& nextLayerWriter) const
    {
        auto writeLength = Base::nextLayer().length(msg);
        auto& members = field.value();
        auto& shortLengthField = std::get<LengthFieldIdx_Short>(members);
        auto& longLengthField = std::get<LengthFieldIdx_Long>(members);

        do {
            if (writeLength <= (std::numeric_limits<std::uint8_t>::max() - 1)) {
                typedef typename std::decay<decltype(shortLengthField)>::type ShortField;
                typedef typename ShortField::ValueType ShortFieldValueType;
                shortLengthField.value() = static_cast<ShortFieldValueType>(writeLength);
                longLengthField.setMode(comms::field::OptionalMode::Missing);
                GASSERT(field.length() == shortLengthField.length());
                break;
            }

            typedef typename std::decay<decltype(shortLengthField)>::type LongField;
            typedef typename LongField::ValueType LongFieldValueType;
            longLengthField.setMode(comms::field::OptionalMode::Exists);
            shortLengthField.value() = 0;
            longLengthField.field().value() = static_cast<LongFieldValueType>(writeLength);
            GASSERT(field.length() == (shortLengthField.length() + longLengthField.length()));
        } while (false);

        auto es = field.write(iter, size);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        GASSERT(field.length() <= size);
        return nextLayerWriter.write(msg, iter, size - field.length());
    }
};

}  // namespace protocol

}  // namespace mqttsn


