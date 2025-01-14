//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayWrapper.h"

#include <cassert>

namespace cc_mqttsn_gateway_app
{
    

GatewayWrapper::GatewayWrapper(boost::asio::io_context& io, GatewayLogger& logger) : 
    m_logger(logger),
    m_timer(io)
{
}

GatewayWrapper::~GatewayWrapper() = default;

bool GatewayWrapper::start(const cc_mqttsn_gateway::Config& config)
{
    assert(m_broadcastReqCb);
    if (!m_broadcastReqCb) {
        m_logger.error() << "Not all callbacks are set for the GatewayWrapper" << std::endl;
        return false;
    }

    m_gw.setGatewayId(config.gatewayId());
    m_gw.setAdvertisePeriod(config.advertisePeriod());
    m_gw.setNextTickProgramReqCb(
        [this](unsigned ms)
        {
            m_timer.expires_after(std::chrono::milliseconds(ms));
            m_timer.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec == boost::asio::error::operation_aborted) {
                        return;
                    }

                    if (ec) {
                        m_logger.error() << "Timer error: " << ec.message() << std::endl;
                        return;
                    }

                    m_gw.tick();
                });
        });

    m_gw.setSendDataReqCb(
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            m_broadcastReqCb(buf, bufSize);
        });
        

    if (!m_gw.start()) {
        m_logger.error() << "Failed to start Gateway" << std::endl;
        return false;
    }

    return true;
}

} // namespace cc_mqttsn_gateway_app
