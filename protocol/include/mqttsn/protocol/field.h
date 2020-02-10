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


template <bool THasClientIdStaticStorageSize>
struct ClientIdExtraOptsStaticStorage;

template <>
struct ClientIdExtraOptsStaticStorage<true>
{
    template <typename TOptions>
    using Type = comms::option::FixedSizeStorage<TOptions::ClientIdStaticStorageSize>;
};

template <>
struct ClientIdExtraOptsStaticStorage<false>
{
    template <typename TOptions>
    using Type = comms::option::EmptyOption;
};

template <bool THasOrigDataView>
struct ClientIdExtraOptsDataView;

template <>
struct ClientIdExtraOptsDataView<true>
{
    template <typename TOptions>
    using Type = comms::option::OrigDataView;
};

template <>
struct ClientIdExtraOptsDataView<false>
{
    template <typename TOptions>
    using Type =
        typename ClientIdExtraOptsStaticStorage<TOptions::HasClientIdStaticStorageSize>::
            template Type<TOptions>;
};

template <typename TOptions>
using ClientIdExtraOptsT =
    typename ClientIdExtraOptsDataView<TOptions::HasOrigDataView>:: template Type<TOptions>;

template <typename TFieldBase, typename... TExtraOpt>
using GwAdd =
    comms::field::ArrayList<
        TFieldBase,
        std::uint8_t,
        TExtraOpt...
    >;

template <bool THasGwAddStaticStorageSize>
struct GwAddExtraOptsStaticStorage;

template <>
struct GwAddExtraOptsStaticStorage<true>
{
    template <typename TOptions>
    using Type = comms::option::FixedSizeStorage<TOptions::GwAddStaticStorageSize>;
};

template <>
struct GwAddExtraOptsStaticStorage<false>
{
    template <typename TOptions>
    using Type = comms::option::EmptyOption;
};

template <bool THasOrigDataView>
struct GwAddExtraOptsDataView;

template <>
struct GwAddExtraOptsDataView<true>
{
    template <typename TOptions>
    using Type = comms::option::OrigDataView;
};

template <>
struct GwAddExtraOptsDataView<false>
{
    template <typename TOptions>
    using Type =
        typename GwAddExtraOptsStaticStorage<TOptions::HasGwAddStaticStorageSize>::
            template Type<TOptions>;
};

template <typename TOptions>
using GwAddExtraOptsT =
    typename GwAddExtraOptsDataView<TOptions::HasOrigDataView>::template Type<TOptions>;

template <typename TFieldBase, typename... TExtraOpt>
using TopicName = comms::field::String<TFieldBase, TExtraOpt...>;

template <bool THasTopicNameStaticStorageSize>
struct TopicNameExtraOptsStaticStorage;

template <>
struct TopicNameExtraOptsStaticStorage<true>
{
    template <typename TOptions>
    using Type = comms::option::FixedSizeStorage<TOptions::TopicNameStaticStorageSize>;
};

template <>
struct TopicNameExtraOptsStaticStorage<false>
{
    template <typename TOptions>
    using Type = comms::option::EmptyOption;
};

template <bool THasUseOrigDataView>
struct TopicNameExtraOptsDataView;

template <>
struct TopicNameExtraOptsDataView<true>
{
    template <typename TOptions>
    using Type = comms::option::OrigDataView;
};

template <>
struct TopicNameExtraOptsDataView<false>
{
    template <typename TOptions>
    using Type =
        typename TopicNameExtraOptsStaticStorage<TOptions::HasTopicNameStaticStorageSize>::
            template Type<TOptions>;
};

template <typename TOptions>
using TopicNameExtraOptsT =
    typename TopicNameExtraOptsDataView<TOptions::HasOrigDataView>::template Type<TOptions>;

template <typename TFieldBase, typename... TExtraOpt>
using WillTopic = comms::field::String<TFieldBase, TExtraOpt...>;

template <typename TFieldBase, typename... TExtraOpt>
using Data =
    comms::field::ArrayList<
        TFieldBase,
        std::uint8_t,
        TExtraOpt...
    >;

template <bool THasMessageDataStaticStorageSize>
struct DataExtraOptsStaticStorage;

template <>
struct DataExtraOptsStaticStorage<true>
{
    template <typename TOptions>
    using Type = comms::option::FixedSizeStorage<TOptions::MessageDataStaticStorageSize>;
};

template <>
struct DataExtraOptsStaticStorage<false>
{
    template <typename TOptions>
    using Type = comms::option::EmptyOption;
};

template <bool THasOrigDataView>
struct DataExtraOptsDataView;

template <>
struct DataExtraOptsDataView<true>
{
    template <typename TOptions>
    using Type = comms::option::OrigDataView;
};

template <>
struct DataExtraOptsDataView<false>
{
    template <typename TOptions>
    using Type =
        typename DataExtraOptsStaticStorage<TOptions::HasMessageDataStaticStorageSize>::
            template Type<TOptions>;
};


template <typename TOptions>
using DataExtraOptsT =
    typename DataExtraOptsDataView<TOptions::HasOrigDataView>::template Type<TOptions>;

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
    ShortName,
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


