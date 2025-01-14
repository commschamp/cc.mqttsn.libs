//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayLogger.h"

#include <map>

namespace cc_mqttsn_gateway_app
{

GatewayLogger::GatewayLogger() :
    m_out(&std::cout)
{
}

void GatewayLogger::configure(const cc_mqttsn_gateway::Config& config)
{
    const std::map<std::string, std::ostream*> Map = {
        {"stdout", &std::cout},
        {"stderr", &std::cerr},
    };

    auto& logFile = config.logFile();
    auto iter = Map.find(logFile);
    if (iter != Map.end()) {
        m_out = iter->second;
        return;
    }

    m_fstream = std::make_unique<std::ofstream>(logFile, std::ios_base::out | std::ios_base::app);
    if (!(*m_fstream)) {
        error() << "Failed to open " << logFile << " for logging." << std::endl;
        return;
    }

    m_out = m_fstream.get();
}

std::ostream& GatewayLogger::error()
{
    return (*m_out) << "[ERROR]: ";
}

std::ostream& GatewayLogger::info()
{
    return (*m_out) << "[INFO]: ";
}

std::ostream& GatewayLogger::warning()
{
    return (*m_out) << "[WARNING]: ";
}

} // namespace cc_mqttsn_gateway_app
