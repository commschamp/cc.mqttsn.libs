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

#include "Pub.h"

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
const QString RetainOpt("retain");
const QString RetainShortOpt("r");
const QString MessageOpt("message");
const QString MessageShortOpt("m");
const int DefaultQos = 0;
const QString DefaultQosStr = QString("%1").arg(DefaultQos);
const QString DebugOpt("debug");
const QString DebugShortOpt("d");

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
        "Topic to publish.",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(topicOpt);

    QCommandLineOption topicIdOpt(
        QStringList() << TopicIdShortOpt << TopicIdOpt,
        "Predefined topic ID to publish.",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(topicIdOpt);

    QCommandLineOption qosOpt(
        QStringList() << QosShortOpt << QosOpt,
        "Quality of service to publish with. Defaults to " + DefaultQosStr + '.',
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(qosOpt);

    QCommandLineOption retainOpt(
        QStringList() << RetainShortOpt << RetainOpt,
        "Published message must be retained."
    );
    parser.addOption(retainOpt);

    QCommandLineOption msgOpt(
        QStringList() << MessageShortOpt << MessageOpt,
        "Message to publish. Treated as ascii string, binary data can be provided using \\x prefix prior to each byte.",
        QCoreApplication::translate("main", "value")
    );
    parser.addOption(msgOpt);

    QCommandLineOption debugOpt(
        QStringList() << DebugShortOpt << DebugOpt,
        "Enable debug output."
    );
    parser.addOption(debugOpt);
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

    clientId = "mqttsn_pub_";
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

std::uint16_t getTopicId(const QCommandLineParser& parser)
{
    std::uint16_t topicId = 0;
    do {
        auto topicIdStr = parser.value(TopicIdOpt);
        if (topicIdStr.isEmpty()) {
            break;
        }

        bool ok = false;
        auto topicIdTmp = topicIdStr.toUInt(&ok);
        if ((!ok) || (topicIdTmp == 0) || (0xffff <= topicIdTmp)) {
            break;
        }

        topicId = static_cast<decltype(topicId)>(topicIdTmp);
    } while (false);
    return topicId;
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
        if (!ok) {
            break;
        }

        value = std::max(-1, std::min(2, valueTmp));
    } while (false);
    return static_cast<MqttsnQoS>(value);
}

std::vector<std::uint8_t> getMessage(const QCommandLineParser& parser)
{
    auto msgStr = parser.value(MessageOpt);
    std::vector<std::uint8_t> result;
    result.reserve(msgStr.size());
    int idx = 0;
    while (idx < msgStr.size()) {
        int remSize = msgStr.size() - idx;
        if ((2 <= remSize) &&
            (msgStr[idx] == '\\') &&
            (msgStr[idx + 1] == '\\')) {
            result.push_back(static_cast<std::uint8_t>('\\'));
            idx += 2;
            continue;
        }

        if ((4 <= remSize) &&
            (msgStr[idx] == '\\') &&
            (msgStr[idx + 1] == 'x')) {
            auto hexNumStr = msgStr.mid(idx + 2, 2);
            bool ok = false;
            auto byteVal = msgStr.toUInt(&ok, 16);
            if (!ok) {
                result.push_back(static_cast<std::uint8_t>(msgStr[idx].toLatin1()));
                ++idx;
                continue;
            }

            result.push_back(static_cast<std::uint8_t>(byteVal));
            idx += 4;
            continue;
        }

        result.push_back(static_cast<std::uint8_t>(msgStr[idx].toLatin1()));
        ++idx;
    }

    return result;
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

    cc_mqttsn_client::app::sub::udp::Pub pub;

    pub.setGwAddr(gwAddr);
    pub.setGwPort(gwPort);
    pub.setGwId(gwId);
    pub.setLocalPort(port);
    pub.setClientId(getClientId(parser).toStdString());
    pub.setKeepAlive(keepAlive);
    pub.setCleanSession(!parser.isSet(NoCleanOpt));
    pub.setTopic(parser.value(TopicOpt).toStdString());
    pub.setTopicId(getTopicId(parser));
    pub.setQos(getQos(parser));
    pub.setRetain(parser.isSet(RetainOpt));
    pub.setMessage(getMessage(parser));
    pub.setDebug(parser.isSet(DebugOpt));

    if (!pub.start()) {
        std::cerr << "ERROR: Failed to start" << std::endl;
        return -1;
    }

    return app.exec();
}



