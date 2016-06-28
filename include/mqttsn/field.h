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
#include "mqttsn/MsgTypeId.h"

namespace mqttsn
{

namespace field
{

template <typename TFieldBase, typename... TExtraOpt>
using ClientId =
    comms::field::String<
        TFieldBase,
        TExtraOpt...
    >;

template <typename TFieldBase, typename... TExtraOpt>
using Data =
    comms::field::ArrayList<
        TFieldBase,
        std::uint8_t,
        TExtraOpt...
    >;

template <typename TFieldBase>
using Duration = comms::field::IntValue<TFieldBase, std::uint16_t>;

enum DupFlagsBits
{
    DupFlagsBits_dup,
    DupFlagsBits_numOfValues
};

template <typename TFieldBase>
using DupFlags =
    comms::field::BitmaskValue<
        TFieldBase,
        comms::option::FixedLength<1U>,
        comms::option::FixedBitLength<1> >;

enum MidFlagsBits
{
    MidFlagsBits_cleanSession,
    MidFlagsBits_will,
    MidFlagsBits_retain,
    MidFlagsBits_numOfValues
};

template <typename TFieldBase>
using MidFlags =
    comms::field::BitmaskValue<
        TFieldBase,
        comms::option::FixedLength<1>,
        comms::option::FixedBitLength<3> >;

enum class QosType : std::uint8_t
{
    AtMostOnceDelivery,
    AtLeastOnceDelivery,
    ExactlyOnceDelivery,
    NumOfValues
};

template <typename TFieldBase>
using QoS = comms::field::EnumValue<
        TFieldBase,
        QosType,
        comms::option::ValidNumValueRange<(int)QosType::AtMostOnceDelivery, (int)QosType::NumOfValues - 1>,
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

enum FlagsMemberIdx
{
    FlagsMemberIdx_topicId,
    FlagsMemberIdx_midFlags,
    FlagsMemberIdx_qos,
    FlagsMemberIdx_dupFlags,
    FlagsMemberIdx_numOfValues
};

template <typename TFieldBase>
using Flags =
    comms::field::Bitfield<
        TFieldBase,
        std::tuple<
            TopicIdType<TFieldBase>,
            MidFlags<TFieldBase>,
            QoS<TFieldBase>,
            DupFlags<TFieldBase>
        >
    >;

template <typename TFieldBase, typename... TExtraOpt>
using GwAdd = comms::field::String<TFieldBase, TExtraOpt...>;

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
    Accepted,
    Conjestion,
    InvalidTopicId,
    NotSupported,
    NumOfValues
};

template <typename TFieldBase>
using ReturnCode =
    comms::field::EnumValue<
        TFieldBase,
        ReturnCodeVal,
        comms::option::ValidNumValueRange<0, (int)ReturnCodeVal::NumOfValues - 1>
    >;

template <typename TFieldBase>
using TopicId =
    comms::field::IntValue<
        TFieldBase,
        std::uint16_t,
        comms::option::ValidNumValueRange<0, 0xfffe>
    >;

template <typename TFieldBase, typename... TExtraOpt>
using TopicName = comms::field::String<TFieldBase, TExtraOpt...>;

template <typename TFieldBase, typename... TExtraOpt>
using WillMsg = comms::field::ArrayList<TFieldBase, std::uint8_t, TExtraOpt...>;

template <typename TFieldBase, typename... TExtraOpt>
using WillTopic = comms::field::String<TFieldBase, TExtraOpt...>;

template <typename TFieldBase>
using MsgType = comms::field::EnumValue<TFieldBase, mqttsn::MsgTypeId>;

}  // namespace field

}  // namespace mqttsn


