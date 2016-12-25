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
#include "mqttsn/protocol/MsgTypeId.h"
#include "mqttsn/protocol/field.h"
#include "mqttsn/protocol/ParsedOptions.h"

namespace mqttsn
{

namespace protocol
{

namespace message
{

namespace details
{

template <bool TClientOnly, bool TGatewayOnly>
struct ExtraWilltopicOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraWilltopicOptions<true, false>
{
    typedef comms::option::NoDefaultFieldsReadImpl Type;
};

template <>
struct ExtraWilltopicOptions<false, true>
{
    typedef comms::option::NoDefaultFieldsWriteImpl Type;
};

template <typename TOpts>
using ExtraWilltopicOptionsT =
    typename ExtraWilltopicOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details

template <typename TFieldBase, typename TOptions>
using WilltopicBaseFields =
    std::tuple<
        comms::field::Optional<
            field::Flags<TFieldBase>,
            comms::option::DefaultOptionalMode<comms::field::OptionalMode::Missing>
        >,
        field::WillTopic<TFieldBase, TOptions>
    >;

template <typename TMsgBase, typename TOptions = ParsedOptions<> >
class WilltopicFieldsBase : public
    comms::MessageBase<
        TMsgBase,
        comms::option::FieldsImpl<WilltopicBaseFields<typename TMsgBase::Field, TOptions> >,
        details::ExtraWilltopicOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::FieldsImpl<WilltopicBaseFields<typename TMsgBase::Field, TOptions> >,
        details::ExtraWilltopicOptionsT<TOptions>
    > Base;

public:
    enum FieldIdx
    {
        FieldIdx_flags,
        FieldIdx_willTopic,
        FieldIdx_numOfValues
    };

    static_assert(std::tuple_size<typename Base::AllFields>::value == FieldIdx_numOfValues,
        "Number of fields is incorrect");

    typedef typename Base::ReadIterator ReadIterator;

protected:
    comms::ErrorStatus readImpl(ReadIterator& iter, std::size_t len) override
    {
        return readInternal(iter, len, ReadTag());
    }

    bool refreshImpl() override
    {
        return refreshInternal(RefreshTag());
    }

private:
    struct NoReadTag {};
    struct HasReadTag {};
    struct NoRefreshTag {};
    struct HasRefreshTag {};

    typedef typename std::conditional<
        TOptions::ClientOnlyVariant && (!TOptions::GatewayOnlyVariant),
        NoReadTag,
        HasReadTag
    >::type ReadTag;

    typedef typename std::conditional<
        (!TOptions::ClientOnlyVariant) && TOptions::GatewayOnlyVariant,
        NoRefreshTag,
        HasRefreshTag
    >::type RefreshTag;

    comms::ErrorStatus doRead(ReadIterator& iter, std::size_t len)
    {
        static_assert(!Base::ImplOptions::HasNoDefaultFieldsReadImpl,
            "Expected to have read implementation in the base");

        auto& allFields = Base::fields();
        auto& flagsField = std::get<FieldIdx_flags>(allFields);
        auto mode = comms::field::OptionalMode::Missing;
        if (0U < len) {
            mode = comms::field::OptionalMode::Exists;
        }
        flagsField.setMode(mode);
        return Base::readImpl(iter, len);
    }

    comms::ErrorStatus readInternal(ReadIterator& iter, std::size_t len, NoReadTag)
    {
        static_assert(Base::ImplOptions::HasNoDefaultFieldsReadImpl,
            "Expected to have not supported read");
        return Base::readImpl(iter, len);
    }

    comms::ErrorStatus readInternal(ReadIterator& iter, std::size_t len, HasReadTag)
    {
        return doRead(iter, len);
    }

    bool doRefresh()
    {
        auto& allFields = Base::fields();
        auto& flagsField = std::get<FieldIdx_flags>(allFields);
        auto& willTopicField = std::get<FieldIdx_willTopic>(allFields);

        auto expectedFlagsMode = comms::field::OptionalMode::Exists;
        if (willTopicField.value().empty()) {
            expectedFlagsMode = comms::field::OptionalMode::Missing;
        }

        bool refreshed = false;
        if (flagsField.getMode() != expectedFlagsMode) {
            flagsField.setMode(expectedFlagsMode);
            refreshed = true;
        }

        return refreshed;
    }

    bool refreshInternal(NoRefreshTag)
    {
        return Base::refreshImpl();
    }

    bool refreshInternal(HasRefreshTag)
    {
        return doRefresh();
    }
};

template <
    typename TMsgBase,
    MsgTypeId TId,
    typename TActual,
    typename TOptions = ParsedOptions<> >
class WilltopicBase : public
    comms::MessageBase<
        WilltopicFieldsBase<TMsgBase, TOptions>,
        comms::option::StaticNumIdImpl<TId>,
        comms::option::DispatchImpl<TActual>
    >
{
};


}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


