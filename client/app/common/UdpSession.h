//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Session.h"

#include <array>
#include <cstdint>
#include <vector>

namespace cc_mqttsn_client_app
{

class UdpSession final : public Session
{
    using Base = Session;
public:
    static Ptr create(boost::asio::io_context& io, const ProgramOptions& opts);

protected:    
    virtual bool startImpl() override;
    virtual void sendDataImpl(const std::uint8_t* buf, std::size_t bufLen, unsigned broadcastRadius) override;

private:
    using Socket = boost::asio::ip::udp::socket;
    using Endpoint = Socket::endpoint_type;
    using InDataBuf = std::array<std::uint8_t, 4096>;
    using DataBuf = std::vector<std::uint8_t>;

    UdpSession(boost::asio::io_context& io, const ProgramOptions& opts) : 
        Base(io, opts),
        m_socket(io)
    {
    }

    void doRead();

    Socket m_socket;
    InDataBuf m_inBuf;
    // DataBuf m_buf;
    Endpoint m_senderEndpoint;
    Endpoint m_remoteEndpoint;
    Endpoint m_broadcastEndpoint;
};

} // namespace cc_mqttsn_client_app
