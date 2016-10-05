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

#include <string>
#include <cstdint>
#include <list>
#include <map>

namespace mqttsn
{

namespace gateway
{

class RegMgr
{
public:
    enum class Type
    {
        Invalid,
        Predefined,
        FromClient,
        FromGw
    };

    bool regPredefined(const std::string& topic, std::uint16_t topicId);
    std::uint16_t mapTopic(const std::string& topic, Type type);

private:

    struct RegInfo
    {
        std::string m_topic;
        std::uint16_t m_topicId = 0U;
        Type m_type = Type::Invalid;
    };
    typedef std::list<RegInfo> RegInfosList;

    void removeFromTopicMap(const std::string& topic, RegInfosList::iterator info);

    RegInfosList m_regInfos;
    std::multimap<std::string, RegInfosList::iterator> m_regInfosMap;
    std::map<std::uint16_t, RegInfosList::iterator> m_regInfosRevMap;
};

}  // namespace gateway

}  // namespace mqttsn


