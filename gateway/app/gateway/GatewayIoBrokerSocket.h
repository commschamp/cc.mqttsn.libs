//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
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

class GatewayIoBrokerSocket
{
public:
    using Ptr = std::unique_ptr<GatewayIoBrokerSocket>;

    virtual ~GatewayIoBrokerSocket();

    static Ptr create(boost::asio::io_context& io, GatewayLogger& logger, const cc_mqttsn_gateway::Config& config);

    bool start();

    using DataReportCb = std::function<void (const std::uint8_t* buf, std::size_t bufSize)>;
    
    template <typename TFunc>
    void setDataReportCb(TFunc&& func)
    {
        m_dataReportCb = std::forward<TFunc>(func);
    }

    using ConnectedReportCb = std::function<void ()>;

    template <typename TFunc>
    void setConnectedReportCb(TFunc&& func)
    {
        m_connectedReportCb = std::forward<TFunc>(func);
    }      

    using ErrorReportCb = std::function<void ()>;
    
    template <typename TFunc>
    void setErrorReportCb(TFunc&& func)
    {
        m_errorReportCb = std::forward<TFunc>(func);
    }    

    void sendData(const std::uint8_t* buf, std::size_t bufSize)
    {
        sendDataImpl(buf, bufSize);
    }

protected:
    GatewayIoBrokerSocket(boost::asio::io_context& io, GatewayLogger& logger) : 
        m_io(io),
        m_logger(logger)
    {
    };    

    virtual bool startImpl() = 0;
    virtual void sendDataImpl(const std::uint8_t* buf, std::size_t bufSize) = 0;

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

    void reportConnected()
    {
        m_connectedReportCb();
    }    

    void reportError()
    {
        m_errorReportCb();
    }

private:
    boost::asio::io_context& m_io; 
    GatewayLogger& m_logger;
    DataReportCb m_dataReportCb;
    ConnectedReportCb m_connectedReportCb;
    ErrorReportCb m_errorReportCb;
};

using GatewayIoBrokerSocketPtr = GatewayIoBrokerSocket::Ptr;

} // namespace cc_mqttsn_gateway_app
