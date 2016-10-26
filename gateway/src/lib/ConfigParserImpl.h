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

#include "mqttsn/gateway/ConfigParser.h"

namespace mqttsn
{

namespace gateway
{

class ConfigParserImpl
{
public:
    typedef ConfigParser::ConfigMap ConfigMap;

    ConfigParserImpl() = default;
    ~ConfigParserImpl() = default;

    void parseConfig(std::istream& stream);

    const ConfigMap& configMap() const
    {
        return m_map;
    }

    std::uint8_t gatewayId() const;

    std::uint16_t advertisePeriod() const;

    const std::list<std::string>& allUsernames() const;


private:
    template <typename T>
    T numericValue(const std::string& key, T defaultValue = T()) const;

    ConfigMap m_map;
    mutable std::list<std::string> m_usernames;
};

}  // namespace gateway

}  // namespace mqttsn


