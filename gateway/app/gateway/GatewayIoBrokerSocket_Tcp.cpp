//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayIoBrokerSocket_Tcp.h"

namespace cc_mqttsn_gateway_app
{

GatewayIoBrokerSocket_Tcp::GatewayIoBrokerSocket_Tcp(boost::asio::io_context& io, GatewayLogger& logger, const cc_mqttsn_gateway::Config& config) :
    Base(io, logger),
    m_socket(io),
    m_resolver(io),
    m_addr(config.brokerTcpHostAddress()),
    m_port(config.brokerTcpHostPort())
{
}

GatewayIoBrokerSocket_Tcp::~GatewayIoBrokerSocket_Tcp() = default;

bool GatewayIoBrokerSocket_Tcp::startImpl()
{
    m_resolver.async_resolve(
        m_addr, std::to_string(m_port),
        [this](const boost::system::error_code& ec, const auto& results)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (ec) {
                logger().error() << "TCP/IP resolution error for address " << m_addr << ": " << ec.message() << std::endl;
                reportError();
                return;
            }

            if (results.empty()) {
                logger().error() << "Failed to resolve address " << m_addr << std::endl;
                reportError();
                return;
            }

            auto endpoint = results.begin()->endpoint();
            m_socket.async_connect(
                endpoint,
                [this](const boost::system::error_code& connectEc)
                {
                    if (connectEc == boost::asio::error::operation_aborted) {
                        return;
                    }

                    if (connectEc) {
                        logger().error() << "Failed to estabilish TCP/IP connection to " << m_addr << ": " << connectEc.message() << std::endl;
                        reportError();
                        return;
                    }

                    m_connected = true;
                    doRead();
                    sendPendingWrites();
                }
            );

        });
    return true;
} 

void GatewayIoBrokerSocket_Tcp::sendDataImpl(const std::uint8_t* buf, std::size_t bufSize)
{
    bool sendRightAway = m_connected && m_sentData.empty();
    m_sentData.emplace_back(buf, buf + bufSize);
    if (sendRightAway) {
        sendPendingWrites();
    }
}

void GatewayIoBrokerSocket_Tcp::doRead()
{
    m_socket.async_read_some(
        boost::asio::buffer(m_inData),
        [this](const boost::system::error_code& ec, std::size_t bytesCount)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (ec) {
                logger().error() << "TCP/IP read error: " << ec.message() << std::endl;
                reportError();
                return;                
            }

            reportClientData(m_inData.data(), bytesCount);
            doRead();
        }
    );
}

void GatewayIoBrokerSocket_Tcp::sendPendingWrites()
{
    if (m_sentData.empty()) {
        return;
    }

    auto& buf = m_sentData.front();
    m_socket.async_write_some(
        boost::asio::buffer(buf),
        [this, &buf](const boost::system::error_code& ec, std::size_t bytesCount)
        {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            if (ec) {
                logger().error() << "TCP/IP write error: " << ec.message() << std::endl;
                reportError();
                return;                
            }

            do {
                if (buf.size() <= bytesCount) {
                    m_sentData.pop_front();
                    break;
                }

                buf.erase(buf.begin(), buf.begin() + bytesCount);
            } while (false);

            sendPendingWrites();
        });
}

} // namespace cc_mqttsn_gateway_app
