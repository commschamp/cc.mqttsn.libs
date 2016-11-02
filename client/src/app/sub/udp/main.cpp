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


#include <iostream>
#include <fstream>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QtCore/QDir>
#include <QtCore/QCommandLineParser>
#include <QtCore/QStringList>
CC_ENABLE_WARNINGS()


namespace
{

void prepareCommandLineOptions(QCommandLineParser& parser)
{
    parser.addHelpOption();

}

}  // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    prepareCommandLineOptions(parser);
    parser.process(app);

//    mqttsn::gateway::Config config;
//    do {
//        if (!parser.isSet(ConfigOptStr)) {
//            break;
//        }
//
//        auto configFile = parser.value(ConfigOptStr).toStdString();
//        std::ifstream stream(configFile);
//        if (!stream) {
//            std::cerr << "WARNING: Failed to open configuration file \"" <<
//                configFile << "\", using default configuration." << std::endl;
//            break;
//        }
//
//        config.read(stream);
//    } while (false);
//
//    mqttsn::gateway::app::udp::Mgr gw(config);
//    if (!gw.start()) {
//        std::cerr << "Failed to start!" << std::endl;
//        return -1;
//    }

    return app.exec();
}



