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


#pragma once

#include <iostream>

#include "mqttsn/gateway/Config.h"

namespace mqttsn
{

namespace gateway
{

class ConfigImpl
{
public:
    typedef Config::ConfigMap ConfigMap;
    typedef Config::PredefinedTopicInfo PredefinedTopicInfo;
    typedef Config::PredefinedTopicsList PredefinedTopicsList;
    typedef Config::AuthInfo AuthInfo;
    typedef Config::AuthInfosList AuthInfosList;
    typedef Config::TopicIdsRange TopicIdsRange;


    ConfigImpl() = default;
    ~ConfigImpl() = default;

    void read(std::istream& stream);

    const ConfigMap& configMap() const
    {
        return m_map;
    }

    std::uint8_t gatewayId() const;

    std::uint16_t advertisePeriod() const;

    unsigned retryPeriod() const;

    unsigned retryCount() const;

    const std::string& defaultClientId() const;

    std::uint16_t pubOnlyKeepAlive() const;

    std::size_t sleepingClientMsgLimit() const;

    const PredefinedTopicsList& predefinedTopics() const;
    const AuthInfosList& authInfos() const;

    TopicIdsRange topicIdAllocRange() const;

    const std::string& brokerTcpHostAddress() const;
    std::uint16_t brokerTcpHostPort() const;

private:
    template <typename T>
    T numericValue(const std::string& key, T defaultValue = T()) const;

    const std::string& stringValue(const std::string& key, const std::string& defaultValue) const;
    const std::string& stringValue(const std::string& key) const;
    void readBrokerAddrInfo() const;

    ConfigMap m_map;
    mutable PredefinedTopicsList m_topics;
    mutable AuthInfosList m_authInfos;
    mutable std::string m_brokerAddress;
    mutable std::uint16_t m_brokerPort = 0;
};

}  // namespace gateway

}  // namespace mqttsn


