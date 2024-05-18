//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayLogger.h"

#include "cc_mqttsn_gateway/Config.h"
#include "cc_mqttsn_gateway/Gateway.h"

#include <boost/asio.hpp>

#include <functional>

namespace cc_mqttsn_gateway_app
{

class GatewayWrapper
{
public:
    explicit GatewayWrapper(boost::asio::io_context& io, GatewayLogger& logger);
    ~GatewayWrapper();

    bool start(const cc_mqttsn_gateway::Config& config);

    using BroadcastReqCb = std::function<void (const std::uint8_t* buf, std::size_t bufSize)>;
    template <typename TFunc>
    void setBroadcastReqCb(TFunc&& func)
    {
        m_broadcastReqCb = std::forward<TFunc>(func);
    }

private:
    using Timer = boost::asio::steady_timer;
    
    GatewayLogger& m_logger;
    Timer m_timer;
    cc_mqttsn_gateway::Gateway m_gw;
    BroadcastReqCb m_broadcastReqCb;
};

} // namespace cc_mqttsn_gateway_app
