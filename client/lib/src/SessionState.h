//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace cc_mqttsn_client
{

struct SessionState
{
    static constexpr unsigned DefaultKeepAlive = 60;

    unsigned m_keepAliveMs = 0U;
    std::uint16_t m_lastRecvMsgId = 0U;
    bool m_connected = false;
    bool m_disconnecting = false;
};

} // namespace cc_mqttsn_client
