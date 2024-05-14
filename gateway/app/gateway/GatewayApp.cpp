//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayApp.h"

#include <fstream>
#include <iostream>

namespace cc_mqttsn_gateway_app
{

namespace 
{

std::ostream& logError()
{
    return std::cerr << "ERROR: ";
}

std::ostream& logInfo()
{
    return std::cout << "INFO: ";
}

std::ostream& logWarning()
{
    return std::cout << "WARNING: ";
}

} // namespace 
    

GatewayApp::GatewayApp(boost::asio::io_context& io) : 
    m_io(io)
{
}

GatewayApp::~GatewayApp() = default;

bool GatewayApp::start(int argc, const char* argv[])
{
    GatewayProgramOptions opts;

    if (!opts.parseArgs(argc, argv)) {
        logError() << "Failed to parse arguments." << std::endl;
        return false;
    }

    if (opts.helpRequested()) {
        std::cout << "Usage: " << argv[0] << " [options...]" << '\n';
        opts.printHelp();
        m_io.stop();
        return true;
    }

    auto configFile = opts.configFile();
    do {
        if (configFile.empty()) {
            logInfo() << "No configuration file provided, using default configuration." << std::endl;
            break;
        }

        std::ifstream stream(configFile);
        if (!stream) {
            logWarning() << "Failed to open configuration file \"" <<
                configFile << "\", using default configuration." << std::endl;
            break;
        }

        m_config.read(stream);
    } while (false);    

    // TODO:
    logError() << "NYI" << std::endl;
    return false;
}

} // namespace cc_mqttsn_gateway_app
