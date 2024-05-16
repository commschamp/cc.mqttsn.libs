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
    

GatewayApp::GatewayApp(boost::asio::io_context& io) : 
    m_io(io)
{
}

GatewayApp::~GatewayApp() = default;

bool GatewayApp::start(int argc, const char* argv[])
{
    GatewayProgramOptions opts;

    if (!opts.parseArgs(argc, argv)) {
        m_logger.error() << "Failed to parse arguments." << std::endl;
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
            m_logger.info() << "No configuration file provided, using default configuration." << std::endl;
            break;
        }

        std::ifstream stream(configFile);
        if (!stream) {
            m_logger.warning() << "Failed to open configuration file \"" <<
                configFile << "\", using default configuration." << std::endl;
            break;
        }

        m_config.read(stream);
    } while (false);    

    m_logger.configure(m_config);

    m_acceptor = GatewayIoClientAcceptor::create(m_io, m_logger, m_config);
    if (!m_acceptor) {
        m_logger.error() << "Unknown / unsupported client socket type" << std::endl;
        return false;
    }

    if (!m_acceptor->start()) {
        m_logger.error() << "Failed to start client socket" << std::endl;
        return false;
    }    

    return true;
}

} // namespace cc_mqttsn_gateway_app
