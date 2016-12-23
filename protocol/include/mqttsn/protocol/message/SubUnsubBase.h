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
struct ExtraSubUnsubOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraSubUnsubOptions<false, true>
{
    typedef comms::option::NoDefaultFieldsWriteImpl Type;
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
        comms::option::NoDefaultFieldsReadImpl,
        details::ExtraSubUnsubOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::FieldsImpl<SubUnsubBaseFields<typename TMsgBase::Field, TOptions> >,
        comms::option::NoDefaultFieldsReadImpl,
        details::ExtraSubUnsubOptionsT<TOptions>
    > Base;

public:
    enum FieldIdx
    {
        FieldIdx_flags,
        FieldIdx_msgId,
        FieldIdx_topicId,
        FieldIdx_topicName,
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
        static_assert(Base::ImplOptions::HasNoDefaultFieldsReadImpl,
            "Expected to have read implementation in the base");

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
class SubUnsubBase : public
    comms::MessageBase<
        SubUnsubFieldsBase<TMsgBase, TOptions>,
        comms::option::StaticNumIdImpl<TId>,
        comms::option::DispatchImpl<TActual>
    >
{
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


