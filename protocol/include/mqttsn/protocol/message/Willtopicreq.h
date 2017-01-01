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
struct ExtraWilltopicreqOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraWilltopicreqOptions<true, false>
{
    typedef comms::option::NoDefaultFieldsWriteImpl Type;
};

template <>
struct ExtraWilltopicreqOptions<false, true>
{
    typedef comms::option::NoDefaultFieldsReadImpl Type;
};

template <typename TOpts>
using ExtraWilltopicreqOptionsT =
    typename ExtraWilltopicreqOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details

template <typename TFieldBase>
using WilltopicreqFields = std::tuple<>;

template <typename TMsgBase, typename TOptions = protocol::ParsedOptions<> >
class Willtopicreq : public
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_WILLTOPICREQ>,
        comms::option::FieldsImpl<WilltopicreqFields<typename TMsgBase::Field> >,
        comms::option::MsgType<Willtopicreq<TMsgBase, TOptions> >,
        comms::option::DispatchImpl,
        details::ExtraWilltopicreqOptionsT<TOptions>
    >
{
    typedef comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_WILLTOPICREQ>,
        comms::option::FieldsImpl<WilltopicreqFields<typename TMsgBase::Field> >,
        comms::option::MsgType<Willtopicreq<TMsgBase, TOptions> >,
        comms::option::DispatchImpl,
        details::ExtraWilltopicreqOptionsT<TOptions>
    > Base;

public:
    enum FieldIdx
    {
        FieldIdx_numOfValues
    };

    static_assert(std::tuple_size<typename Base::AllFields>::value == FieldIdx_numOfValues,
        "Number of fields is incorrect");
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


