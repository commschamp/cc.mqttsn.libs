//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#pragma once

#include <memory>
#include <type_traits>
#include <string>
#include <list>

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>

#include "client.h"

namespace cc_mqttsn_client
{

namespace app
{

namespace sub
{

namespace udp
{

class Sub : public QObject
{
    Q_OBJECT
public:
    Sub();
    ~Sub();

    void setGwAddr(const QString& value)
    {
        m_gwAddr = value;
    }

    void setGwPort(unsigned short value)
    {
        m_gwPort = value;
    }

    void setGwId(int value)
    {
        m_gwId = value;
    }

    void setLocalPort(unsigned short value)
    {
        m_localPort = value;
    }

    void setClientId(const std::string& value)
    {
        m_clientId = value;
    }

    void setKeepAlive(unsigned short value)
    {
        m_keepAlive = value;
    }

    void setCleanSession(bool value)
    {
        m_cleanSession = value;
    }

    typedef std::list<std::string> TopicsList;
    void setTopics(const TopicsList& topics)
    {
        m_topics = topics;
    }

    typedef std::list<std::uint16_t> TopicIdsList;
    void setTopicIds(const TopicIdsList& topicIds)
    {
        m_topicIds = topicIds;
    }

    void setQos(CC_MqttsnQoS value)
    {
        m_qos = value;
    }

    void setVerbose(bool value)
    {
        m_verbose = value;
    }

    void setNoRetain(bool value)
    {
        m_noRetain = value;
    }

    void setHexOutput(bool value)
    {
        m_hexOutput = value;
    }

    bool start();

private slots:
    void tick();
    void readFromSocket();
    void socketErrorOccurred(QAbstractSocket::SocketError err);

private:
    void nextTickProgram(unsigned ms);
    static void nextTickProgramCb(void* obj, unsigned ms);

    unsigned cancelTick();
    static unsigned caneclTickCb(void* obj);

    void sendData(const unsigned char* buf, unsigned bufLen, bool broadcast);
    static void sendDataCb(void* obj, const unsigned char* buf, unsigned bufLen, bool broadcast);

    void gwStatusReport(unsigned short gwId, CC_MqttsnGwStatus status);
    static void gwStatusReportCb(void* obj, unsigned char gwId, CC_MqttsnGwStatus status);

    void gwDisconnectReport();
    static void gwDisconnectReportCb(void* obj);

    void messageReport(const CC_MqttsnMessageInfo* msgInfo);
    static void messageReportCb(void* obj, const CC_MqttsnMessageInfo* msgInfo);

    void doConnect(bool reconnecting = false);
    void doSubscribe();
    void connectComplete(CC_MqttsnAsyncOpStatus status);
    static void connectCompleteCb(void* obj, CC_MqttsnAsyncOpStatus status);
    void subscribeComplete(CC_MqttsnAsyncOpStatus status);
    static void subscribeCompleteCb(void* obj, CC_MqttsnAsyncOpStatus status, CC_MqttsnQoS qos);
    bool bindLocalPort();
    bool openSocket();
    bool connectToGw();
    void broadcastData(const unsigned char* buf, unsigned bufLen);
    void sendDataConnected(const unsigned char* buf, unsigned bufLen);

    CC_MqttsnClientHandle m_client;
    QTimer m_timer;
    unsigned m_reqTimeout = 0;
    QString m_gwAddr;
    unsigned short m_gwPort = 0;
    int m_gwId = -1;
    unsigned short m_localPort = 0;
    QUdpSocket m_socket;
    QHostAddress m_lastSenderAddress;
    quint16 m_lastSenderPort;
    std::string m_clientId;
    unsigned short m_keepAlive = 0;
    bool m_cleanSession = true;
    bool m_verbose = false;
    bool m_noRetain = false;
    bool m_hexOutput = false;
    TopicsList m_topics;
    TopicIdsList m_topicIds;
    CC_MqttsnQoS m_qos = CC_MqttsnQoS_ExactlyOnceDelivery;
};

}  // namespace udp

}  // namespace sub

}  // namespace app

}  // namespace cc_mqttsn_client


