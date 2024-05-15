//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ConfigImpl.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>
#include <limits>
#include <map>
#include <string>

namespace cc_mqttsn_gateway
{

namespace
{

const char CommentChar = '#';
const std::string SpaceChars(" \t");
const std::string GatewayIdKey("mqttsn_gw_id");
const std::string AdvertiseKey("mqttsn_advertise");
const std::string RetryPeriodKey("mqttsn_retry_period");
const std::string RetryCountKey("mqttsn_retry_count");
const std::string DefaultClientIdKey("mqttsn_default_client_id");
const std::string PubOnlyKeepAliveKey("mqttsn_pub_only_keep_alive");
const std::string SleepingClientMsgLimitKey("mqttsn_sleeping_client_msg_limit");
const std::string PredefinedTopicKey("mqttsn_predefined_topic");
const std::string AuthKey("mqttsn_auth");
const std::string TopicIdAllocRangeKey("mqttsn_topic_id_alloc_range");
const std::string BrokerKey("mqttsn_broker");
const std::string LogFileKey("mqttsn_log_file");
const std::string ClientSocketKey("mqttsn_client_socket");

const std::uint16_t DefaultAdvertise = 15 * 60;
const unsigned DefaultRetryPeriod = 10;
const unsigned DefaultRetryCount = 3;
const std::uint16_t DefaultPubOnlyKeepAlive = 60;
const std::size_t DefaultMsgLimit = std::numeric_limits<std::size_t>::max();
const std::uint16_t DefaultMinTopicId = 1;
const std::uint16_t DefaultMaxTopicId = 0xfffe;
const std::string DefaultBrokerAddress("127.0.0.1");
const std::uint16_t DefaultBrokerPort = 1883;

}  // namespace

template <typename T>
T ConfigImpl::numericValue(const std::string& key, T defaultValue) const
{
    auto iter = m_map.find(key);
    if (iter == m_map.end()) {
        return defaultValue;
    }

    try {
        return static_cast<T>(std::stoul(iter->second));
    }
    catch (...) {
    }

    return defaultValue;
}


void ConfigImpl::read(std::istream& stream)
{
    ConfigMap map;
    std::string str;
    while (stream.good()) {
        std::getline(stream, str);
        if (str.empty()) {
            continue;
        }

        auto commentPos = str.find(CommentChar);
        if (commentPos != std::string::npos) {
            str.resize(commentPos);
        }

        if (str.empty()) {
            continue;
        }

        auto spacePos = str.find_first_of(SpaceChars);
        if (spacePos == std::string::npos) {
            map.insert(std::make_pair(str, std::string()));
            continue;
        }

        std::string key(str.begin(), str.begin() + spacePos);
        auto valuePos = str.find_first_not_of(SpaceChars, spacePos + 1);
        if (valuePos == std::string::npos) {
            map.insert(std::make_pair(str, std::string()));
            continue;
        }

        map.insert(std::make_pair(std::move(key), std::string(str.begin() + valuePos, str.end())));
    }

    m_map.swap(map);
}

std::uint8_t ConfigImpl::gatewayId() const
{
    return numericValue<std::uint8_t>(GatewayIdKey);
}

std::uint16_t ConfigImpl::advertisePeriod() const
{
    return numericValue<std::uint16_t>(AdvertiseKey, DefaultAdvertise);
}

unsigned ConfigImpl::retryPeriod() const
{
    return numericValue<unsigned>(RetryPeriodKey, DefaultRetryPeriod);
}

unsigned ConfigImpl::retryCount() const
{
    return numericValue<unsigned>(RetryCountKey, DefaultRetryCount);
}

const std::string& ConfigImpl::defaultClientId() const
{
    return stringValue(DefaultClientIdKey);
}

std::uint16_t ConfigImpl::pubOnlyKeepAlive() const
{
    return numericValue<std::uint16_t>(PubOnlyKeepAliveKey, DefaultPubOnlyKeepAlive);
}

std::size_t ConfigImpl::sleepingClientMsgLimit() const
{
    return numericValue<std::size_t>(SleepingClientMsgLimitKey, DefaultMsgLimit);
}

const ConfigImpl::PredefinedTopicsList& ConfigImpl::predefinedTopics() const
{
    if (!m_topics.empty()) {
        return m_topics;
    }

    auto topics = m_map.equal_range(PredefinedTopicKey);
    if (topics.first == topics.second) {
        return m_topics;
    }

    decltype(m_topics) topicsList;
    topicsList.reserve(std::distance(topics.first, topics.second));
    for (auto iter = topics.first; iter != topics.second; ++iter) {
        try {
            auto& valStr = iter->second;

            auto firstSpacePos = valStr.find_first_of(SpaceChars);
            if (firstSpacePos == std::string::npos) {
                continue;
            }

            auto topicPos = valStr.find_first_not_of(SpaceChars, firstSpacePos + 1);
            if (topicPos == std::string::npos) {
                continue;
            }

            auto secondSpacePos = valStr.find_first_of(SpaceChars, topicPos + 1);
            if (secondSpacePos == std::string::npos) {
                continue;
            }

            auto topicIdPos = valStr.find_first_not_of(SpaceChars, secondSpacePos + 1);
            if (topicIdPos == std::string::npos) {
                continue;
            }

            std::string topicIdStr(valStr.begin() + topicIdPos, valStr.end());
            auto thirdSpacePos = valStr.find_first_of(SpaceChars, topicIdPos + 1);
            if (thirdSpacePos != std::string::npos) {
                topicIdStr.resize(thirdSpacePos - topicIdPos);
            }

            int base = 10;
            if ((2U < topicIdStr.size()) &&
                (topicIdStr[0] == '0') &&
                (static_cast<char>(std::tolower(topicIdStr[1])) == 'x')) {
                base = 16;
            }

            auto topicId = static_cast<std::uint16_t>(std::stoul(topicIdStr, 0 , base));
            if ((topicId == 0) ||
                (topicId == 0xffff)) {
                continue;
            }

            PredefinedTopicInfo info;
            info.clientId.assign(valStr.begin(), valStr.begin() + firstSpacePos);
            info.topic.assign(valStr.begin() + topicPos, valStr.begin() + secondSpacePos);
            info.topicId = topicId;
            topicsList.push_back(std::move(info));
        }
        catch (...) {
            continue;
        }
    }

    std::sort(
        topicsList.begin(), topicsList.end(),
        [](PredefinedTopicsList::const_reference elem1, PredefinedTopicsList::const_reference elem2) -> bool
        {
            return elem1.clientId < elem2.clientId;
        });
    m_topics.swap(topicsList);
    return m_topics;
}

const ConfigImpl::AuthInfosList& ConfigImpl::authInfos() const
{
    if (!m_authInfos.empty()) {
        return m_authInfos;
    }

    auto auth = m_map.equal_range(AuthKey);
    if (auth.first == auth.second) {
        return m_authInfos;
    }

    decltype(m_authInfos) authInfos;
    authInfos.reserve(std::distance(auth.first, auth.second));
    for (auto iter = auth.first; iter != auth.second; ++iter) {
        auto& valStr = iter->second;

        AuthInfo info;
        do {
            auto firstSpacePos = valStr.find_first_of(SpaceChars);
            if (firstSpacePos == std::string::npos) {
                info.clientId = valStr;
                break;
            }

            info.clientId.assign(valStr.begin(), valStr.begin() + firstSpacePos);
            auto usernamePos = valStr.find_first_not_of(SpaceChars, firstSpacePos + 1);
            if (usernamePos == std::string::npos) {
                break;
            }

            auto secondSpacePos = valStr.find_first_of(SpaceChars, usernamePos + 1);
            if (secondSpacePos == std::string::npos) {
                info.username.assign(valStr.begin() + usernamePos, valStr.end());
                break;
            }

            info.username.assign(valStr.begin() + usernamePos, valStr.begin() + secondSpacePos);

            auto passwordPos = valStr.find_first_not_of(SpaceChars, secondSpacePos + 1);
            if (passwordPos == std::string::npos) {
                break;
            }

            auto endOfPasswordPos = valStr.find_last_not_of(SpaceChars) + 1;
            assert(passwordPos < endOfPasswordPos);
            info.password.assign(valStr.begin() + passwordPos, valStr.begin() + endOfPasswordPos);
        } while (false);

        if (!info.clientId.empty()) {
            authInfos.push_back(std::move(info));
        }
    }

    std::sort(
        authInfos.begin(), authInfos.end(),
        [](AuthInfosList::const_reference elem1, AuthInfosList::const_reference elem2) -> bool
        {
            return elem1.clientId < elem2.clientId;
        });

    m_authInfos.swap(authInfos);
    return m_authInfos;
}

ConfigImpl::TopicIdsRange ConfigImpl::topicIdAllocRange() const
{
    auto minVal = DefaultMinTopicId;
    auto maxVal = DefaultMaxTopicId;
    do {
        auto iter = m_map.find(TopicIdAllocRangeKey);
        if (iter == m_map.end()) {
            break;
        }

        auto& valStr = iter->second;
        if (valStr.empty()) {
            break;
        }

        std::string minNumStr;
        auto firstSpacePos = valStr.find_first_of(SpaceChars);
        if (firstSpacePos == std::string::npos) {
            minNumStr = valStr;
        }
        else {
            minNumStr.assign(valStr.begin(), valStr.begin() + firstSpacePos);
        }

        try {
            minVal = static_cast<decltype(minVal)>(std::stoul(minNumStr));
        }
        catch (...) {
            // Nothing to do
        }

        if (firstSpacePos == std::string::npos) {
            break;
        }


        auto maxNumPos = valStr.find_first_not_of(SpaceChars, firstSpacePos + 1);
        if (maxNumPos == std::string::npos) {
            break;
        }

        std::string maxNumStr;
        auto secondSpacePos = valStr.find_first_of(SpaceChars, maxNumPos + 1);
        if (secondSpacePos == std::string::npos) {
            maxNumStr.assign(valStr.begin() + maxNumPos, valStr.end());
        }
        else {
            maxNumStr.assign(valStr.begin() + maxNumPos, valStr.begin() + secondSpacePos);
        }

        try {
            maxVal = static_cast<decltype(minVal)>(std::stoul(maxNumStr));
        }
        catch (...) {
            // Nothing to do
        }
    } while (false);

    if (maxVal < minVal) {
        std::swap(minVal, maxVal);
    }

    minVal = std::max(DefaultMinTopicId, minVal);
    maxVal = std::min(DefaultMaxTopicId, maxVal);
    return std::make_pair(minVal, maxVal);
}

const std::string& ConfigImpl::brokerTcpHostAddress() const
{
    if (m_brokerAddress.empty()) {
        readBrokerAddrInfo();
    }

    assert(!m_brokerAddress.empty());
    return m_brokerAddress;
}

std::uint16_t ConfigImpl::brokerTcpHostPort() const
{
    if (m_brokerPort == 0) {
        readBrokerAddrInfo();
    }

    assert(m_brokerPort != 0);
    return m_brokerPort;
}

const std::string& ConfigImpl::logFile() const
{
    static const std::string DefaultLogFile("stdout");
    return stringValue(LogFileKey, DefaultLogFile);
}

ConfigImpl::ClientConnectionType ConfigImpl::clientConnectionType() const
{
    static const std::map<std::string, ClientConnectionType> Map = {
        {"udp", ClientConnectionType::ClientConnectionType_Udp},
    };

    ClientConnectionType result = ClientConnectionType::ClientConnectionType_Udp;

    do {
        auto typeStr = stringValue(ClientSocketKey);
        if (typeStr.empty()) {
            break;
        }

        auto iter = Map.find(typeStr);
        if (iter == Map.end()) {
            result = ClientConnectionType::ClientConnectionType_ValuesLimit;
            break;
        }

        result = iter->second;       
    } while (false);

    return result;
}

const std::string& ConfigImpl::stringValue(
    const std::string& key,
    const std::string& defaultValue) const
{
    auto iter = m_map.find(key);
    if (iter == m_map.end()) {
        return defaultValue;
    }

    return iter->second;
}

const std::string& ConfigImpl::stringValue(
    const std::string& key) const
{
    static const std::string EmptyString;
    return stringValue(key, EmptyString);
}

void ConfigImpl::readBrokerAddrInfo() const
{
    m_brokerAddress = DefaultBrokerAddress;
    m_brokerPort = DefaultBrokerPort;

    auto iter = m_map.find(BrokerKey);
    if (iter == m_map.end()) {
        return;
    }

    auto& valStr = iter->second;
    auto firstSpacePos = valStr.find_first_of(SpaceChars);
    if (firstSpacePos == std::string::npos) {
        m_brokerAddress = valStr;
        return;
    }

    m_brokerAddress.assign(valStr.begin(), valStr.begin() + firstSpacePos);

    auto portPos = valStr.find_first_not_of(SpaceChars, firstSpacePos + 1);
    if (portPos == std::string::npos) {
        return;
    }

    std::string portStr(valStr.begin() + portPos, valStr.end());
    auto secondSpacePos = valStr.find_first_of(SpaceChars, portPos + 1);
    if (secondSpacePos != std::string::npos) {
        portStr.assign(valStr.begin() + portPos, valStr.begin() + secondSpacePos);
    }

    try {
        m_brokerPort = static_cast<decltype(m_brokerPort)>(std::stoul(portStr));
    }
    catch (...) {
        // Nothing to do
    }
}

}  // namespace cc_mqttsn_gateway
