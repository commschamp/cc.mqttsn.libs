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
struct ExtraWillmsgupdOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraWillmsgupdOptions<true, false>
{
    typedef comms::option::NoReadImpl Type;
};

template <>
struct ExtraWillmsgupdOptions<false, true>
{
    typedef comms::option::NoWriteImpl Type;
};

template <typename TOpts>
using ExtraWillmsgupdOptionsT =
    typename ExtraWillmsgupdOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details

template <typename TFieldBase, typename TOptions>
using WillmsgupdFields =
    std::tuple<
        field::WillMsg<TFieldBase, TOptions>
    >;

template <typename TMsgBase, typename TOptions, template<class, class> class TActual>
using WillmsgupdBase =
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_WILLMSGUPD>,
        comms::option::FieldsImpl<WillmsgupdFields<typename TMsgBase::Field, TOptions> >,
        comms::option::MsgType<TActual<TMsgBase, TOptions> >,
        details::ExtraWillmsgupdOptionsT<TOptions>
    >;

template <typename TMsgBase, typename TOptions = ParsedOptions<> >
class Willmsgupd : public WillmsgupdBase<TMsgBase, TOptions, Willmsgupd>
{
    typedef WillmsgupdBase<TMsgBase, TOptions, mqttsn::protocol::message::Willmsgupd> Base;

public:
    COMMS_MSG_FIELDS_ACCESS(Base, willMsg);
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


