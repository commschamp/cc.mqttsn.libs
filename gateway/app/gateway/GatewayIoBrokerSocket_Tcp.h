//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayIoBrokerSocket.h"

#include <list>
#include <vector>

namespace cc_mqttsn_gateway_app
{

class GatewayIoBrokerSocket_Tcp final : public GatewayIoBrokerSocket
{
    using Base = GatewayIoBrokerSocket;
public:
    explicit GatewayIoBrokerSocket_Tcp(boost::asio::io_context& io, GatewayLogger& logger, const cc_mqttsn_gateway::Config& config);

    virtual ~GatewayIoBrokerSocket_Tcp();

protected:
    
    virtual bool startImpl() override;
    virtual void sendDataImpl(const std::uint8_t* buf, std::size_t bufSize) override;

private:
    using Socket = boost::asio::ip::tcp::socket;
    using Resolver = boost::asio::ip::tcp::resolver;
    using DataBuf = std::vector<std::uint8_t>;
    using DataBufsList = std::list<DataBuf>;

    void doRead();
    void sendPendingWrites();

    Socket m_socket;
    Resolver m_resolver;
    std::string m_addr;
    std::uint16_t m_port = 0;
    bool m_connected = false;
    DataBufsList m_sentData;
    std::array<std::uint8_t, 4096> m_inData;
};


} // namespace cc_mqttsn_gateway_app
