//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "cc_mqttsn_gateway/Config.h"

#include <boost/asio.hpp>

#include <memory>

namespace cc_mqttsn_gateway_app
{

class GatewayIoClientSocket
{
public:
    virtual ~GatewayIoClientSocket();

    bool start()
    {
        return startImpl();
    }

protected:
    GatewayIoClientSocket(boost::asio::io_context& io) : 
        m_io(io)
    {
    };    

    virtual bool startImpl() = 0;

    boost::asio::io_context& io()
    {
        return m_io;
    }

private:
    boost::asio::io_context& m_io; 
};

using GatewayIoClientSocketPtr = std::unique_ptr<GatewayIoClientSocket>;

} // namespace cc_mqttsn_gateway_app
