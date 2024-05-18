//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayLogger.h"
#include "GatewayIoBrokerSocket.h"
#include "GatewayIoClientSocket.h"

#include "cc_mqttsn_gateway/Config.h"
#include "cc_mqttsn_gateway/Session.h"

#include <boost/asio.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <vector>

namespace cc_mqttsn_gateway_app
{

class GatewaySession
{
public:
    explicit GatewaySession(
        boost::asio::io_context& io, 
        GatewayLogger& logger, 
        const cc_mqttsn_gateway::Config& config, 
        GatewayIoClientSocketPtr clientSocket);

    bool start();

    using TermpReqCb = std::function<void ()>;
    template <typename TFunc>
    void setTermpReqCb(TFunc&& func)
    {
        m_termReqCb = std::forward<TFunc>(func);
    }

private:
    using Timer = boost::asio::steady_timer;
    using TimestampClock = std::chrono::steady_clock;
    using Timestamp = std::chrono::time_point<TimestampClock>;
    using DataBuf = std::vector<std::uint8_t>;
    using AuthInfo = cc_mqttsn_gateway::Session::AuthInfo;

    void doTerminate();
    void doBrokerConnect();
    void doBrokerReconnect();
    bool startSession();
    AuthInfo getAuthInfoFor(const std::string& clientId);
    
    boost::asio::io_context& m_io;
    GatewayLogger& m_logger;
    const cc_mqttsn_gateway::Config& m_config;
    Timer m_timer;
    GatewayIoClientSocketPtr m_clientSocket;
    GatewayIoBrokerSocketPtr m_brokerSocket;
    cc_mqttsn_gateway::Session m_session;
    TermpReqCb m_termReqCb;
    DataBuf m_brokerData;
    Timestamp m_tickReqTs;
    bool m_brokerConnected = false;
};

using GatewaySessionPtr = std::unique_ptr<GatewaySession>;

} // namespace cc_mqttsn_gateway_app
