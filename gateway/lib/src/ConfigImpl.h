//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iostream>

#include "cc_mqttsn_gateway/Config.h"

namespace cc_mqttsn_gateway
{

class ConfigImpl
{
public:
    using ConfigMap = Config::ConfigMap;
    using PredefinedTopicInfo = Config::PredefinedTopicInfo;
    using PredefinedTopicsList = Config::PredefinedTopicsList;
    using AuthInfo = Config::AuthInfo;
    using AuthInfosList = Config::AuthInfosList;
    using TopicIdsRange = Config::TopicIdsRange;
    using ClientConnectionType = Config::ClientConnectionType;
    using BrokerConnectionType = Config::BrokerConnectionType;


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

    const std::string& logFile() const;

    ClientConnectionType clientConnectionType() const;
    BrokerConnectionType brokerConnectionType() const;

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

}  // namespace cc_mqttsn_gateway
