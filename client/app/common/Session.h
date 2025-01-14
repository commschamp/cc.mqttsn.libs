//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ProgramOptions.h"

#include "cc_mqttsn_client/common.h"

#include <boost/asio.hpp>

#include <cstdint>
#include <iosfwd>
#include <memory>

namespace cc_mqttsn_client_app
{

class Session
{
public:
    using Ptr = std::unique_ptr<Session>;
    using Addr = std::vector<std::uint8_t>;

    virtual ~Session() = default;

    static Ptr create(boost::asio::io_context& io, const ProgramOptions& opts);

    bool start()
    {
        return startImpl();
    }

    void sendData(const std::uint8_t* buf, std::size_t bufLen, unsigned broadcastRadius)
    {
        sendDataImpl(buf, bufLen, broadcastRadius);
    }

    using DataReportCb = std::function<void (const std::uint8_t* buf, std::size_t bufLen, const Addr& addr, CC_MqttsnDataOrigin origin)>;
    template <typename TFunc>
    void setDataReportCb(TFunc&& func)
    {
        m_dataReportCb = std::forward<TFunc>(func);
    }

    using NetworkErrorReportCb = std::function<void ()>;
    template <typename TFunc>
    void setNetworkErrorReportCb(TFunc&& func)
    {
        m_networkErrorReportCb = std::forward<TFunc>(func);
    }


protected:
    Session(boost::asio::io_context& io, const ProgramOptions& opts);

    boost::asio::io_context& io()
    {
        return m_io;
    }

    const ProgramOptions& opts() const
    {
        return m_opts;
    }

    static std::ostream& logError();

    void reportData(const std::uint8_t* buf, std::size_t bufLen, const Addr& addr, CC_MqttsnDataOrigin origin);
    void reportNetworkError();

    virtual bool startImpl() = 0;
    virtual void sendDataImpl(const std::uint8_t* buf, std::size_t bufLen, unsigned broadcastRadius) = 0;

private:
    boost::asio::io_context& m_io; 
    const ProgramOptions& m_opts;
    DataReportCb m_dataReportCb;
    NetworkErrorReportCb m_networkErrorReportCb;
    bool m_networkError = false;
};

using SessionPtr = Session::Ptr;

} // namespace cc_mqttsn_client_app
