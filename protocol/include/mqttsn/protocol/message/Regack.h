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
using RegackFields =
    std::tuple<
        field::TopicId<TFieldBase>,
        field::MsgId<TFieldBase>,
        field::ReturnCode<TFieldBase>
    >;

template <typename TMsgBase>
class Regack : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_REGACK>,
        comms::option::FieldsImpl<RegackFields<typename TMsgBase::Field> >,
        comms::option::MsgType<Regack<TMsgBase> >,
        comms::option::DispatchImpl
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_REGACK>,
        comms::option::FieldsImpl<RegackFields<typename TMsgBase::Field> >,
        comms::option::MsgType<Regack<TMsgBase> >,
        comms::option::DispatchImpl
    > Base;

public:
    COMMS_MSG_FIELDS_ACCESS(Base, topicId, msgId, returnCode);
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


