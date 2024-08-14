//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ProgramOptions.h"
#include "Session.h"

#include "client.h"

#include <boost/asio.hpp>

#include <functional>
#include <iosfwd>

namespace cc_mqttsn_client_app
{

class AppClient
{
    struct ClientDeleter
    {
        void operator()(CC_MqttsnClient* ptr)
        {
            ::cc_mqttsn_client_free(ptr);
        }
    };

public:
    bool start(int argc, const char* argv[]);    

    static std::string toString(CC_MqttsnErrorCode val);
    //static std::string toString(CC_MqttsnAsyncOpStatus val);
    //void print(const CC_MqttsnMessageInfo& info, bool printMessage = true);

protected:
    using Timer = boost::asio::steady_timer;
    using Addr = Session::Addr;

    explicit AppClient(boost::asio::io_context& io, int& result);
    ~AppClient() = default;

    CC_MqttsnClientHandle client()
    {
        return m_client.get();
    }

    boost::asio::io_context& io()
    {
        return m_io;
    }

    ProgramOptions& opts()
    {
        return m_opts;
    }

    static std::ostream& logError();
    static std::ostream& logInfo();

    void doTerminate(int result = 1);
    void doComplete();
    bool doConnect();

    virtual bool startImpl();
    virtual void messageReceivedImpl(const CC_MqttsnMessageInfo* info);
    virtual void connectCompleteImpl();

    static std::vector<std::uint8_t> parseBinaryData(const std::string& val);

    const Addr& lastAddr() const
    {
        return m_lastAddr;
    }

    static std::string toString(CC_MqttsnAsyncOpStatus val);
    static std::string toString(CC_MqttsnReturnCode val);

private:
    using ClientPtr = std::unique_ptr<CC_MqttsnClient, ClientDeleter>;
    using Clock = Timer::clock_type;
    using Timestamp = Timer::time_point;

    void nextTickProgramInternal(unsigned duration);
    unsigned cancelNextTickWaitInternal();
    void sendDataInternal(const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);
    bool createSession();
    void connectCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info);

    static void sendDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);
    static void messageReceivedCb(void* data, const CC_MqttsnMessageInfo* info);
    static void logMessageCb(void* data, const char* msg);
    static void nextTickProgramCb(void* data, unsigned duration);
    static unsigned cancelNextTickWaitCb(void* data);
    static void connectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info);

    boost::asio::io_context& m_io;
    int& m_result;
    Timer m_timer;
    Timestamp m_lastWaitProgram;
    ProgramOptions m_opts;
    ClientPtr m_client;
    SessionPtr m_session;
    Addr m_lastAddr;
};

} // namespace cc_mqttsn_client_app
