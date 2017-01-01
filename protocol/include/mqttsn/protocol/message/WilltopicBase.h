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
struct ExtraWilltopicBaseOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraWilltopicBaseOptions<false, true>
{
    typedef comms::option::NoDefaultFieldsWriteImpl Type;
};

template <typename TOpts>
using ExtraWilltopicBaseOptionsT =
    typename ExtraWilltopicBaseOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

template <bool TClientOnly, bool TGatewayOnly>
struct ExtraWilltopicOptions
{
    typedef comms::option::MsgDoRead Type;
};

template <>
struct ExtraWilltopicOptions<true, false>
{
    typedef comms::option::NoDefaultFieldsReadImpl Type;
};

template <>
struct ExtraWilltopicOptions<false, true>
{
    typedef std::tuple<
        comms::option::MsgDoRead,
        comms::option::NoDefaultFieldsWriteImpl
    > Type;
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
        comms::option::NoDefaultFieldsReadImpl,
        details::ExtraWilltopicBaseOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::FieldsImpl<WilltopicBaseFields<typename TMsgBase::Field, TOptions> >,
        comms::option::NoDefaultFieldsReadImpl,
        details::ExtraWilltopicBaseOptionsT<TOptions>
    > Base;

public:
    COMMS_MSG_FIELDS_ACCESS(Base, flags, willTopic);

    template <typename TIter>
    comms::ErrorStatus doRead(TIter& iter, std::size_t len)
    {
        auto& allFields = Base::fields();
        auto& flagsField = std::get<FieldIdx_flags>(allFields);
        auto mode = comms::field::OptionalMode::Missing;
        if (0U < len) {
            mode = comms::field::OptionalMode::Exists;
        }
        flagsField.setMode(mode);
        return Base::doRead(iter, len);
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
        comms::option::MsgType<TActual>,
        comms::option::DispatchImpl,
        comms::option::MsgDoRefresh,
        details::ExtraWilltopicOptionsT<TOptions>
    >
{
};


}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


