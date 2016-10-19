//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "mqttsn/gateway/Session.h"
#include "SessionImpl.h"

namespace mqttsn
{

namespace gateway
{

Session::Session()
  : m_pImpl(new SessionImpl)
{
}

Session::~Session() = default;

void Session::setNextTickProgramReqCb(NextTickProgramReqCb&& func)
{
    m_pImpl->setNextTickProgramReqCb(std::move(func));
}

void Session::setCancelTickWaitReqCb(CancelTickWaitReqCb&& func)
{
    m_pImpl->setCancelTickWaitReqCb(std::move(func));
}

void Session::setSendDataClientReqCb(SendDataReqCb&& func)
{
    m_pImpl->setSendDataClientReqCb(std::move(func));
}

void Session::setSendDataBrokerReqCb(SendDataReqCb&& func)
{
    m_pImpl->setSendDataBrokerReqCb(std::move(func));
}

void Session::setTerminationReqCb(TerminationReqCb&& func)
{
    m_pImpl->setTerminationReqCb(std::move(func));
}

void Session::setBrokerReconnectReqCb(BrokerReconnectReqCb&& func)
{
    m_pImpl->setBrokerReconnectReqCb(std::move(func));
}

void Session::setGatewayId(std::uint8_t value)
{
    m_pImpl->setGatewayId(value);
}

void Session::setAuthInfo(
    const std::string& username,
    const std::uint8_t* password,
    std::size_t passLen)
{
    m_pImpl->setAuthInfo(username, password, passLen);
}

void Session::setAuthInfo(
    const char* username,
    const std::uint8_t* password,
    std::size_t passLen)
{
    m_pImpl->setAuthInfo(username, password, passLen);
}

void Session::setRetryPeriod(unsigned value)
{
    m_pImpl->setRetryPeriod(value);
}

void Session::setRetryCount(unsigned value)
{
    m_pImpl->setRetryCount(value);
}

void Session::setSleepingClientMsgLimit(std::size_t value)
{
    m_pImpl->setSleepingClientMsgLimit(value);
}

bool Session::start()
{
    return m_pImpl->start();
}

void Session::stop()
{
    m_pImpl->stop();
}

bool Session::isRunning() const
{
    return m_pImpl->isRunning();
}

void Session::tick(unsigned ms)
{
    m_pImpl->tick(ms);
}

std::size_t Session::dataFromClient(const std::uint8_t* buf, std::size_t len)
{
    return m_pImpl->dataFromClient(buf, len);
}

std::size_t Session::dataFromBroker(const std::uint8_t* buf, std::size_t len)
{
    return m_pImpl->dataFromBroker(buf, len);
}

void Session::setBrokerConnected(bool connected)
{
    m_pImpl->setBrokerConnected(connected);
}

bool Session::addPredefinedTopic(const std::string& topic, std::uint16_t topicId)
{
    return m_pImpl->addPredefinedTopic(topic, topicId);
}

bool Session::setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal)
{
    return m_pImpl->setTopicIdAllocationRange(minVal, maxVal);
}

}  // namespace gateway

}  // namespace mqttsn

