//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ExtConfig.h"
#include "ObjListType.h"
#include "ProtocolDefs.h"

#include "cc_mqttsn_client/common.h"

#include <cstdint>

namespace cc_mqttsn_client
{

struct ClientState
{
    using GwAddr = ObjListType<std::uint8_t, ExtConfig::GatewayAddrLen, ExtConfig::HasGatewayDiscovery>;
    using Timestamp = std::uint64_t;

    struct GwInfo
    {
        Timestamp m_expiryTimestamp = 0U;
        GwAddr m_addr;
        std::uint8_t m_gwId = 0;
    };

    using PacketIdsList = ObjListType<std::uint16_t, ExtConfig::PacketIdsLimit>;
    using GwInfosList = ObjListType<GwInfo, ExtConfig::GatewayInfoxMaxLimit, ExtConfig::HasGatewayDiscovery>;

    static constexpr unsigned DefaultKeepAlive = 60;

    GwInfosList m_gwInfos;
    PacketIdsList m_allocatedPacketIds;
    Timestamp m_timestamp = 0U;
    std::uint16_t m_lastPacketId = 0U;
    bool m_initialized = false;
    bool m_firstConnect = true;
    // bool m_networkDisconnected = false;
};

} // namespace cc_mqttsn_client
