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
#include <limits>
#include <cstdint>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QtCore/QDir>
#include <QtCore/QCommandLineParser>
#include <QtCore/QStringList>
CC_ENABLE_WARNINGS()

#include "Sub.h"

namespace
{

const QString GwOpt("gateway");
const QString GwShortOpt("G");
const QString GwIdOpt("gateway-id");
const QString GwIdShortOpt("g");
const QString PortOpt("port");
const QString PortShortOpt("p");
const unsigned short DefaultPort = 1883;
const QString DefaultPortStr = QString("%1").arg(DefaultPort);
const QString ClientIdOpt("client-id");
const QString ClientIdShortOpt("i");
const QString KeepAliveOpt("keep-alive");
const QString KeepAliveShortOpt("keep-alive");
const unsigned short DefaultKeepAlivePeriod = 60;
const QString DefaultKeepAlivePeriodStr = QString("%1").arg(DefaultKeepAlivePeriod);
const QString NoCleanOpt("no-clean");
const QString NoCleanShortOpt("c");

void prepareCommandLineOptions(QCommandLineParser& parser)
{
    parser.addHelpOption();

    QCommandLineOption gwOpt(
        QStringList() << GwShortOpt << GwOpt,
        "Gateway address. Expected to be in <address>:<port> format. "
        "Port is optional, if not provided defaults to " + DefaultPortStr +
        ". If this option is used, no SEARCHGW messages are sent to the gateway, and -" +
        GwIdShortOpt + " option is ignored." ,
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(gwOpt);

    QCommandLineOption gwIdOpt(
        QStringList() << GwIdShortOpt << GwIdOpt,
        QCoreApplication::translate("main",
            "Gateway ID to connect to when discovered. "
            "If not provided, the first gateway, that responds to SEARCHGW message, "
            "will be chosen."),
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(gwIdOpt);

    QCommandLineOption portOpt(
        QStringList() << PortShortOpt << PortOpt,
        "Local network port. Defaults to " + DefaultPortStr + " in case -" + GwShortOpt + " option is NOT used.",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(portOpt);

    QCommandLineOption idOpt(
        QStringList() << ClientIdShortOpt << ClientIdOpt,
        "Client ID.",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(idOpt);

    QCommandLineOption keepAliveOpt(
        QStringList() << KeepAliveShortOpt << KeepAliveOpt,
        "Keep alive period in seconds. Defaults to " + DefaultKeepAlivePeriodStr + '.',
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(keepAliveOpt);


    QCommandLineOption noCleanOpt(
        QStringList() << NoCleanShortOpt << NoCleanOpt,
        "Connect to GW with 'clean session' bit cleared, i.e. preserve previous subscriptions.",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(noCleanOpt);

}

}  // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    prepareCommandLineOptions(parser);
    parser.process(app);

    auto gwUrl = parser.value(GwOpt);
    auto gwAddr = gwUrl;
    auto gwPort = DefaultPort;
    auto colonIdx = gwUrl.indexOf(QChar(':'));
    if (0 < colonIdx) {
        gwAddr = gwUrl.left(colonIdx);
        auto gwPortStr = gwUrl.right(gwUrl.size() - (colonIdx + 1));
        bool ok = false;
        auto gwPortTmp = gwPortStr.toUInt(&ok);
        if (ok && (0 < gwPortTmp) && (gwPortTmp <= std::numeric_limits<std::uint16_t>::max())) {
            gwPort = static_cast<decltype(gwPort)>(gwPortTmp);
        }
    }

    int gwId = -1;
    do {
        if (!gwAddr.isEmpty()) {
            break;
        }

        auto gwIdStr = parser.value(GwIdOpt);
        if (gwIdStr.isEmpty()) {
            break;
        }

        bool ok = false;
        auto gwIdTmp = gwIdStr.toInt(&ok);
        if ((!ok) || (gwIdTmp < 0) || (0xffff < gwIdTmp)) {
            break;
        }

        gwId = gwIdTmp;
    } while (false);

    unsigned short port = DefaultPort;
    do {
        if (!gwAddr.isEmpty()) {
            port = 0;
            break;
        }

        auto portStr = parser.value(PortOpt);
        if (portStr.isEmpty()) {
            break;
        }

        bool ok = false;
        auto portTmp = portStr.toUInt(&ok);
        if ((!ok) || (0xffff < portTmp)) {
            break;
        }

        port = portTmp;
    } while (false);

    auto keepAlive = DefaultKeepAlivePeriod;
    do {
        auto keepAliveStr = parser.value(KeepAliveOpt);
        if (keepAliveStr.isEmpty()) {
            break;
        }

        bool ok = false;
        auto keepAliveTmp = keepAliveStr.toUInt(&ok);
        if ((!ok) || (keepAliveTmp == 0) || (0xffff < keepAliveTmp)) {
            break;
        }

        keepAlive = keepAliveTmp;
    } while (false);

    mqttsn::client::app::sub::udp::Sub sub;

    sub.setGwAddr(gwAddr);
    sub.setGwPort(gwPort);
    sub.setGwId(gwId);
    sub.setLocalPort(port);
    sub.setClientId(parser.value(ClientIdOpt).toStdString());
    sub.setKeepAlive(keepAlive);
    sub.setCleanSession(!parser.isSet(NoCleanOpt));

    if (!sub.start()) {
        std::cerr << "ERROR: Failed to start" << std::endl;
        return -1;
    }



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



