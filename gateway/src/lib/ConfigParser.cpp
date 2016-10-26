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

#include "mqttsn/gateway/ConfigParser.h"

#include "ConfigParserImpl.h"

namespace mqttsn
{

namespace gateway
{

namespace
{

const std::string EmptyStr;

}  // namespace

ConfigParser::ConfigParser()
  : m_pImpl(new ConfigParserImpl)
{
}

ConfigParser::~ConfigParser() = default;

void ConfigParser::parseConfig(std::istream& stream)
{
    m_pImpl->parseConfig(stream);
}

const ConfigParser::ConfigMap& ConfigParser::configMap() const
{
    return m_pImpl->configMap();
}

std::uint8_t ConfigParser::gatewayId() const
{
    return m_pImpl->gatewayId();
}

std::uint16_t ConfigParser::advertisePeriod() const
{
    return m_pImpl->advertisePeriod();
}

const std::string& ConfigParser::username() const
{
    auto& list = allUsernames();
    if (list.empty()) {
        return EmptyStr;
    }

    return list.front();
}

const std::list<std::string>& ConfigParser::allUsernames() const
{
    return m_pImpl->allUsernames();
}

}  // namespace gateway

}  // namespace mqttsn


