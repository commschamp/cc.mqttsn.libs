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

#include <memory>
#include <cstdint>
#include <map>
#include <iosfwd>
#include <vector>
#include <cstdint>
#include <string>
#include <list>
#include <utility>

namespace mqttsn
{

namespace gateway
{

class ConfigImpl;
class Config
{
public:

    typedef std::multimap<std::string, std::string> ConfigMap;
    typedef std::vector<std::uint8_t> BinaryData;

    struct PredefinedTopicInfo
    {
        std::string clientId;
        std::string topic;
        std::uint16_t topicId = 0;
    };
    typedef std::vector<PredefinedTopicInfo> PredefinedTopicsList;

    struct AuthInfo
    {
        std::string clientId;
        std::string username;
        std::string password;
    };

    typedef std::vector<AuthInfo> AuthInfosList;

    typedef std::pair<std::uint16_t, std::uint16_t> TopicIdsRange;


    Config();
    ~Config();

    void read(std::istream& stream);
    const ConfigMap& configMap() const;

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
    std::unique_ptr<ConfigImpl> m_pImpl;
};

}  // namespace gateway

}  // namespace mqttsn


