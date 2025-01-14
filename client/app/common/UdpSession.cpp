//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "UdpSession.h"

#include <cassert>

namespace cc_mqttsn_client_app
{

Session::Ptr UdpSession::create(boost::asio::io_context& io, const ProgramOptions& opts)
{
    return Ptr(new UdpSession(io, opts));
}

bool UdpSession::startImpl()
{
    boost::asio::ip::udp::resolver resolver(io());
    boost::system::error_code ec;
    auto remoteEndpoints = resolver.resolve(opts().networkAddress(), std::to_string(opts().networkRemotePort()), ec);
    if (ec) {
        logError() << "Failed to resolve remote address: " << ec.message() << std::endl;
        return false;
    }

    m_remoteEndpoint = *remoteEndpoints.begin();

    auto broadcastEndpoints = resolver.resolve(opts().networkBroadcastAddress(), std::to_string(opts().networkRemotePort()), ec);
    if (ec) {
        logError() << "Failed to resolve broadcast address: " << ec.message() << std::endl;
        return false;
    }    

    m_broadcastEndpoint = *broadcastEndpoints.begin();

    auto localPort = opts().networkLocalPort();
    if (localPort > 0U) {
        m_socket.open(boost::asio::ip::udp::v4(), ec);
        if (ec) {
            logError() << "Failed to open udp socket: " << ec.message() << std::endl;
            return false;
        }

        m_socket.set_option(boost::asio::socket_base::broadcast(true), ec);
        if (ec) {
            logError() << "Failed to enable broadcast for UDP socket: " << ec.message() << std::endl;
            return false;
        }

        
        m_socket.bind(Endpoint(boost::asio::ip::udp::v4(), localPort), ec);
        if (ec) {
            logError() << "Failed to bind local port " << localPort << ": " << ec.message() << std::endl;
            return false;
        }
    }
    else {
        m_socket.connect(m_remoteEndpoint, ec);
        if (ec) {
            logError() << "Failed to connect to remote address: " << ec.message() << std::endl;
            return false;
        }        
    }

    doRead();
    return true;
}

void UdpSession::sendDataImpl(const std::uint8_t* buf, std::size_t bufLen, unsigned broadcastRadius)
{
    boost::system::error_code ec;
    auto* endpoint = &m_remoteEndpoint;
    auto ttl = 128U;
    if (broadcastRadius > 0U) {
        endpoint = &m_broadcastEndpoint;
        ttl = broadcastRadius;
    }

    m_socket.set_option(boost::asio::ip::unicast::hops(ttl), ec);
    if (ec) {
        logError() << "Failed to update outgoing packet TTL: " << ec.message() << std::endl;
    }    

    auto written = m_socket.send_to(boost::asio::buffer(buf, bufLen), *endpoint, 0, ec);
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        logError() << "Failed to write data: " << ec.message() << std::endl;
        reportNetworkError();
        return;
    }    


    if (written != bufLen) {
        logError() << "Not all data has been written." << std::endl;
    }
}

void UdpSession::doRead()
{
    m_socket.async_receive_from(
        boost::asio::buffer(m_inBuf), 
        m_senderEndpoint,
        [this](boost::system::error_code ec, std::size_t bytesCount)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (ec) {
                logError() << "UDP read error: " << ec.message() << std::endl;
                reportNetworkError();
                return;
            }

            auto remoteAddr = m_senderEndpoint.address().to_v4().to_bytes();
            Addr addrToReport(remoteAddr.begin(), remoteAddr.end());
            auto origin = CC_MqttsnDataOrigin_Any;
            if (m_senderEndpoint == m_remoteEndpoint) {
                origin = CC_MqttsnDataOrigin_ConnectedGw;
            }

            reportData(m_inBuf.data(), bytesCount, addrToReport, origin);
            doRead();
        });    
}

} // namespace cc_mqttsn_client_app
