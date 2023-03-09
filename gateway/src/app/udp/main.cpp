//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>
#include <fstream>

#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QtCore/QDir>
#include <QtCore/QCommandLineParser>
#include <QtCore/QStringList>

#include "Mgr.h"
#include "cc_mqttsn_gateway/Config.h"

namespace
{

const QString ConfigOptStr("config");

void prepareCommandLineOptions(QCommandLineParser& parser)
{
    parser.addHelpOption();

    QCommandLineOption configOpt(
        QStringList() << "c" << ConfigOptStr,
        QCoreApplication::translate("main", "Configuration file."),
        QCoreApplication::translate("main", "filename")
    );
    parser.addOption(configOpt);

}

}  // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    prepareCommandLineOptions(parser);
    parser.process(app);

    cc_mqttsn_gateway::Config config;
    do {
        if (!parser.isSet(ConfigOptStr)) {
            break;
        }

        auto configFile = parser.value(ConfigOptStr).toStdString();
        std::ifstream stream(configFile);
        if (!stream) {
            std::cerr << "WARNING: Failed to open configuration file \"" <<
                configFile << "\", using default configuration." << std::endl;
            break;
        }

        config.read(stream);
    } while (false);

    cc_mqttsn_gateway::app::udp::Mgr gw(config);
    if (!gw.start()) {
        std::cerr << "Failed to start!" << std::endl;
        return -1;
    }

    return app.exec();
}



