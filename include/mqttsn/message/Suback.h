//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Sublic License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Sublic License for more details.
//
// You should have received a copy of the GNU General Sublic License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "comms/comms.h"
#include "mqttsn/MsgTypeId.h"
#include "mqttsn/field.h"

namespace mqttsn
{

namespace message
{

template <typename TFieldBase>
using SubackFields =
    std::tuple<
        field::Flags<TFieldBase>,
        field::TopicId<TFieldBase>,
        field::MsgId<TFieldBase>,
        field::ReturnCode<TFieldBase>
    >;

template <typename TMsgBase>
class Suback : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<mqttsn::MsgTypeId_SUBACK>,
        comms::option::FieldsImpl<SubackFields<typename TMsgBase::Field> >,
        comms::option::DispatchImpl<Suback<TMsgBase> >
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<mqttsn::MsgTypeId_SUBACK>,
        comms::option::FieldsImpl<SubackFields<typename TMsgBase::Field> >,
        comms::option::DispatchImpl<Suback<TMsgBase> >
    > Base;

public:
    enum FieldIdx
    {
        FieldIdx_flags,
        FieldIdx_topicId,
        FieldIdx_msgId,
        FieldIdx_returnCode,
        FieldIdx_numOfValues
    };

    static_assert(std::tuple_size<typename Base::AllFields>::value == FieldIdx_numOfValues,
        "Number of fields is incorrect");
};

}  // namespace message

}  // namespace mqttsn


