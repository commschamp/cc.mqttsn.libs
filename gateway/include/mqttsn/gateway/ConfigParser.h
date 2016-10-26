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

#include "mqttsn/gateway/Api.h"


namespace mqttsn
{

namespace gateway
{

class ConfigParserImpl;
class MQTTSN_GATEWAY_API ConfigParser
{
public:

    typedef std::multimap<std::string, std::string> ConfigMap;
    typedef std::vector<std::uint8_t> BinaryData;

    ConfigParser();
    ~ConfigParser();

    void parseConfig(std::istream& stream);
    const ConfigMap& configMap() const;

    std::uint8_t gatewayId() const;
    std::uint16_t advertisePeriod() const;
    const std::string& username() const;
    const std::list<std::string>& allUsernames() const;

private:
    std::unique_ptr<ConfigParserImpl> m_pImpl;
};

}  // namespace gateway

}  // namespace mqttsn


