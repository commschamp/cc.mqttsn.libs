//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayIoBrokerSocket.h"

#include "GatewayIoBrokerSocket_Tcp.h"

#include <type_traits>

namespace cc_mqttsn_gateway_app
{

GatewayIoBrokerSocket::~GatewayIoBrokerSocket() = default;

GatewayIoBrokerSocket::Ptr GatewayIoBrokerSocket::create(boost::asio::io_context& io, GatewayLogger& logger, const cc_mqttsn_gateway::Config& config)
{
    using CreateFunc = Ptr (*)(boost::asio::io_context&, GatewayLogger&, const cc_mqttsn_gateway::Config&);
    static const CreateFunc Map[] = {
        /* BrokerConnectionType_Tcp */ &GatewayIoBrokerSocket_Tcp::create,
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == cc_mqttsn_gateway::Config::BrokerConnectionType_ValuesLimit);

    auto idx = static_cast<unsigned>(config.clientConnectionType());
    if (MapSize <= idx) {
        logger.error() << "Unknown client connection type" << std::endl;
        return Ptr();
    }

    auto func = Map[idx];
    // if (func == nullptr) {
    //     return Ptr();
    // }

    return func(io, logger, config);    
}

bool GatewayIoBrokerSocket::start()
{
    if ((!m_dataReportCb) ||
        (!m_errorReportCb)) {
        m_logger.error() << "Not all callbacks are set for GatewayIoBrokerSocket" << std::endl;
        return false;
    }

    return startImpl();
}

} // namespace cc_mqttsn_gateway_app
