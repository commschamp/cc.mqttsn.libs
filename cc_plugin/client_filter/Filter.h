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


#pragma once

#include <memory>
#include <type_traits>
#include <string>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtCore/QList>
#include <QtCore/QTimer>
CC_ENABLE_WARNINGS()

#include "comms_champion/Filter.h"

#include "client.h"

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

class Filter : public comms_champion::Filter
{
    Q_OBJECT

public:
    typedef comms_champion::DataInfoPtr DataInfoPtr;

    struct PubSubInfo
    {
        QString m_topic;
        MqttsnTopicId m_topicId = 0;
        MqttsnQoS m_qos = MqttsnQoS_AtMostOnceDelivery;
        bool m_retain = false;
    };

    typedef QList<PubSubInfo> SubInfosList;
    typedef unsigned short KeepAliveType;

    Filter();
    ~Filter();

    std::string& clientId()
    {
        return m_clientId;
    }

    int& retryPeriod()
    {
        return m_retryPeriod;
    }

    int& retryCount()
    {
        return m_retryCount;
    }

    KeepAliveType& keepAlivePeriod()
    {
        return m_keepAlivePeriod;
    }

    PubSubInfo& defaultPubInfo()
    {
        return m_pub;
    }

    QList<PubSubInfo>& subInfos()
    {
        return m_subs;
    }


protected:
    virtual bool startImpl() override;
    virtual void stopImpl() override;
    virtual QList<DataInfoPtr> recvDataImpl(DataInfoPtr dataPtr) override;
    virtual QList<DataInfoPtr> sendDataImpl(DataInfoPtr dataPtr) override;

private slots:
    void tick();
    void startClient();

private:
    struct MqttsnClientDeleter
    {
        void operator()(MqttsnClientHandle client)
        {
            mqttsn_client_free(client);
        }
    };

    typedef std::unique_ptr<
        typename std::remove_pointer<MqttsnClientHandle>::type,
        MqttsnClientDeleter
    > ClientPtr;

    void doPublish();
    void publishComplete(MqttsnAsyncOpStatus status);
    void doConnect();
    void connectComplete(MqttsnAsyncOpStatus status);
    void sendAccumulatedMessages();
    void doSubscribe();
    void subscribeComplete(MqttsnAsyncOpStatus status, MqttsnQoS qos);

    void reportReceivedMessage(const MqttsnMessageInfo& msgInfo);
    void sendMessage(const unsigned char* buf, unsigned bufLen, bool broadcast);
    void programNextTick(unsigned duration);
    unsigned cancelTick();
    void gwStatusReport(unsigned short gwId, MqttsnGwStatus status);
    void gwDisconnectReport();

    static void messageArrivedCb(void* data, const MqttsnMessageInfo* msgInfo);
    static void sendMessageCb(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast);
    static void programNextTickCb(void* data, unsigned duration);
    static unsigned cancelTickCb(void* data);
    static void gwStatusReportCb(void* data, unsigned char gwId, MqttsnGwStatus status);
    static void gwDisconnectReportCb(void* data);
    static void connectCompleteCb(void* data, MqttsnAsyncOpStatus status);
    static void publishCompleteCb(void* data, MqttsnAsyncOpStatus status);
    static void subscribeCompleteCb(void* data, MqttsnAsyncOpStatus status, MqttsnQoS qos);

    ClientPtr m_client;
    std::string m_clientId = "cc_filter";
    int m_retryPeriod = 5;
    int m_retryCount = 3;
    KeepAliveType m_keepAlivePeriod = 60;
    PubSubInfo m_pub;
    SubInfosList m_subs;
    int m_completedSubsCount = 0;
    QString m_broadcastPropertyName = "broadcast";
    QString m_topicPropertyName = "topic";
    QString m_topicIdPropertyName = "topic_id";
    QString m_qosPropertyName = "qos";
    QString m_retainPropertyName = "retain";
    QList<DataInfoPtr> m_readData;
    QList<DataInfoPtr> m_sendData;
    QList<DataInfoPtr> m_pendingPubs;
    DataInfoPtr m_input;
    QTimer m_tickTimer;
    unsigned m_tickDuration = 0;
    bool m_readSendInProgress = false;
    std::string m_topicHolder;
    bool m_connected = false;
    QList<unsigned short> m_gateways;
};

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn



