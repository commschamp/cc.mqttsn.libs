//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Config.h"

namespace cc_mqttsn_client
{

struct ConfigState
{
    static constexpr unsigned DefaultResponseTimeoutMs = 10000;
    static constexpr unsigned DefaultRetryCount = 3U;
    static constexpr unsigned DefaultBroadcastRadius = 3U;
    static constexpr unsigned DefaultGwAdvTimeoutMs = 15 * 60 * 1000; //
    static constexpr unsigned MaxBroadcastRadius = 255U;
    

    unsigned m_responseTimeoutMs = DefaultResponseTimeoutMs;
    unsigned m_retryCount = DefaultRetryCount;
    unsigned m_broadcastRadius = DefaultBroadcastRadius;
    unsigned m_gwAdvTimeoutMs = DefaultGwAdvTimeoutMs;
    // CC_MqttsnPublishOrdering m_publishOrdering = CC_MqttsnPublishOrdering_SameQos;
    // bool m_verifyOutgoingTopic = Config::HasTopicFormatVerification;
    // bool m_verifyIncomingTopic = Config::HasTopicFormatVerification;
    // bool m_verifySubFilter = Config::HasSubTopicVerification;
};

} // namespace cc_mqttsn_client
