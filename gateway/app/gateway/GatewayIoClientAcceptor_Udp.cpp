//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayIoClientAcceptor_Udp.h"

namespace cc_mqttsn_gateway_app
{

namespace 
{

const std::string UdpListenPortKey("udp_listen_port");
const std::string UdpBroadcastPortKey("udp_broadcast_port");
const std::uint16_t DefaultListenPort = 1883;
const std::uint16_t DefaultBroadcastPort = 1883;

std::uint16_t getPortInfo(
    GatewayLogger& logger,
    const cc_mqttsn_gateway::Config& config,
    const std::string& key,
    std::uint16_t defaultValue)
{
    auto& map = config.configMap();
    auto iter = map.find(key);
    if ((iter == map.end()) ||
        (iter->second.empty())) {
        return defaultValue;
    }

    try {
        return static_cast<std::uint16_t>(std::stoul(iter->second));
    }
    catch (...) {
        logger.warning() << "Invalid value (" << iter->second << ") specified for key \"" << key << "\" in the configuration, assuming " << defaultValue << std::endl;
        // nothing to do
    }

    return defaultValue;
}

} // namespace 
    

GatewayIoClientAcceptor_Udp::GatewayIoClientAcceptor_Udp(
    boost::asio::io_context& io, 
    GatewayLogger& logger, 
    const cc_mqttsn_gateway::Config& config) : 
    Base(io, logger),
    m_socket(io),
    m_acceptPort(getPortInfo(logger, config, UdpListenPortKey, DefaultListenPort)),
    m_broadcastPort(getPortInfo(logger, config, UdpBroadcastPortKey, DefaultBroadcastPort))
{
    static_cast<void>(config);
}

GatewayIoClientAcceptor_Udp::~GatewayIoClientAcceptor_Udp() = default;

GatewayIoClientAcceptor_Udp::Ptr GatewayIoClientAcceptor_Udp::create(
    boost::asio::io_context& io, 
    GatewayLogger& logger,
    const cc_mqttsn_gateway::Config& config)
{
    return std::make_unique<GatewayIoClientAcceptor_Udp>(io, logger, config);
}

bool GatewayIoClientAcceptor_Udp::startImpl()
{
    boost::system::error_code ec;
    m_socket.open(boost::asio::ip::udp::v4(), ec);
    if (ec) {
        logger().error() << "Failed to open local UDP socket: " << ec.message() << std::endl;
        return false;
    }


    m_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), m_acceptPort));
    if (ec) {
        logger().error() << "Failed to bind local port " << m_acceptPort << ": " << ec.message() << std::endl;
        return false;
    }

    doAccept();
    return true;
}

void GatewayIoClientAcceptor_Udp::doAccept()
{
    m_socket.async_receive_from(
        boost::asio::buffer(m_inBuf), 
        m_senderEndpoint,
        [this](boost::system::error_code ec, std::size_t bytesCount)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            do {
                if (ec) {
                    logger().error() << "UDP read error: " << ec.message() << std::endl;
                    break;
                }

                auto iter = m_clients.find(m_senderEndpoint);
                if (iter != m_clients.end()) {
                    auto* socketPtr = iter->second;
                    assert(socketPtr != nullptr);
                    socketPtr->newDataArrived(m_inBuf.data(), bytesCount);
                    break;
                }

                auto socketPtr = std::make_unique<GatewayIoClientSocket_Udp>(io(), logger(), m_senderEndpoint);
                socketPtr->setSendDataCb(
                    std::bind(&GatewayIoClientAcceptor_Udp::sendData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

                socketPtr->setSocketDeletedCb(
                    [this](const Endpoint& endpoint)
                    {
                        auto delIter = m_clients.find(endpoint);
                        assert(delIter != m_clients.end());
                        m_clients.erase(delIter);
                    });

                socketPtr->newDataArrived(m_inBuf.data(), bytesCount);
                m_clients[m_senderEndpoint] = socketPtr.get();
                reportNewConnection(std::move(socketPtr));
            } while (false);

            doAccept();
        });    
}

void GatewayIoClientAcceptor_Udp::sendData(const Endpoint& endpoint, const std::uint8_t* buf, std::size_t bufSize)
{
    bool toSend = m_pendingWrites.empty();
    m_pendingWrites.push_back(WriteInfo());
    auto& info = m_pendingWrites.back();
    info.m_endpoint = endpoint;
    info.m_data.assign(buf, buf + bufSize);

    if (toSend) {
        sendPendingWrites();
    }
}

void GatewayIoClientAcceptor_Udp::sendPendingWrites()
{
    if (m_pendingWrites.empty()) {
        return;
    }

    auto& info = m_pendingWrites.front();
    m_socket.async_send_to(
        boost::asio::buffer(info.m_data), info.m_endpoint,
        [this, &info](boost::system::error_code ec, std::size_t bytesSent)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            do {
                if (ec) {
                    logger().error() << "Failed to send data to " << info.m_endpoint << ": " << ec.message() << std::endl;
                    break;
                }

                if (bytesSent < info.m_data.size()) {
                    logger().warning() << "Not all bytes are sent to " << info.m_endpoint << std::endl;
                    break;
                }

            } while (false);

            m_pendingWrites.pop_front();
            sendPendingWrites();
        });    
}

} // namespace cc_mqttsn_gateway_app
