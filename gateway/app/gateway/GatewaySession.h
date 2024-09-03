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
#include <list>
#include <vector>

namespace cc_mqttsn_gateway_app
{

class GatewaySession
{
public:
    using Ptr = std::unique_ptr<GatewaySession>;

    // Used by the GatewayApp
    explicit GatewaySession(
        boost::asio::io_context& io, 
        GatewayLogger& logger, 
        const cc_mqttsn_gateway::Config& config, 
        GatewayIoClientSocketPtr clientSocket);

    // Used when forwarding encapsulated session is reported
    explicit GatewaySession(
        boost::asio::io_context& io, 
        GatewayLogger& logger, 
        const cc_mqttsn_gateway::Config& config, 
        cc_mqttsn_gateway::Session* session);        

    ~GatewaySession();

    bool start();

    const std::string& clientId() const
    {
        return m_clientId;
    }

    using TermReqCb = std::function<void ()>;
    template <typename TFunc>
    void setTermReqCb(TFunc&& func)
    {
        m_termReqCb = std::forward<TFunc>(func);
    }

    using ClientIdReportCb = std::function<void (const std::string& clientId)>;
    template <typename TFunc>
    void setClientIdReportCb(TFunc&& func)
    {
        m_clientIdReportCb = std::forward<TFunc>(func);
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

    std::ostream& logError();
    std::ostream& logInfo();
    
    boost::asio::io_context& m_io;
    GatewayLogger& m_logger;
    const cc_mqttsn_gateway::Config& m_config;
    Timer m_timer;
    Timer m_reconnectTimer;
    GatewayIoClientSocketPtr m_clientSocket;
    GatewayIoBrokerSocketPtr m_brokerSocket;
    std::unique_ptr<cc_mqttsn_gateway::Session> m_sessionPtr;
    cc_mqttsn_gateway::Session* m_session = nullptr;
    TermReqCb m_termReqCb;
    ClientIdReportCb m_clientIdReportCb;
    std::string m_clientId;
    DataBuf m_brokerData;
    Timestamp m_tickReqTs;
    std::list<Ptr> m_fwdEncSessions;
    bool m_brokerConnected = false;
    bool m_destructing = false;
    bool m_hadBrokerData = false;
};

using GatewaySessionPtr = GatewaySession::Ptr;

} // namespace cc_mqttsn_gateway_app
