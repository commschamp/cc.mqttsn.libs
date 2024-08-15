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

GatewayProgramOptions::GatewayProgramOptions()
{
    po::options_description opts("Options");
    opts.add_options()
        ("help,h", "Display help message")
        ("config,c", po::value<std::string>()->default_value(std::string()), "Configuration file.")
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

} // namespace cc_mqttsn_gateway_app
