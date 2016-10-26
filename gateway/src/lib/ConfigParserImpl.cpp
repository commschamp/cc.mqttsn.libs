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

#include "ConfigParserImpl.h"

#include <string>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace
{

const char CommentChar = '#';
const std::string SpaceChars(" \t");
const std::string GatewayIdKey("mqttsn_gw_id");
const std::string AdvertiseKey("mqttsn_advertise");
const std::string UsernameKey("mqttsn_username");

std::uint16_t DefaultAdvertise = 15 * 60;

}  // namespace

template <typename T>
T ConfigParserImpl::numericValue(const std::string& key, T defaultValue) const
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


void ConfigParserImpl::parseConfig(std::istream& stream)
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

std::uint8_t ConfigParserImpl::gatewayId() const
{
    return numericValue<std::uint8_t>(GatewayIdKey);
}

std::uint16_t ConfigParserImpl::advertisePeriod() const
{
    return numericValue<std::uint16_t>(AdvertiseKey, DefaultAdvertise);
}

const std::list<std::string>& ConfigParserImpl::allUsernames() const
{
    if (!m_usernames.empty()) {
        return m_usernames;
    }

    auto result = m_map.equal_range(UsernameKey);
    std::transform(
        result.first, result.second, std::back_inserter(m_usernames),
        [](ConfigMap::const_reference elem) -> const std::string&
        {
            return elem.second;
        });
    return m_usernames;
}

}  // namespace gateway

}  // namespace mqttsn


