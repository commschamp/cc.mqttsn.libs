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
struct ExtraSubUnsubOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraSubUnsubOptions<true, false>
{
    typedef comms::option::NoReadImpl Type;
};

template <>
struct ExtraSubUnsubOptions<false, true>
{
    typedef comms::option::NoWriteImpl Type;
};

template <typename TOpts>
using ExtraSubUnsubOptionsT =
    typename ExtraSubUnsubOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details



template <typename TFieldBase, typename TOptions>
using SubUnsubBaseFields =
    std::tuple<
        field::Flags<TFieldBase>,
        field::MsgId<TFieldBase>,
        comms::field::Optional<
            field::TopicId<TFieldBase>,
            comms::option::DefaultOptionalMode<comms::field::OptionalMode::Exists>
        >,
        comms::field::Optional<
            field::TopicName<TFieldBase, TOptions>,
            comms::option::DefaultOptionalMode<comms::field::OptionalMode::Missing>
        >
    >;

template <
    typename TMsgBase,
    MsgTypeId TId,
    typename TActual,
    typename TOptions = ParsedOptions<> >
class SubUnsubBase : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<TId>,
        comms::option::MsgType<TActual>,
        comms::option::FieldsImpl<SubUnsubBaseFields<typename TMsgBase::Field, TOptions> >,
        comms::option::HasDoRefresh,
        details::ExtraSubUnsubOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<TId>,
        comms::option::MsgType<TActual>,
        comms::option::FieldsImpl<SubUnsubBaseFields<typename TMsgBase::Field, TOptions> >,
        comms::option::HasDoRefresh,
        details::ExtraSubUnsubOptionsT<TOptions>
    > Base;

public:
    COMMS_MSG_FIELDS_ACCESS(flags, msgId, topicId, topicName);

    template <typename TIter>
    comms::ErrorStatus doRead(TIter& iter, std::size_t len)
    {
        auto es = Base::template readFieldsUntil<FieldIdx_msgId>(iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        if (field_flags().field_topicId().value() == field::TopicIdTypeVal::Name) {
            field_topicId().setMissing();
            field_topicName().setExists();
        }
        else {
            field_topicId().setExists();
            field_topicName().setMissing();
        }

        return Base::template readFieldsFrom<FieldIdx_msgId>(iter, len);
    }

    bool doRefresh()
    {
        auto expectedTopicIdMode = comms::field::OptionalMode::Exists;
        auto expectedTopicNameMode = comms::field::OptionalMode::Missing;
        if (field_flags().field_topicId().value() == field::TopicIdTypeVal::Name) {
            expectedTopicIdMode = comms::field::OptionalMode::Missing;
            expectedTopicNameMode = comms::field::OptionalMode::Exists;
        }

        bool refreshed = false;
        if (field_topicId().getMode() != expectedTopicIdMode) {
            field_topicId().setMode(expectedTopicIdMode);
            refreshed = true;
        }

        if (field_topicName().getMode() != expectedTopicNameMode) {
            field_topicName().setMode(expectedTopicNameMode);
            refreshed = true;
        }

        return refreshed;
    }

protected:
    ~SubUnsubBase() noexcept = default;
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


