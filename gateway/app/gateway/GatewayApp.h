//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayProgramOptions.h"

#include "cc_mqttsn_gateway/Config.h"

#include <boost/asio.hpp>

namespace cc_mqttsn_gateway_app
{

class GatewayApp
{
public:
    GatewayApp(boost::asio::io_context& io);
    ~GatewayApp();

    bool start(int argc, const char* argv[]);

private:
    boost::asio::io_context& m_io; 
    cc_mqttsn_gateway::Config m_config;
};

} // namespace cc_mqttsn_gateway_app
