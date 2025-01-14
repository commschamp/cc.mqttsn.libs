//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ExtConfig.h"
#include "ProtocolDefs.h"
#include "TopicFilterDefs.h"

#include "cc_mqttsn_client/common.h"

namespace cc_mqttsn_client
{

struct ReuseState
{
    SubFiltersMap m_subFilters;
    InRegTopicsMap m_inRegTopics;
    OutRegTopicsMap m_outRegTopics;
    std::uint16_t m_lastRecvMsgId = 0U;

#if CC_MQTTSN_CLIENT_HAS_WILL    
    using WillTopicType = WilltopicMsg::Field_willTopic::ValueType;
    using WillMsgType = WillmsgMsg::Field_willMsg::ValueType;

    struct WillInfo
    {
        WillTopicType m_topic;
        WillMsgType m_msg;
        CC_MqttsnQoS m_qos = CC_MqttsnQoS_AtMostOnceDelivery;
        bool m_retain = false;
    };

    WillInfo m_prevWill;

#endif // #if CC_MQTTSN_CLIENT_HAS_WILL       
};

} // namespace cc_mqttsn_client
