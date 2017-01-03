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
struct ExtraSubackOptions
{
    typedef std::tuple<> Type;
};

template <>
struct ExtraSubackOptions<true, false>
{
    typedef comms::option::NoWriteImpl Type;
};

template <>
struct ExtraSubackOptions<false, true>
{
    typedef comms::option::NoReadImpl Type;
};

template <typename TOpts>
using ExtraSubackOptionsT =
    typename ExtraSubackOptions<TOpts::ClientOnlyVariant, TOpts::GatewayOnlyVariant>::Type;

}  // namespace details

template <typename TFieldBase>
using SubackFields =
    std::tuple<
        field::Flags<TFieldBase>,
        field::TopicId<TFieldBase>,
        field::MsgId<TFieldBase>,
        field::ReturnCode<TFieldBase>
    >;

template <typename TMsgBase, typename TOptions, template<class, class> class TActual>
using SubackBase =
    comms::MessageBase<
        TMsgBase,
        comms::option::StaticNumIdImpl<MsgTypeId_SUBACK>,
        comms::option::FieldsImpl<SubackFields<typename TMsgBase::Field> >,
        comms::option::MsgType<TActual<TMsgBase, TOptions> >,
        details::ExtraSubackOptionsT<TOptions>
    >;

template <typename TMsgBase, typename TOptions = protocol::ParsedOptions<> >
class Suback : public SubackBase<TMsgBase, TOptions, Suback>
{
    typedef SubackBase<TMsgBase, TOptions, Suback> Base;

public:
    COMMS_MSG_FIELDS_ACCESS(Base, flags, topicId, msgId, returnCode);
};

}  // namespace message

}  // namespace protocol

}  // namespace mqttsn


