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

#include "mqttsn/gateway/Config.h"

#include "ConfigImpl.h"

namespace mqttsn
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

const std::string& Config::pubOnlyClientId() const
{
    return m_pImpl->pubOnlyClientId();
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

}  // namespace gateway

}  // namespace mqttsn


