//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayIoClientAcceptor.h"
#include "GatewayIoClientSocket_Udp.h"

#include <array>
#include <list>
#include <map>
#include <vector>

namespace cc_mqttsn_gateway_app
{

class GatewayIoClientAcceptor_Udp final : public GatewayIoClientAcceptor
{
    using Base = GatewayIoClientAcceptor;
public:
    GatewayIoClientAcceptor_Udp(boost::asio::io_context& io, GatewayLogger& logger, const cc_mqttsn_gateway::Config& config);
    virtual ~GatewayIoClientAcceptor_Udp();

    static Ptr create(boost::asio::io_context& io, GatewayLogger& logger, const cc_mqttsn_gateway::Config& config);

protected:
    virtual bool startImpl() override;
    virtual void broadcastDataImpl(const std::uint8_t* buf, std::size_t bufSize) override;

private:
    using Socket = boost::asio::ip::udp::socket;
    using Endpoint = Socket::endpoint_type;
    using ClientsMap = std::map<Endpoint, GatewayIoClientSocket_Udp*>;
    using DataBuf = std::vector<std::uint8_t>;
    struct WriteInfo
    {
        Endpoint m_endpoint;
        DataBuf m_data;
        unsigned m_ttl = 0U;
    };

    void doAccept();
    void sendData(const Endpoint& endpoint, const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius);
    void sendPendingWrites();

    Socket m_socket;
    Endpoint m_senderEndpoint;
    Endpoint m_broadcastEndpoint;
    DataBuf m_lastBroadcastData;
    std::uint16_t m_acceptPort = 0U;
    std::uint16_t m_broadcastPort = 0U;
    std::string m_broadcastAddress;
    unsigned m_defaultTtl = 128;
    unsigned m_broadcastTtl = 128;
    std::array<std::uint8_t, 2048> m_inBuf;
    ClientsMap m_clients;
    std::list<WriteInfo> m_pendingWrites;
};

} // namespace cc_mqttsn_gateway_app
