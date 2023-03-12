//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains interface of cc_mqttsn_gateway::Config class.

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

namespace cc_mqttsn_gateway
{

class ConfigImpl;

/// @brief Interface for @b Config entity.
/// @details The responsibility of the @b Config object is to to provide a
///     common way to parse the configuration file and provide a convenient
///     interface to retrieve any configuration values, which can later be
///     applied to the @b Gateway and/or @b Session objects.
class Config
{
public:

    /// @brief Full configuration map
    /// @details The key is the first word in the configuration line, and the
    ///     value is rest of the string until the end of the line.
    ///     @b NOTE, that the type is @b multimap, which allows multiple
    ///     entries with the same key.
    typedef std::multimap<std::string, std::string> ConfigMap;

    /// @brief Type of buffer that contains binary data.
    typedef std::vector<std::uint8_t> BinaryData;

    /// @brief Info about single predefined topic
    struct PredefinedTopicInfo
    {
        std::string clientId; ///< Client ID
        std::string topic; ///< Topic string
        std::uint16_t topicId = 0; ///< Numeric topic ID
    };

    /// @brief Type of list containing predefined topics.
    typedef std::vector<PredefinedTopicInfo> PredefinedTopicsList;

    /// @brief Authentication info for single client
    struct AuthInfo
    {
        std::string clientId; ///< Client ID
        std::string username; ///< Username
        std::string password; ///< Password string (from the configuration)
    };

    /// @brief Type of list containing authentication information for multiple clients.
    typedef std::vector<AuthInfo> AuthInfosList;

    /// @brief Range of topic IDs
    /// @details First element of the pair is minimal ID, and second
    ///     element of the pair is maximal ID.
    typedef std::pair<std::uint16_t, std::uint16_t> TopicIdsRange;

    /// @brief Constructor
    Config();

    /// @brief Destructor
    ~Config();

    /// @brief Read configuration from input stream
    /// @details Updates the default values with values read from the stream.
    /// @param[in] stream Input stream.
    void read(std::istream& stream);

    /// @brief Get access to the full configuration map.
    const ConfigMap& configMap() const;

    /// @brief Get gateway numeric ID.
    /// @details Default value is @b 0.
    /// @return Numeric gateway ID.
    std::uint8_t gatewayId() const;

    /// @brief Get advertise period.
    /// @details Default value is @b 900 seconds (15 minutes).
    /// @return Advertise period in @b seconds.
    std::uint16_t advertisePeriod() const;

    /// @brief Get retry period
    /// @details Default value is @b 10 seconds.
    /// @return Retry period in @b seconds
    unsigned retryPeriod() const;

    /// @brief Get number of retry attempts.
    /// @details Default value is @b 3.
    /// @return Number of retry attempts
    unsigned retryCount() const;

    /// @brief Get default client ID.
    /// @details Default value is empty string.
    /// @return Default client ID.
    const std::string& defaultClientId() const;

    /// @brief Get keep alive period for publish only clients
    /// @details Default value is @b 60 seconds.
    /// @return Keep alive period for publish only clients.
    std::uint16_t pubOnlyKeepAlive() const;

    /// @brief Get limit for max number of messages to accumulate for sleeping
    ///     clients.
    /// @details Default value is equivalent to @b std::numeric_limits<std::size_t>::max() seconds.
    /// @return Max number of accumulated messages for sleeping clients.
    std::size_t sleepingClientMsgLimit() const;

    /// @brief Get access to the list of predefined topics.
    const PredefinedTopicsList& predefinedTopics() const;

    /// @brief Get access to list of authentication informations.
    const AuthInfosList& authInfos() const;

    /// @brief Get range of allowed topic IDs for allocation.
    /// @details Default range is [1, 0xfffe]
    TopicIdsRange topicIdAllocRange() const;

    /// @brief Get TCP/IP address of the broker.
    /// @details Default address is @b 127.0.0.1
    const std::string& brokerTcpHostAddress() const;

    /// @brief Get TCP/IP port of the broker.
    /// @details Default value is @b 1883
    std::uint16_t brokerTcpHostPort() const;

private:
    std::unique_ptr<ConfigImpl> m_pImpl;
};

}  // namespace cc_mqttsn_gateway


