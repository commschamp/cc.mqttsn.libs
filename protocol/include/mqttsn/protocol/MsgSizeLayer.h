//
// Copyright 2016 - 2017 (C). Alex Robenko. All rights reserved.
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
        >,
        comms::option::DefaultOptionalMode<comms::field::OptionalMode::Missing>
    >;

struct LengthField : public
    comms::field::Bundle<
        comms::Field<comms::option::BigEndian>,
        std::tuple<
            ShortLengthField,
            LongLengthField
        >,
        comms::option::HasCustomRead,
        comms::option::HasCustomRefresh
    >
{
    COMMS_FIELD_MEMBERS_ACCESS_NOTEMPLATE(short, long);

    /// @brief Extra function to get current length value
    std::size_t getLengthValue() const
    {
        if (field_short().value() != 0U) {
            return static_cast<std::size_t>(field_short().value());
        }

        COMMS_ASSERT(field_long().doesExist());
        return static_cast<std::size_t>(field_long().field().value());
    }

    /// @brief Extra function to set current length value
    void setLengthValue(std::size_t val)
    {
        if (val <= 0xfe) {
            field_short().value() = static_cast<std::uint8_t>(val);
            field_long().setMissing();
            return;
        }

        field_short().value() = 0U;
        field_long().setExists();
        field_long().field().value() = static_cast<std::uint16_t>(val);
    }

    template <typename TIter>
    comms::ErrorStatus read(TIter& iter, std::size_t len)
    {
        using Base = typename std::decay<decltype(comms::field::toFieldBase(*this))>::type;
        auto es = Base::template readUntilAndUpdateLen<FieldIdx_long>(iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        refresh_long();

        es = Base::template readFrom<FieldIdx_long>(iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        return comms::ErrorStatus::Success;
    }

    bool refresh()
    {
        using Base = typename std::decay<decltype(comms::field::toFieldBase(*this))>::type;
        bool updated = Base::refresh();
        updated = refresh_long() || updated;
        return updated;
    }

private:
    bool refresh_long()
    {
        auto mode = comms::field::OptionalMode::Missing;
        if (field_short().value() == 0U) {
            mode = comms::field::OptionalMode::Exists;
        }

        if (field_long().getMode() == mode) {
            return false;
        }

        field_long().setMode(mode);
        return true;

    }
};

enum LengthFieldIdx
{
    LengthFieldIdx_Short,
    LengthFieldIdx_Long
};

template <typename TNextLayer>
class MsgSizeLayer : public
    comms::protocol::MsgSizeLayer<
        LengthField,
        TNextLayer,
        comms::option::ExtendingClass<MsgSizeLayer<TNextLayer> >
    >
{
    typedef
        comms::protocol::MsgSizeLayer<
            LengthField,
            TNextLayer,
            comms::option::ExtendingClass<MsgSizeLayer<TNextLayer> >
        > Base;
public:

    using Field = typename Base::Field;

    static std::size_t getRemainingSizeFromField(const Field& field)
    {
        return field.getLengthValue();
    }

    template <typename TMsg>
    static void prepareFieldForWrite(std::size_t size, const TMsg* msg, Field& field)
    {
        static_cast<void>(msg);
        field.setLengthValue(size);
    }
};

}  // namespace protocol

}  // namespace mqttsn


