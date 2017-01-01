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

namespace details
{

template <bool TClientOnly, bool TGatewayOnly>
struct ExtraSearchgwOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraSearchgwOptions<true, false>
{
    typedef comms::option::NoDefaultFieldsReadImpl Type;
};

template <>
struct ExtraSearchgwOptions<false, true>
{
    typedef comms::option::NoDefaultFieldsWriteImpl Type;
};

template <typename TOpts>
using ExtraSearchgwOptionsT =
    typename ExtraSearchgwOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details

template <typename TFieldBase>
using SearchgwFields =
    std::tuple<
        field::Radius<TFieldBase>
    >;

template <typename TMsgBase, typename TOptions = protocol::ParsedOptions<> >
class Searchgw : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_SEARCHGW>,
        comms::option::FieldsImpl<SearchgwFields<typename TMsgBase::Field> >,
        comms::option::MsgType<Searchgw<TMsgBase, TOptions> >,
        comms::option::DispatchImpl,
        details::ExtraSearchgwOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_SEARCHGW>,
        comms::option::FieldsImpl<SearchgwFields<typename TMsgBase::Field> >,
        comms::option::MsgType<Searchgw<TMsgBase, TOptions> >,
        comms::option::DispatchImpl,
        details::ExtraSearchgwOptionsT<TOptions>
    > Base;

public:
    COMMS_MSG_FIELDS_ACCESS(Base, radius);
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


