//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayLogger.h"

#include "cc_mqttsn_gateway/Config.h"

#include <boost/asio.hpp>

#include <memory>

namespace cc_mqttsn_gateway_app
{

class GatewayIoClientSocket
{
public:
    virtual ~GatewayIoClientSocket();

    bool start();

    using DataReportCb = std::function<void (const std::uint8_t* buf, std::size_t bufSize)>;
    
    template <typename TFunc>
    void setDataReportCb(TFunc&& func)
    {
        m_dataReportCb = std::forward<TFunc>(func);
    }

    void sendData(const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius = 0)
    {
        sendDataImpl(buf, bufSize, broadcastRadius);
    }

protected:
    GatewayIoClientSocket(boost::asio::io_context& io, GatewayLogger& logger) : 
        m_io(io),
        m_logger(logger)
    {
    };    

    virtual bool startImpl() = 0;
    virtual void sendDataImpl(const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius) = 0;

    boost::asio::io_context& io()
    {
        return m_io;
    }

    GatewayLogger& logger()
    {
        return m_logger;
    }

    void reportClientData(const std::uint8_t* buf, std::size_t bufSize)
    {
        m_dataReportCb(buf, bufSize);
    }

private:
    boost::asio::io_context& m_io; 
    GatewayLogger& m_logger;
    DataReportCb m_dataReportCb;
};

using GatewayIoClientSocketPtr = std::unique_ptr<GatewayIoClientSocket>;

} // namespace cc_mqttsn_gateway_app
