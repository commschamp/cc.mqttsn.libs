//
// Copyright 2023 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayProgramOptions.h"

#include <cassert>
#include <iostream>
#include <map>

namespace po = boost::program_options;

namespace cc_mqttsn_gateway_app
{

namespace 
{

using ConnectionTypeMap = std::map<std::string, GatewayProgramOptions::ConnectionType>;
const ConnectionTypeMap& connectionTypeMap()
{
    static const ConnectionTypeMap Map = {
        {"udp", GatewayProgramOptions::ConnectionType_Udp},
    };

    return Map;
}

std::string supportedConnectionTypes()
{
    std::string result;
    for (auto& info : connectionTypeMap()) {
        if (!result.empty()) {
            result.append(", ");
        }

        result.append(info.first);
    }

    return result;
}

const std::string& defaultConnectionType()
{
    auto& map = connectionTypeMap();
    auto iter = 
        std::find_if(
            map.begin(), map.end(),
            [](auto& info)
            {
                return info.second == GatewayProgramOptions::DefaultConnectionType;
            });

    assert(iter != map.end());
    return iter->first;
}

} // namespace 
    

GatewayProgramOptions::GatewayProgramOptions()
{
    po::options_description opts("Options");
    opts.add_options()
        ("help,h", "Display help message")
        ("config,c", po::value<std::string>()->default_value(std::string()), "Configuration file.")
        ("socket,s", po::value<std::string>()->default_value(defaultConnectionType()), 
            ("Low level connection socket type. Available: " + supportedConnectionTypes()).c_str())
    ;    

    m_desc.add(opts);
}    

void GatewayProgramOptions::printHelp()
{
    std::cout << m_desc << std::endl;
}

bool GatewayProgramOptions::parseArgs(int argc, const char* argv[])
{
    po::store(po::parse_command_line(argc, argv, m_desc), m_vm);
    po::notify(m_vm);  

    return true;
}

bool GatewayProgramOptions::helpRequested() const
{
    return m_vm.count("help") > 0U;
}

std::string GatewayProgramOptions::configFile() const
{
    return m_vm["config"].as<std::string>();
}

GatewayProgramOptions::ConnectionType GatewayProgramOptions::connectionType() const
{
    auto socketStr = m_vm["socket"].as<std::string>();
    auto& map = connectionTypeMap();
    auto iter = map.find(socketStr);
    if (iter == map.end()) {
        return ConnectionType_ValuesLimit;
    }

    return iter->second;
}

} // namespace cc_mqttsn_gateway_app
