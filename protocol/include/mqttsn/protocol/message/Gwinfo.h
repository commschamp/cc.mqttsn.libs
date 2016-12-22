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
struct ExtraGwinfoOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraGwinfoOptions<true, false>
{
    typedef comms::option::NoDefaultFieldsWriteImpl Type;
};

template <>
struct ExtraGwinfoOptions<false, true>
{
    typedef comms::option::NoDefaultFieldsReadImpl Type;
};

template <typename TOpts>
using ExtraGwinfoOptionsT =
    typename ExtraGwinfoOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details

template <typename TFieldBase, typename TOptions = protocol::ParsedOptions<> >
using GwinfoFields =
    std::tuple<
        field::GwId<TFieldBase>,
        field::GwAdd<TFieldBase, TOptions>
    >;

template <typename TMsgBase, typename TOptions = ParsedOptions<> >
class Gwinfo : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_GWINFO>,
        comms::option::FieldsImpl<GwinfoFields<typename TMsgBase::Field, TOptions> >,
        comms::option::DispatchImpl<Gwinfo<TMsgBase, TOptions> >,
        details::ExtraGwinfoOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_GWINFO>,
        comms::option::FieldsImpl<GwinfoFields<typename TMsgBase::Field, TOptions> >,
        comms::option::DispatchImpl<Gwinfo<TMsgBase, TOptions> >,
        details::ExtraGwinfoOptionsT<TOptions>
    > Base;

public:
    enum FieldIdx
    {
        FieldIdx_gwId,
        FieldIdx_gwAdd,
        FieldIdx_numOfValues
    };

    static_assert(std::tuple_size<typename Base::AllFields>::value == FieldIdx_numOfValues,
        "Number of fields is incorrect");
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


