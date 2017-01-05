//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Unsublic License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Unsublic License for more details.
//
// You should have received a copy of the GNU General Unsublic License
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

namespace details
{

template <bool TClientOnly, bool TGatewayOnly>
struct ExtraUnsubackOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraUnsubackOptions<true, false>
{
    typedef comms::option::NoWriteImpl Type;
};

template <>
struct ExtraUnsubackOptions<false, true>
{
    typedef comms::option::NoReadImpl Type;
};

template <typename TOpts>
using ExtraUnsubackOptionsT =
    typename ExtraUnsubackOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details

template <typename TFieldBase>
using UnsubackFields =
    std::tuple<
        field::MsgId<TFieldBase>
    >;

template <typename TMsgBase, typename TOptions, template<class, class> class TActual>
using UnsubackBase =
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_UNSUBACK>,
        comms::option::FieldsImpl<UnsubackFields<typename TMsgBase::Field> >,
        comms::option::MsgType<TActual<TMsgBase, TOptions> >,
        details::ExtraUnsubackOptionsT<TOptions>
    >;

template <typename TMsgBase, typename TOptions = protocol::ParsedOptions<> >
class Unsuback : public UnsubackBase<TMsgBase, TOptions, Unsuback>
{
    typedef UnsubackBase<TMsgBase, TOptions, mqttsn::protocol::message::Unsuback> Base;

public:
    COMMS_MSG_FIELDS_ACCESS(Base, msgId);
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


