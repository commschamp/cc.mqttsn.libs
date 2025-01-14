//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "cc_mqttsn_gateway/Session.h"
#include "SessionImpl.h"

namespace cc_mqttsn_gateway
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

void Session::setSendDataClientReqCb(ClientSendDataReqCb&& func)
{
    m_pImpl->setSendDataClientReqCb(std::move(func));
}

void Session::setSendDataBrokerReqCb(BrokerSendDataReqCb&& func)
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

void Session::setClientConnectedReportCb(ClientConnectedReportCb&& func)
{
    m_pImpl->setClientConnectedReportCb(std::move(func));
}

void Session::setAuthInfoReqCb(AuthInfoReqCb&& func)
{
    m_pImpl->setAuthInfoReqCb(std::move(func));
}

void Session::setErrorReportCb(ErrorReportCb&& func)
{
    m_pImpl->setErrorReportCb(std::move(func));
}

void Session::setFwdEncSessionCreatedReportCb(FwdEncSessionCreatedReportCb&& func)
{
    m_pImpl->setFwdEncSessionCreatedReportCb(std::move(func));
}

void Session::setFwdEncSessionDeletedReportCb(FwdEncSessionDeletedReportCb&& func)
{
    m_pImpl->setFwdEncSessionDeletedReportCb(std::move(func));
}

void Session::setGatewayId(std::uint8_t value)
{
    m_pImpl->setGatewayId(value);
}

std::uint8_t Session::getGatewayId() const
{
    return m_pImpl->getGatewayId();
}

void Session::setRetryPeriod(unsigned value)
{
    m_pImpl->setRetryPeriod(value);
}

unsigned Session::getRetryPeriod() const
{
    return m_pImpl->getRetryPeriod();
}

void Session::setRetryCount(unsigned value)
{
    m_pImpl->setRetryCount(value);
}

unsigned Session::getRetryCount() const
{
    return m_pImpl->getRetryCount();
}

void Session::setSleepingClientMsgLimit(std::size_t value)
{
    m_pImpl->setSleepingClientMsgLimit(value);
}

std::size_t Session::getSleepingClientMsgLimit() const
{
    return m_pImpl->getSleepingClientMsgLimit();
}

void Session::setDefaultClientId(const std::string& value)
{
    m_pImpl->setDefaultClientId(value);
}

const std::string& Session::getDefaultClientId() const
{
    return m_pImpl->getDefaultClientId();
}

void Session::setPubOnlyKeepAlive(std::uint16_t value)
{
    m_pImpl->setPubOnlyKeepAlive(value);
}

std::uint16_t Session::getPubOnlyKeepAlive() const
{
    return m_pImpl->getPubOnlyKeepAlive();
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

void Session::tick()
{
    m_pImpl->tick();
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

bool Session::getBrokerConnected() const
{
    return m_pImpl->getBrokerConnected();
}

bool Session::addPredefinedTopic(const std::string& topic, std::uint16_t topicId)
{
    return m_pImpl->addPredefinedTopic(topic, topicId);
}

bool Session::setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal)
{
    return m_pImpl->setTopicIdAllocationRange(minVal, maxVal);
}

}  // namespace cc_mqttsn_gateway

