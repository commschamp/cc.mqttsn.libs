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

namespace mqttsn
{

namespace protocol
{

namespace message
{

template <typename TFieldBase>
using SubUnsubBaseFields =
    std::tuple<
        field::Flags<TFieldBase>,
        field::MsgId<TFieldBase>,
        comms::field::Optional<
            field::TopicId<TFieldBase>,
            comms::option::DefaultOptionalMode<comms::field::OptionalMode::Exists>
        >,
        comms::field::Optional<
            field::TopicName<TFieldBase>,
            comms::option::DefaultOptionalMode<comms::field::OptionalMode::Missing>
        >
    >;

template <typename TMsgBase, MsgTypeId TId, typename TActual>
class SubUnsubBase : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<TId>,
        comms::option::FieldsImpl<SubUnsubBaseFields<typename TMsgBase::Field> >,
        comms::option::DispatchImpl<TActual>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<TId>,
        comms::option::FieldsImpl<SubUnsubBaseFields<typename TMsgBase::Field> >,
        comms::option::DispatchImpl<TActual >
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

    bool refreshImpl() override
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

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


