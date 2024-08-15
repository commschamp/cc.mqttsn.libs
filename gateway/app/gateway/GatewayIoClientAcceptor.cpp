//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayIoClientAcceptor.h"

#include "GatewayIoClientAcceptor_Udp.h"

#include <cassert>
#include <type_traits>

namespace cc_mqttsn_gateway_app
{

GatewayIoClientAcceptor::~GatewayIoClientAcceptor() = default;

GatewayIoClientAcceptor::Ptr GatewayIoClientAcceptor::create(
    boost::asio::io_context& io, 
    GatewayLogger& logger, 
    const cc_mqttsn_gateway::Config& config)    
{
    using CreateFunc = Ptr (*)(boost::asio::io_context&, GatewayLogger&, const cc_mqttsn_gateway::Config&);
    static const CreateFunc Map[] = {
        /* ClientConnectionType_Udp */ &GatewayIoClientAcceptor_Udp::create,
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == cc_mqttsn_gateway::Config::ClientConnectionType_ValuesLimit);

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

bool GatewayIoClientAcceptor::start()
{
    if (!m_newConnectionReportCb) {
        return false;
    }

    return startImpl();
}

void GatewayIoClientAcceptor::reportNewConnection(GatewayIoClientSocketPtr socket)
{
    assert(m_newConnectionReportCb);
    m_newConnectionReportCb(std::move(socket));
}



} // namespace cc_mqttsn_gateway_app
