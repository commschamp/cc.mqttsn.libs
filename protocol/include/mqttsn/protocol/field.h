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
#include "mqttsn/protocol/ParsedOptions.h"

namespace mqttsn
{

namespace protocol
{

namespace field
{

namespace details
{
template <typename TFieldBase, typename... TExtraOpt>
using ClientId =
    comms::field::String<
        TFieldBase,
        TExtraOpt...
    >;


template <typename TOptions, bool THasClientIdStaticStorageSize>
struct ClientIdExtraOpts;

template <typename TOptions>
struct ClientIdExtraOpts<TOptions, true>
{
    typedef comms::option::FixedSizeStorage<TOptions::ClientIdStaticStorageSize> Type;
};

template <typename TOptions>
struct ClientIdExtraOpts<TOptions, false>
{
    typedef comms::option::EmptyOption Type;
};

template <typename TOptions>
using ClientIdExtraOptsT =
    typename ClientIdExtraOpts<TOptions, TOptions::HasClientIdStaticStorageSize>::Type;


template <typename TFieldBase, typename... TExtraOpt>
using GwAdd =
    comms::field::ArrayList<
        TFieldBase,
        std::uint8_t,
        TExtraOpt...
    >;

template <typename TOptions, bool THasGwAddStaticStorageSize>
struct GwAddExtraOpts;

template <typename TOptions>
struct GwAddExtraOpts<TOptions, true>
{
    typedef comms::option::FixedSizeStorage<TOptions::GwAddStaticStorageSize> Type;
};

template <typename TOptions>
struct GwAddExtraOpts<TOptions, false>
{
    typedef comms::option::EmptyOption Type;
};

template <typename TOptions>
using GwAddExtraOptsT =
    typename GwAddExtraOpts<TOptions, TOptions::HasGwAddStaticStorageSize>::Type;

template <typename TFieldBase, typename... TExtraOpt>
using TopicName = comms::field::String<TFieldBase, TExtraOpt...>;

template <typename TOptions, bool THasTopicNameStaticStorageSize>
struct TopicNameExtraOpts;

template <typename TOptions>
struct TopicNameExtraOpts<TOptions, true>
{
    typedef comms::option::FixedSizeStorage<TOptions::TopicNameStaticStorageSize> Type;
};

template <typename TOptions>
struct TopicNameExtraOpts<TOptions, false>
{
    typedef comms::option::EmptyOption Type;
};

template <typename TOptions>
using TopicNameExtraOptsT =
    typename TopicNameExtraOpts<TOptions, TOptions::HasTopicNameStaticStorageSize>::Type;

template <typename TFieldBase, typename... TExtraOpt>
using WillTopic = comms::field::String<TFieldBase, TExtraOpt...>;

template <typename TFieldBase, typename... TExtraOpt>
using Data =
    comms::field::ArrayList<
        TFieldBase,
        std::uint8_t,
        TExtraOpt...
    >;

template <typename TOptions, bool THasMessageDataStaticStorageSize>
struct DataExtraOpts;

template <typename TOptions>
struct DataExtraOpts<TOptions, true>
{
    typedef comms::option::FixedSizeStorage<TOptions::MessageDataStaticStorageSize> Type;
};

template <typename TOptions>
struct DataExtraOpts<TOptions, false>
{
    typedef comms::option::EmptyOption Type;
};

template <typename TOptions>
using DataExtraOptsT =
    typename DataExtraOpts<TOptions, TOptions::HasMessageDataStaticStorageSize>::Type;

template <typename TFieldBase, typename... TExtraOpt>
using WillMsg =
    comms::field::ArrayList<TFieldBase, std::uint8_t, TExtraOpt...>;

}  // namespace details

template <typename TFieldBase, typename TOptions = ParsedOptions<> >
using ClientId =
    details::ClientId<TFieldBase, details::ClientIdExtraOptsT<TOptions> >;

template <typename TFieldBase, typename TOptions = ParsedOptions<> >
using Data =
    details::Data<TFieldBase, details::DataExtraOptsT<TOptions> >;

template <typename TFieldBase>
using Duration = comms::field::IntValue<TFieldBase, std::uint16_t>;

template <typename TFieldBase>
struct DupFlags : public
    comms::field::BitmaskValue<
        TFieldBase,
        comms::option::FixedLength<1U>,
        comms::option::FixedBitLength<1> >
{
    COMMS_BITMASK_BITS(bit);
};

template <typename TFieldBase>
struct MidFlags : public
    comms::field::BitmaskValue<
        TFieldBase,
        comms::option::FixedLength<1>,
        comms::option::FixedBitLength<3> >
{
    COMMS_BITMASK_BITS(cleanSession, will, retain);
};

enum class QosType : std::uint8_t
{
    AtMostOnceDelivery = 0,
    AtLeastOnceDelivery,
    ExactlyOnceDelivery,
    NoGwPublish,
};

template <typename TFieldBase>
using QoS = comms::field::EnumValue<
        TFieldBase,
        QosType,
        comms::option::ValidNumValueRange<(unsigned)QosType::AtMostOnceDelivery, (unsigned)QosType::NoGwPublish>,
        comms::option::FixedBitLength<2>
    >;

enum class TopicIdTypeVal : std::uint8_t
{
    Normal,
    PreDefined,
    Name,
    NumOfValues
};

template <typename TFieldBase>
using TopicIdType = comms::field::EnumValue<
        TFieldBase,
        TopicIdTypeVal,
        comms::option::ValidNumValueRange<(int)TopicIdTypeVal::Normal, (int)TopicIdTypeVal::NumOfValues - 1>,
        comms::option::FixedBitLength<2>
    >;

template <typename TFieldBase>
struct Flags : public
    comms::field::Bitfield<
        TFieldBase,
        std::tuple<
            TopicIdType<TFieldBase>,
            MidFlags<TFieldBase>,
            QoS<TFieldBase>,
            DupFlags<TFieldBase>
        >
    >
{
    COMMS_FIELD_MEMBERS_ACCESS(topicId, midFlags, qos, dupFlags)
};

template <typename TFieldBase, typename TOptions = ParsedOptions<> >
using GwAdd = details::GwAdd<TFieldBase, details::GwAddExtraOptsT<TOptions> >;

template <typename TFieldBase>
using GwId = comms::field::IntValue<TFieldBase, std::uint8_t>;

template <typename TFieldBase>
using MsgId = comms::field::IntValue<TFieldBase, std::uint16_t>;

template <typename TFieldBase>
using ProtocolId =
    comms::field::IntValue<
        TFieldBase,
        std::uint8_t,
        comms::option::DefaultNumValue<1>,
        comms::option::ValidNumValueRange<1, 1>>;

template <typename TFieldBase>
using Radius = comms::field::IntValue<TFieldBase, std::uint8_t>;

enum ReturnCodeVal : std::uint8_t
{
    ReturnCodeVal_Accepted,
    ReturnCodeVal_Congestion,
    ReturnCodeVal_InvalidTopicId,
    ReturnCodeVal_NotSupported,
    ReturnCodeVal_NumOfValues
};

template <typename TFieldBase>
using ReturnCode =
    comms::field::EnumValue<
        TFieldBase,
        ReturnCodeVal,
        comms::option::ValidNumValueRange<0, (int)ReturnCodeVal_NumOfValues - 1>
    >;

template <typename TFieldBase>
using TopicId =
    comms::field::IntValue<
        TFieldBase,
        std::uint16_t,
        comms::option::ValidNumValueRange<0, 0xfffe>
    >;

template <typename TFieldBase, typename TOptions = ParsedOptions<> >
using TopicName =
    details::TopicName<TFieldBase, details::TopicNameExtraOptsT<TOptions> >;

template <typename TFieldBase, typename TOptions = ParsedOptions<> >
using WillMsg =
    details::WillMsg<TFieldBase, details::DataExtraOptsT<TOptions> >;

template <typename TFieldBase, typename TOptions = ParsedOptions<> >
using WillTopic =
    details::WillTopic<TFieldBase, details::TopicNameExtraOptsT<TOptions> >;

template <typename TFieldBase>
using MsgType = comms::field::EnumValue<TFieldBase, MsgTypeId>;

}  // namespace field

}  // namespace protocol

}  // namespace mqttsn


