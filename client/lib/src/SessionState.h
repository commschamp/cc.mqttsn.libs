//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ProtocolDefs.h"

namespace cc_mqttsn_client
{

struct SessionState
{
    using ClientIdStr = ConnectMsg::Field_clientId::ValueType;
    static constexpr unsigned DefaultKeepAlive = 60;

    ClientIdStr m_clientId;
    unsigned m_keepAliveMs = 0U;
    CC_MqttsnConnectionStatus m_connectionStatus = CC_MqttsnConnectionStatus_Disconnected;
    bool m_disconnecting = false;
};

} // namespace cc_mqttsn_client
