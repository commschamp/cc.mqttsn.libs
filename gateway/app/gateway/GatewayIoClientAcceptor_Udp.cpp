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
const std::string UdpBroadcastAddressKey("udp_broadcast_address");
const std::string UdpBroadcastRadiusKey("udp_broadcast_radius");
const unsigned DefaultListenPort = 1883;
const unsigned DefaultBroadcastPort = 1883;
const std::string DefaultBroadcastAddress("255.255.255.255");
const unsigned DefaultBroadcastRadius = 128;

unsigned long getUnsignedConfigValue(
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
        return std::stoul(iter->second);
    }
    catch (...) {
        logger.warning() << "Invalid value (" << iter->second << ") specified for key \"" << key << "\" in the configuration, assuming " << defaultValue << std::endl;
        // nothing to do
    }

    return defaultValue;
}

std::uint16_t getListenPort(GatewayLogger& logger, const cc_mqttsn_gateway::Config& config)
{
    return static_cast<std::uint16_t>(getUnsignedConfigValue(logger, config, UdpListenPortKey, DefaultListenPort));
}

std::uint16_t getBroadcastPort(GatewayLogger& logger, const cc_mqttsn_gateway::Config& config)
{
    return static_cast<std::uint16_t>(getUnsignedConfigValue(logger, config, UdpBroadcastPortKey, DefaultBroadcastPort));
}

const std::string& getBroadcastAddress(const cc_mqttsn_gateway::Config& config)
{
    auto& map = config.configMap();
    auto iter = map.find(UdpBroadcastAddressKey);
    if ((iter == map.end()) ||
        (iter->second.empty())) {
        return DefaultBroadcastAddress;
    }

    return iter->second;
}

unsigned getBroadcastTtl(GatewayLogger& logger, const cc_mqttsn_gateway::Config& config)
{
    return static_cast<unsigned>(getUnsignedConfigValue(logger, config, UdpBroadcastRadiusKey, DefaultBroadcastRadius));
}

} // namespace 
    

GatewayIoClientAcceptor_Udp::GatewayIoClientAcceptor_Udp(
    boost::asio::io_context& io, 
    GatewayLogger& logger, 
    const cc_mqttsn_gateway::Config& config) : 
    Base(io, logger),
    m_socket(io),
    m_acceptPort(getListenPort(logger, config)),
    m_broadcastPort(getBroadcastPort(logger, config)),
    m_broadcastAddress(getBroadcastAddress(config)),
    m_broadcastTtl(getBroadcastTtl(logger, config))
{
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
    auto asioBroadcastAddr = boost::asio::ip::make_address_v4(m_broadcastAddress, ec);
    if (ec) {
        logger().error() << "Invalid UDP broadcast address: " << ec.message() << std::endl;
        return false;
    }

    m_broadcastEndpoint.address(asioBroadcastAddr);
    m_broadcastEndpoint.port(m_broadcastPort);
    
    m_socket.open(boost::asio::ip::udp::v4(), ec);
    if (ec) {
        logger().error() << "Failed to open local UDP socket: " << ec.message() << std::endl;
        return false;
    }

    boost::asio::ip::unicast::hops defaultTtl;
    m_socket.get_option(defaultTtl, ec);
    if (ec) {
        logger().error() << "Failed to retrieve defaultTTL: " << ec.message() << ", assuming " << m_defaultTtl << std::endl;
    }    
    else {
        m_defaultTtl = defaultTtl.value();
    }

    m_socket.set_option(boost::asio::socket_base::broadcast(true), ec);
    if (ec) {
        logger().error() << "Failed to enable broadcast for UDP socket: " << ec.message() << std::endl;
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

void GatewayIoClientAcceptor_Udp::broadcastDataImpl(const std::uint8_t* buf, std::size_t bufSize)
{
    sendData(m_broadcastEndpoint, buf, bufSize, m_broadcastTtl);
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
                    std::bind(
                        &GatewayIoClientAcceptor_Udp::sendData, this, 
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

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

void GatewayIoClientAcceptor_Udp::sendData(const Endpoint& endpoint, const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius)
{
    bool toSend = m_pendingWrites.empty();
    m_pendingWrites.push_back(WriteInfo());
    auto& info = m_pendingWrites.back();
    info.m_data.assign(buf, buf + bufSize);

    if (broadcastRadius > 0U) {
        info.m_ttl = broadcastRadius;
        info.m_endpoint = m_broadcastEndpoint;
    }
    else {
        info.m_endpoint = endpoint;
        info.m_ttl = m_defaultTtl;
    }

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

    boost::system::error_code ecTmp;
    m_socket.set_option(boost::asio::ip::unicast::hops(info.m_ttl), ecTmp);
    if (ecTmp) {
        logger().error() << "Failed to update outgoing packet TTL: " << ecTmp.message() << std::endl;
    }

    // logger().info() << "!!! (udp) <-- " << info.m_endpoint << " -- " << info.m_data.size() << std::endl;
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
