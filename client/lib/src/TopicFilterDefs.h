//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "cc_mqttsn_client/common.h"

#include "Config.h"
#include "ObjListType.h"
#include "ProtocolDefs.h"


namespace cc_mqttsn_client
{

using TopicNameStr = SubscribeMsg::Field_topicName::Field::ValueType;

struct RegTopicInfo
{
    TopicNameStr m_topic;
    CC_MqttsnTopicId m_topicId = 0U; 

    template <typename T>
    RegTopicInfo(T&& topic, CC_MqttsnTopicId topicId) : m_topic(std::forward<T>(topic)), m_topicId(topicId) {}

    RegTopicInfo(const char* topic) : m_topic(topic) {}
    RegTopicInfo(CC_MqttsnTopicId topicId) : m_topicId(topicId) {}
};

using SubFiltersMap = ObjListType<RegTopicInfo, Config::SubFiltersLimit, Config::HasSubTopicVerification>; // key is m_topic
using InRegTopicsMap = ObjListType<RegTopicInfo, Config::InRegTopicsLimit>; // key is m_topicId;
using OutRegTopicsMap = ObjListType<RegTopicInfo, Config::OutRegTopicsLimit>; // key is m_topic;

} // namespace cc_mqttsn_client
