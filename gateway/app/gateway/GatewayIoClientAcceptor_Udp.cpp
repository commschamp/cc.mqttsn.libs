//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayIoClientAcceptor_Udp.h"

namespace cc_mqttsn_gateway_app
{

GatewayIoClientAcceptor_Udp::GatewayIoClientAcceptor_Udp(boost::asio::io_context& io, const cc_mqttsn_gateway::Config& config) : 
    Base(io)
{
    static_cast<void>(config);
}

GatewayIoClientAcceptor_Udp::~GatewayIoClientAcceptor_Udp() = default;

GatewayIoClientAcceptor_Udp::Ptr GatewayIoClientAcceptor_Udp::create(boost::asio::io_context& io, const cc_mqttsn_gateway::Config& config)
{
    return std::make_unique<GatewayIoClientAcceptor_Udp>(io, config);
}

bool GatewayIoClientAcceptor_Udp::startImpl()
{
    // TODO
    return false;
}

} // namespace cc_mqttsn_gateway_app
