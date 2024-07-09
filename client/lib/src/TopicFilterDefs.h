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
using SubFiltersMap = ObjListType<TopicNameStr, Config::SubFiltersLimit, Config::HasSubTopicVerification>;

struct RegTopicInfo
{
    TopicNameStr m_topic;
    CC_MqttsnTopicId m_topicId = 0U; // key
};

using InRegTopicsMap = ObjListType<RegTopicInfo, Config::InRegTopicsLimit>;

} // namespace cc_mqttsn_client
