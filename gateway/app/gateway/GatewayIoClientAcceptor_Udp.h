//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayIoClientAcceptor.h"

namespace cc_mqttsn_gateway_app
{

class GatewayIoClientAcceptor_Udp : public GatewayIoClientAcceptor
{
    using Base = GatewayIoClientAcceptor;
public:
    GatewayIoClientAcceptor_Udp(boost::asio::io_context& io, const cc_mqttsn_gateway::Config& config);
    virtual ~GatewayIoClientAcceptor_Udp();

    static Ptr create(boost::asio::io_context& io, const cc_mqttsn_gateway::Config& config);

protected:
    virtual bool startImpl() override;

private:
};

} // namespace cc_mqttsn_gateway_app
