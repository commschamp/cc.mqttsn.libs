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
struct ExtraSubUnsubBaseOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraSubUnsubBaseOptions<false, true>
{
    typedef comms::option::NoWriteImpl Type;
};

template <typename TOpts>
using ExtraSubUnsubBaseOptionsT =
    typename ExtraSubUnsubBaseOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;


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
    typedef std::tuple<
        comms::option::NoWriteImpl
    >Type;
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

template <typename TMsgBase, typename TOptions>
class SubUnsubFieldsBase : public
    comms::MessageBase<
        TMsgBase,
        comms::option::FieldsImpl<SubUnsubBaseFields<typename TMsgBase::Field, TOptions> >,
        comms::option::NoReadImpl,
        details::ExtraSubUnsubBaseOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::FieldsImpl<SubUnsubBaseFields<typename TMsgBase::Field, TOptions> >,
        comms::option::NoReadImpl,
        details::ExtraSubUnsubBaseOptionsT<TOptions>
    > Base;

public:
    COMMS_MSG_FIELDS_ACCESS(Base, flags, msgId, topicId, topicName);

    template <typename TIter>
    comms::ErrorStatus doRead(TIter& iter, std::size_t len)
    {
        auto es = Base::template readFieldsUntil<FieldIdx_msgId>(iter, len);
        if (es != comms::ErrorStatus::Success) {
            return es;
        }

        auto& allFields = Base::fields();
        auto& flagsField = std::get<FieldIdx_flags>(allFields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<field::FlagsMemberIdx_topicId>(flagsMembers);

        auto& topicIdField = std::get<FieldIdx_topicId>(allFields);
        auto& topicNameField = std::get<FieldIdx_topicName>(allFields);
        if (topicIdTypeField.value() == field::TopicIdTypeVal::Name) {
            topicIdField.setMode(comms::field::OptionalMode::Missing);
            topicNameField.setMode(comms::field::OptionalMode::Exists);
        }
        else {
            topicIdField.setMode(comms::field::OptionalMode::Exists);
            topicNameField.setMode(comms::field::OptionalMode::Missing);
        }

        return Base::template readFieldsFrom<FieldIdx_msgId>(iter, len);
    }

    bool doRefresh()
    {
        auto& allFields = Base::fields();
        auto& flagsField = std::get<FieldIdx_flags>(allFields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<field::FlagsMemberIdx_topicId>(flagsMembers);

        auto expectedTopicIdMode = comms::field::OptionalMode::Exists;
        auto expectedTopicNameMode = comms::field::OptionalMode::Missing;
        if (topicIdTypeField.value() == field::TopicIdTypeVal::Name) {
            expectedTopicIdMode = comms::field::OptionalMode::Missing;
            expectedTopicNameMode = comms::field::OptionalMode::Exists;
        }

        bool refreshed = false;
        auto& topicIdField = std::get<FieldIdx_topicId>(allFields);
        if (topicIdField.getMode() != expectedTopicIdMode) {
            topicIdField.setMode(expectedTopicIdMode);
            refreshed = true;
        }

        auto& topicNameField = std::get<FieldIdx_topicName>(allFields);
        if (topicNameField.getMode() != expectedTopicNameMode) {
            topicNameField.setMode(expectedTopicNameMode);
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
class SubUnsubBase : public
    comms::MessageBase<
        SubUnsubFieldsBase<TMsgBase, TOptions>,
        comms::option::StaticNumIdImpl<TId>,
        comms::option::MsgType<TActual>,
        comms::option::NoValidImpl,
        comms::option::NoLengthImpl,
        comms::option::HasDoRefresh,
        comms::option::AssumeFieldsExistence,
        details::ExtraSubUnsubOptionsT<TOptions>
    >
{
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


