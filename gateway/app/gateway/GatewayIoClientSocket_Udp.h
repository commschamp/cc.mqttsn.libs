//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "GatewayIoClientSocket.h"

#include <functional>
#include <list>
#include <vector>

namespace cc_mqttsn_gateway_app
{

class GatewayIoClientSocket_Udp final : public GatewayIoClientSocket
{
    using Base = GatewayIoClientSocket;
public:
    using Endpoint = boost::asio::ip::udp::endpoint;

    explicit GatewayIoClientSocket_Udp(boost::asio::io_context& io, GatewayLogger& loggerParam, const Endpoint& endpoint);

    virtual ~GatewayIoClientSocket_Udp();

    void newDataArrived(const std::uint8_t* buf, std::size_t bufSize);

    using SendDataCb = std::function<void (const Endpoint& endpoint, const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius)>;
    template <typename TFunc>
    void setSendDataCb(TFunc&& func)
    {
        m_sendDataCb = std::forward<TFunc>(func);
    }

    using SocketDeletedCb = std::function<void (const Endpoint& endpoint)>;
    template <typename TFunc>
    void setSocketDeletedCb(TFunc&& func)
    {
        m_socketDeletedCb = std::forward<TFunc>(func);
    }

protected:
    
    virtual bool startImpl() override;
    virtual void sendDataImpl(const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius) override;

private:
    using DataBuf = std::vector<std::uint8_t>;
    using DataBufsList = std::list<DataBuf>;

    void reportPendingData();

    const Endpoint m_endpoint;
    bool m_started = false;
    DataBufsList m_pendingData;
    SendDataCb m_sendDataCb;
    SocketDeletedCb m_socketDeletedCb;
};


} // namespace cc_mqttsn_gateway_app
