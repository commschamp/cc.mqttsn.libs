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
struct ExtraWillmsgrespOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraWillmsgrespOptions<true, false>
{
    typedef comms::option::NoDefaultFieldsWriteImpl Type;
};

template <>
struct ExtraWillmsgrespOptions<false, true>
{
    typedef comms::option::NoDefaultFieldsReadImpl Type;
};

template <typename TOpts>
using ExtraWillmsgrespOptionsT =
    typename ExtraWillmsgrespOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details


template <typename TFieldBase>
using WillmsgrespFields =
    std::tuple<
        field::ReturnCode<TFieldBase>
    >;

template <typename TMsgBase, typename TOptions = protocol::ParsedOptions<> >
class Willmsgresp : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_WILLMSGRESP>,
        comms::option::FieldsImpl<WillmsgrespFields<typename TMsgBase::Field> >,
        comms::option::DispatchImpl<Willmsgresp<TMsgBase, TOptions> >,
        details::ExtraWillmsgrespOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_WILLMSGRESP>,
        comms::option::FieldsImpl<WillmsgrespFields<typename TMsgBase::Field> >,
        comms::option::DispatchImpl<Willmsgresp<TMsgBase, TOptions> >,
        details::ExtraWillmsgrespOptionsT<TOptions>
    > Base;

public:
    enum FieldIdx
    {
        FieldIdx_returnCode,
        FieldIdx_numOfValues
    };

    static_assert(std::tuple_size<typename Base::AllFields>::value == FieldIdx_numOfValues,
        "Number of fields is incorrect");
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


