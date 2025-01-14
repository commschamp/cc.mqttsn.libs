//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayIoClientSocket.h"
#include "GatewayLogger.h"

#include "cc_mqttsn_gateway/Config.h"

#include <boost/asio.hpp>

#include <functional>
#include <memory>

namespace cc_mqttsn_gateway_app
{

class GatewayIoClientAcceptor
{
public:
    using Ptr = std::unique_ptr<GatewayIoClientAcceptor>;
    using NewConnectionReportCb = std::function<void (GatewayIoClientSocketPtr)>;

    virtual ~GatewayIoClientAcceptor();

    static Ptr create(boost::asio::io_context& io, GatewayLogger& logger, const cc_mqttsn_gateway::Config& config);

    bool start();

    template <typename TFunc>
    void setNewConnectionReportCb(TFunc&& func)
    {
        m_newConnectionReportCb = std::forward<TFunc>(func);
    }

    void broadcastData(const std::uint8_t* buf, std::size_t bufSize)
    {
        broadcastDataImpl(buf, bufSize);
    }

protected:
    GatewayIoClientAcceptor(boost::asio::io_context& io, GatewayLogger& logger) : 
        m_io(io),
        m_logger(logger)
    {
    };    

    virtual bool startImpl() = 0;
    virtual void broadcastDataImpl(const std::uint8_t* buf, std::size_t bufSize) = 0;

    boost::asio::io_context& io()
    {
        return m_io;
    }

    GatewayLogger& logger()
    {
        return m_logger;
    }

    void reportNewConnection(GatewayIoClientSocketPtr socket);

private:
    boost::asio::io_context& m_io; 
    GatewayLogger& m_logger;
    NewConnectionReportCb m_newConnectionReportCb;
};

using GatewayIoClientAcceptorPtr = GatewayIoClientAcceptor::Ptr;

} // namespace cc_mqttsn_gateway_app
