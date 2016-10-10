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

#include "comms/comms.h"
#include "mqttsn/protocol/Stack.h"
#include "mqtt/Message.h"
#include "mqtt/Stack.h"

class TestMsgHandler;

typedef mqttsn::protocol::MessageT<
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::Handler<TestMsgHandler>
> TestMqttsnMessage;

typedef mqtt::MessageT<
    comms::option::BigEndian,
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::MsgIdType<mqtt::MsgId>,
    comms::option::Handler<TestMsgHandler>,
    comms::option::RefreshInterface,
    comms::option::ValidCheckInterface,
    comms::option::LengthInfoInterface
> TestMqttMessage;

typedef mqttsn::protocol::message::Advertise<TestMqttsnMessage> AdvertiseMsg_SN;
typedef mqttsn::protocol::message::Searchgw<TestMqttsnMessage> SearchgwMsg_SN;
typedef mqttsn::protocol::message::Gwinfo<TestMqttsnMessage> GwinfoMsg_SN;
typedef mqttsn::protocol::message::Connect<TestMqttsnMessage> ConnectMsg_SN;
typedef mqttsn::protocol::message::Connack<TestMqttsnMessage> ConnackMsg_SN;
typedef mqttsn::protocol::message::Willtopicreq<TestMqttsnMessage> WilltopicreqMsg_SN;
typedef mqttsn::protocol::message::Willtopic<TestMqttsnMessage> WilltopicMsg_SN;
typedef mqttsn::protocol::message::Willmsgreq<TestMqttsnMessage> WillmsgreqMsg_SN;
typedef mqttsn::protocol::message::Willmsg<TestMqttsnMessage> WillmsgMsg_SN;
typedef mqttsn::protocol::message::Register<TestMqttsnMessage> RegisterMsg_SN;
typedef mqttsn::protocol::message::Regack<TestMqttsnMessage> RegackMsg_SN;
typedef mqttsn::protocol::message::Publish<TestMqttsnMessage> PublishMsg_SN;
typedef mqttsn::protocol::message::Puback<TestMqttsnMessage> PubackMsg_SN;
typedef mqttsn::protocol::message::Pubrec<TestMqttsnMessage> PubrecMsg_SN;
typedef mqttsn::protocol::message::Pubrel<TestMqttsnMessage> PubrelMsg_SN;
typedef mqttsn::protocol::message::Pubcomp<TestMqttsnMessage> PubcompMsg_SN;
typedef mqttsn::protocol::message::Subscribe<TestMqttsnMessage> SubscribeMsg_SN;
typedef mqttsn::protocol::message::Suback<TestMqttsnMessage> SubackMsg_SN;
typedef mqttsn::protocol::message::Unsubscribe<TestMqttsnMessage> UnsubscribeMsg_SN;
typedef mqttsn::protocol::message::Unsuback<TestMqttsnMessage> Unsuback_SN;
typedef mqttsn::protocol::message::Pingreq<TestMqttsnMessage> PingreqMsg_SN;
typedef mqttsn::protocol::message::Pingresp<TestMqttsnMessage> PingrespMsg_SN;
typedef mqttsn::protocol::message::Disconnect<TestMqttsnMessage> DisconnectMsg_SN;
typedef mqttsn::protocol::message::Willtopicupd<TestMqttsnMessage> Willtopicupd_SN;
typedef mqttsn::protocol::message::Willtopicresp<TestMqttsnMessage> Willtopicresp_SN;
typedef mqttsn::protocol::message::Willmsgupd<TestMqttsnMessage> Willmsgupd_SN;
typedef mqttsn::protocol::message::Willmsgresp<TestMqttsnMessage> Willmsgresp_SN;
typedef mqttsn::protocol::Stack<TestMqttsnMessage> TestMqttsnProtStack;

typedef mqtt::message::Connect<TestMqttMessage> ConnectMsg;
typedef mqtt::message::Connack<TestMqttMessage> ConnackMsg;
typedef mqtt::message::Publish<TestMqttMessage> PublishMsg;
typedef mqtt::message::Puback<TestMqttMessage> PubackMsg;
typedef mqtt::message::Pubrec<TestMqttMessage> PubrecMsg;
typedef mqtt::message::Pubrel<TestMqttMessage> PubrelMsg;
typedef mqtt::message::Pubcomp<TestMqttMessage> PubcompMsg;
typedef mqtt::message::Subscribe<TestMqttMessage> SubscribeMsg;
typedef mqtt::message::Suback<TestMqttMessage> SubackMsg;
typedef mqtt::message::Unsubscribe<TestMqttMessage> UnsubscribeMsg;
typedef mqtt::message::Unsuback<TestMqttMessage> UnsubackMsg;
typedef mqtt::message::Pingreq<TestMqttMessage> PingreqMsg;
typedef mqtt::message::Pingresp<TestMqttMessage> PingrespMsg;
typedef mqtt::message::Disconnect<TestMqttMessage> DisconnectMsg;
typedef mqtt::Stack<TestMqttMessage> TestMqttProtStack;

using TestMqttsnMsgHandler = comms::GenericHandler<
    TestMqttsnMessage,
    mqttsn::protocol::AllMessages<TestMqttsnMessage>
>;

using TestMqttMsgHandler = comms::GenericHandler<
    TestMqttMessage,
    mqtt::AllMessages<TestMqttMessage>
>;


class TestMsgHandler : public TestMqttsnMsgHandler, public TestMqttMsgHandler
{
    typedef TestMqttsnMsgHandler MqttsnBase;
    typedef TestMqttMsgHandler MqttBase;
public:
    typedef std::vector<std::uint8_t> DataBuf;

    TestMsgHandler();
    ~TestMsgHandler();

    typedef std::function<void (const GwinfoMsg_SN& msg)> GwinfoMsgHandlerFunc;
    GwinfoMsgHandlerFunc setGwinfoMsgHandler(GwinfoMsgHandlerFunc&& func);

    typedef std::function<void (const ConnackMsg_SN&)> ConnackMsgHandlerFunc;
    ConnackMsgHandlerFunc setConnackMsgHandler(ConnackMsgHandlerFunc&& func);

    typedef std::function<void (const WilltopicreqMsg_SN&)> WilltopicreqMsgHandlerFunc;
    WilltopicreqMsgHandlerFunc setWilltopicreqMsgHandler(WilltopicreqMsgHandlerFunc&& func);

    typedef std::function<void (const WillmsgreqMsg_SN&)> WillmsgreqMsgHandlerFunc;
    WillmsgreqMsgHandlerFunc setWillmsgreqMsgHandler(WillmsgreqMsgHandlerFunc&& func);

    typedef std::function<void (const DisconnectMsg_SN&)> DisconnectSnMsgHandlerFunc;
    DisconnectSnMsgHandlerFunc setDisconnectSnMsgHandler(DisconnectSnMsgHandlerFunc&& func);

    typedef std::function<void (const RegisterMsg_SN&)> RegisterMsgHandlerFunc;
    RegisterMsgHandlerFunc setRegisterMsgHandler(RegisterMsgHandlerFunc&& func);

    typedef std::function<void (const RegackMsg_SN&)> RegackMsgHandlerFunc;
    RegackMsgHandlerFunc setRegackMsgHandler(RegackMsgHandlerFunc&& func);

    typedef std::function<void (const PublishMsg_SN&)> PublishMsgHandlerFunc;
    PublishMsgHandlerFunc setPublishMsgHandler(PublishMsgHandlerFunc&& func);

    typedef std::function<void (const PubrelMsg_SN&)> PubrelMsgHandlerFunc;
    PubrelMsgHandlerFunc setPubrelMsgHandler(PubrelMsgHandlerFunc&& func);

    typedef std::function<void (const ConnectMsg&)> ConnectMsgHandlerFunc;
    ConnectMsgHandlerFunc setConnectMsgHandler(ConnectMsgHandlerFunc&& func);

    typedef std::function<void (const DisconnectMsg&)> DisconnectMsgHandlerFunc;
    DisconnectMsgHandlerFunc setDisconnectMsgHandler(DisconnectMsgHandlerFunc&& func);

    typedef std::function<void (const PingreqMsg&)> PingreqMsgHandlerFunc;
    PingreqMsgHandlerFunc setPingreqMsgHandler(PingreqMsgHandlerFunc&& func);

    typedef std::function<void (const PubackMsg&)> PubackMsgHandlerFunc;
    PubackMsgHandlerFunc setPubackMsgHandler(PubackMsgHandlerFunc&& func);

    typedef std::function<void (const PubrecMsg&)> PubrecMsgHandlerFunc;
    PubrecMsgHandlerFunc setPubrecMsgHandler(PubrecMsgHandlerFunc&& func);

    typedef std::function<void (const PubcompMsg&)> PubcompMsgHandlerFunc;
    PubcompMsgHandlerFunc setPubcompMsgHandler(PubcompMsgHandlerFunc&& func);

    using MqttsnBase::handle;
    using MqttBase::handle;

    virtual void handle(GwinfoMsg_SN& msg) override;
    virtual void handle(ConnackMsg_SN& msg) override;
    virtual void handle(WilltopicreqMsg_SN& msg) override;
    virtual void handle(WillmsgreqMsg_SN& msg) override;
    virtual void handle(DisconnectMsg_SN& msg) override;
    virtual void handle(RegisterMsg_SN& msg) override;
    virtual void handle(RegackMsg_SN& msg) override;
    virtual void handle(PublishMsg_SN& msg) override;
    virtual void handle(PubrelMsg_SN& msg) override;
    virtual void handle(TestMqttsnMessage& msg) override;

    virtual void handle(ConnectMsg& msg) override;
    virtual void handle(DisconnectMsg& msg) override;
    virtual void handle(PingreqMsg& msg) override;
    virtual void handle(PubackMsg& msg) override;
    virtual void handle(PubrecMsg& msg) override;
    virtual void handle(PubcompMsg& msg) override;
    virtual void handle(TestMqttMessage& msg) override;

    void processDataForClient(const DataBuf& data);
    void processDataForBroker(const DataBuf& data);

    DataBuf prepareInput(const TestMqttsnMessage& msg);
    DataBuf prepareInput(const TestMqttMessage& msg);

    DataBuf prepareSearchgw(std::uint8_t radius = 0);
    DataBuf prepareClientConnect(const std::string& id, std::uint16_t keepAlive, bool hasWill, bool clean);
    DataBuf prepareClientWilltopic(
        const std::string& topic,
        mqttsn::protocol::field::QosType qos,
        bool retain);
    DataBuf prepareClientWillmsg(const DataBuf& data);
    DataBuf prepareClientDisconnect(std::uint16_t duration = 0);
    DataBuf prepareClientRegister(const std::string& topic, std::uint16_t msgId);
    DataBuf prepareClientRegack(
        std::uint16_t topicId,
        std::uint16_t msgId,
        mqttsn::protocol::field::ReturnCodeVal rc);
    DataBuf prepareClientPuback(
        std::uint16_t topicId,
        std::uint16_t msgId,
        mqttsn::protocol::field::ReturnCodeVal rc);
    DataBuf prepareClientPubrec(std::uint16_t msgId);
    DataBuf prepareClientPubcomp(std::uint16_t msgId);

    DataBuf prepareBrokerConnack(mqtt::message::ConnackResponseCode rc, bool sessionPresent = false);
    DataBuf prepareBrokerDisconnect();
    DataBuf prepareBrokerPingresp();
    DataBuf prepareBrokerPublish(
        const std::string& topic,
        const DataBuf& msgData,
        std::uint16_t packetId,
        mqtt::field::QosType qos,
        bool retain,
        bool duplicate);

    DataBuf prepareBrokerPubrel(std::uint16_t packetId);


private:
    template <typename TStack>
    void processOutputInternal(TStack& stack, const DataBuf& data);

    template <typename TStack, typename TMsg>
    static DataBuf prepareInputInternal(TStack& stack, const TMsg& msg);

    TestMqttsnProtStack m_mqttsnStack;
    TestMqttProtStack m_mqttStack;

    GwinfoMsgHandlerFunc m_gwInfoMsgHandler;
    ConnackMsgHandlerFunc m_connackMsgHandler;
    WilltopicreqMsgHandlerFunc m_willtopicreqMsgHandler;
    WillmsgreqMsgHandlerFunc m_willmsgreqMsgHandler;
    DisconnectSnMsgHandlerFunc m_disconnectSnMsgHandler;
    RegisterMsgHandlerFunc m_registerMsgHandler;
    RegackMsgHandlerFunc m_regackMsgHandler;
    PublishMsgHandlerFunc m_publishMsgHandler;
    PubrelMsgHandlerFunc m_pubrelMsgHandler;

    ConnectMsgHandlerFunc m_connectMsgHandler;
    DisconnectMsgHandlerFunc m_disconnectMsgHandler;
    PingreqMsgHandlerFunc m_pingreqMsgHandler;
    PubackMsgHandlerFunc m_pubackMsgHandler;
    PubrecMsgHandlerFunc m_pubrecMsgHandler;
    PubcompMsgHandlerFunc m_pubcompMsgHandler;
};
