//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "cc_mqttsn/gateway/Config.h"

#include "ConfigImpl.h"

namespace cc_mqttsn
{

namespace gateway
{

Config::Config()
  : m_pImpl(new ConfigImpl)
{
}

Config::~Config() = default;

void Config::read(std::istream& stream)
{
    m_pImpl->read(stream);
}

const Config::ConfigMap& Config::configMap() const
{
    return m_pImpl->configMap();
}

std::uint8_t Config::gatewayId() const
{
    return m_pImpl->gatewayId();
}

std::uint16_t Config::advertisePeriod() const
{
    return m_pImpl->advertisePeriod();
}

unsigned Config::retryPeriod() const
{
    return m_pImpl->retryPeriod();
}

unsigned Config::retryCount() const
{
    return m_pImpl->retryCount();
}

const std::string& Config::defaultClientId() const
{
    return m_pImpl->defaultClientId();
}

std::uint16_t Config::pubOnlyKeepAlive() const
{
    return m_pImpl->pubOnlyKeepAlive();
}

std::size_t Config::sleepingClientMsgLimit() const
{
    return m_pImpl->sleepingClientMsgLimit();
}

const Config::PredefinedTopicsList& Config::predefinedTopics() const
{
    return m_pImpl->predefinedTopics();
}

const Config::AuthInfosList& Config::authInfos() const
{
    return m_pImpl->authInfos();
}

Config::TopicIdsRange Config::topicIdAllocRange() const
{
    return m_pImpl->topicIdAllocRange();
}

const std::string& Config::brokerTcpHostAddress() const
{
    return m_pImpl->brokerTcpHostAddress();
}

std::uint16_t Config::brokerTcpHostPort() const
{
    return m_pImpl->brokerTcpHostPort();
}


}  // namespace gateway

}  // namespace cc_mqttsn


