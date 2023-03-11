//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>
#include <fstream>
#include <limits>
#include <cstdint>
#include <tuple>
#include <algorithm>
#include <random>

#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QtCore/QDir>
#include <QtCore/QCommandLineParser>
#include <QtCore/QStringList>

#include "Sub.h"

namespace
{

const QString GwOpt("gateway");
const QString GwShortOpt("g");
const QString GwIdOpt("gateway-id");
const QString GwIdShortOpt("G");
const QString PortOpt("port");
const QString PortShortOpt("p");
const unsigned short DefaultPort = 1883;
const QString DefaultPortStr = QString("%1").arg(DefaultPort);
const QString ClientIdOpt("client-id");
const QString ClientIdShortOpt("i");
const QString EmptyClientIdOpt("empty-client-id");
const QString EmptyClientIdShortOpt("I");
const QString KeepAliveOpt("keep-alive");
const QString KeepAliveShortOpt("k");
const unsigned short DefaultKeepAlivePeriod = 60;
const QString DefaultKeepAlivePeriodStr = QString("%1").arg(DefaultKeepAlivePeriod);
const QString NoCleanOpt("no-clean");
const QString NoCleanShortOpt("c");
const QString TopicOpt("topic");
const QString TopicShortOpt("t");
const QString TopicIdOpt("topic-id");
const QString TopicIdShortOpt("T");
const QString QosOpt("qos");
const QString QosShortOpt("q");
const QString VerboseOpt("verbose");
const QString VerboseShortOpt("v");
const QString NoRetainOpt("no-retained");
const QString NoRetainShortOpt("R");
const QString HexOpt("hex-output");
const QString HexShortOpt("x");


const int DefaultQos = 2;
const QString DefaultQosStr = QString("%1").arg(DefaultQos);

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
        "Client ID. If not provided, the client ID is randomised. If provided suppresses usage of -" + EmptyClientIdShortOpt + " option.",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(idOpt);

    QCommandLineOption emptyIdOpt(
        QStringList() << EmptyClientIdShortOpt << EmptyClientIdOpt,
        "Use empty client ID."
    );
    parser.addOption(emptyIdOpt);

    QCommandLineOption keepAliveOpt(
        QStringList() << KeepAliveShortOpt << KeepAliveOpt,
        "Keep alive period in seconds. Defaults to " + DefaultKeepAlivePeriodStr + '.',
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(keepAliveOpt);

    QCommandLineOption noCleanOpt(
        QStringList() << NoCleanShortOpt << NoCleanOpt,
        "Connect to GW with 'clean session' bit cleared, i.e. preserve previous subscriptions."
    );
    parser.addOption(noCleanOpt);

    QCommandLineOption topicOpt(
        QStringList() << TopicShortOpt << TopicOpt,
        "Topic to subscribe to, may be repeated multiple times",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(topicOpt);

    QCommandLineOption topicIdOpt(
        QStringList() << TopicIdShortOpt << TopicIdOpt,
        "Predefined topic ID to subscribe to, may be repeated multiple times",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(topicIdOpt);

    QCommandLineOption qosOpt(
        QStringList() << QosShortOpt << QosOpt,
        "Max quality of service to use for subscription. Defaults to " + DefaultQosStr + '.',
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(qosOpt);

    QCommandLineOption verboseOpt(
        QStringList() << VerboseShortOpt << VerboseOpt,
        "Verbose print of published messages."
    );
    parser.addOption(verboseOpt);

    QCommandLineOption noRetainOpt(
        QStringList() << NoRetainShortOpt << NoRetainOpt,
        "Do NOT print retained messages."
    );
    parser.addOption(noRetainOpt);

    QCommandLineOption hexOpt(
        QStringList() << HexShortOpt << HexOpt,
        "Print message body in hexadecimal byte values."
    );
    parser.addOption(hexOpt);
}

std::tuple<QString, unsigned short> splitGwAddr(const QCommandLineParser& parser)
{
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

    return std::make_tuple(gwAddr, gwPort);
}

int getGwId(const QCommandLineParser& parser, const QString& gwAddr)
{
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
    return gwId;
}

QString getClientId(const QCommandLineParser& parser)
{
    QString clientId = parser.value(ClientIdOpt);
    if (!clientId.isEmpty()) {
        return clientId;
    }

    if (parser.isSet(EmptyClientIdOpt)) {
        return clientId;
    }

    clientId = "mqttsn_sub_";
    std::random_device rd;
    std::mt19937 gen(rd());
    static const QString Map =
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<> dis(0, Map.size() - 1);

    static const unsigned GenCount = 10;
    for (auto c = 0U; c < GenCount; ++c) {
        clientId.append(Map[dis(gen)]);
    }
    return clientId;
}

unsigned short getLocalPort(const QCommandLineParser& parser, const QString& gwAddr)
{
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

        port = static_cast<decltype(port)>(portTmp);
    } while (false);
    return port;
}

unsigned short getKeepAlive(const QCommandLineParser& parser)
{
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

        keepAlive = static_cast<decltype(keepAlive)>(keepAliveTmp);
    } while (false);
    return keepAlive;
}

cc_mqttsn_client::app::sub::udp::Sub::TopicsList getTopics(const QCommandLineParser& parser)
{
    auto topicStrings = parser.values(TopicOpt);
    std::vector<std::string> topics;
    topics.reserve(topicStrings.size());
    std::transform(
        topicStrings.begin(), topicStrings.end(), std::back_inserter(topics),
        [](const QString& s) -> std::string
        {
            return s.toStdString();
        });

    std::sort(topics.begin(), topics.end());
    topics.erase(
        std::unique(topics.begin(), topics.end()),
        topics.end());
    return cc_mqttsn_client::app::sub::udp::Sub::TopicsList(topics.begin(), topics.end());
}

cc_mqttsn_client::app::sub::udp::Sub::TopicIdsList getTopicIds(const QCommandLineParser& parser)
{
    auto topicStrings = parser.values(TopicIdOpt);
    std::vector<std::uint16_t> topics;
    topics.reserve(topicStrings.size());
    std::transform(
        topicStrings.begin(), topicStrings.end(), std::back_inserter(topics),
        [](const QString& s) -> std::uint16_t
        {
            return static_cast<std::uint16_t>(s.toUInt());
        });

    topics.erase(
        std::remove_if(
            topics.begin(), topics.end(),
            [](std::uint16_t value) -> bool
            {
                return value == 0;
            }),
        topics.end());

    std::sort(topics.begin(), topics.end());
    topics.erase(
        std::unique(topics.begin(), topics.end()),
        topics.end());

    return cc_mqttsn_client::app::sub::udp::Sub::TopicIdsList(topics.begin(), topics.end());
}

MqttsnQoS getQos(const QCommandLineParser& parser)
{
    auto value = DefaultQos;
    do {
        auto qosStr = parser.value(QosOpt);
        if (qosStr.isEmpty()) {
            break;
        }

        bool ok = false;
        auto valueTmp = qosStr.toInt(&ok);
        if ((!ok) || (valueTmp < 0)) {
            break;
        }

        value = std::min(value, valueTmp);
    } while (false);
    return static_cast<MqttsnQoS>(value);
}


}  // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    prepareCommandLineOptions(parser);
    parser.process(app);

    QString gwAddr;
    unsigned short gwPort = 0;
    std::tie(gwAddr, gwPort) = splitGwAddr(parser);
    int gwId = getGwId(parser, gwAddr);
    unsigned short port = getLocalPort(parser, gwAddr);
    auto keepAlive = getKeepAlive(parser);

    cc_mqttsn_client::app::sub::udp::Sub sub;

    sub.setGwAddr(gwAddr);
    sub.setGwPort(gwPort);
    sub.setGwId(gwId);
    sub.setLocalPort(port);
    sub.setClientId(getClientId(parser).toStdString());
    sub.setKeepAlive(keepAlive);
    sub.setCleanSession(!parser.isSet(NoCleanOpt));
    sub.setTopics(getTopics(parser));
    sub.setTopicIds(getTopicIds(parser));
    sub.setQos(getQos(parser));
    sub.setVerbose(parser.isSet(VerboseOpt));
    sub.setNoRetain(parser.isSet(NoRetainOpt));
    sub.setHexOutput(parser.isSet(HexOpt));

    if (!sub.start()) {
        std::cerr << "ERROR: Failed to start" << std::endl;
        return -1;
    }

    return app.exec();
}



