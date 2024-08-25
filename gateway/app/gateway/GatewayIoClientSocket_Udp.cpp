//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayIoClientSocket_Udp.h"

namespace cc_mqttsn_gateway_app
{

GatewayIoClientSocket_Udp::GatewayIoClientSocket_Udp(boost::asio::io_context& io, GatewayLogger& loggerParam, const Endpoint& endpoint) : 
    Base(io, loggerParam),
    m_endpoint(endpoint)
{
    logger().info() << "New UDP client connection from: " << m_endpoint << std::endl;
};    

GatewayIoClientSocket_Udp::~GatewayIoClientSocket_Udp()
{
    logger().info() << "Terminated UDP client connection from: " << m_endpoint << std::endl;

    if (m_socketDeletedCb) {
        m_socketDeletedCb(m_endpoint);
    }
}

void GatewayIoClientSocket_Udp::newDataArrived(const std::uint8_t* buf, std::size_t bufSize)
{
    if (m_started && m_pendingData.empty()) {
        reportClientData(buf, bufSize);
        return;
    }

    m_pendingData.emplace_back(buf, buf + bufSize);
}

bool GatewayIoClientSocket_Udp::startImpl()
{
    if ((!m_sendDataCb) ||
        (!m_socketDeletedCb)) {
        logger().error() << "Not all callbacks are set for GatewayIoClientSocket_Udp" << std::endl;
        return false;
    }

    boost::asio::post(io(),
        [this]()
        {
            m_started = true;
            reportPendingData();
        });

    return true;
} 

void GatewayIoClientSocket_Udp::sendDataImpl(const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius)
{
    assert(m_sendDataCb);
    m_sendDataCb(m_endpoint, buf, bufSize, broadcastRadius);
}

void GatewayIoClientSocket_Udp::reportPendingData()
{
    while (!m_pendingData.empty()) {
        auto& dataBuf = m_pendingData.front();
        reportClientData(dataBuf.data(), dataBuf.size());
        m_pendingData.pop_front();
    }
}

} // namespace cc_mqttsn_gateway_app
