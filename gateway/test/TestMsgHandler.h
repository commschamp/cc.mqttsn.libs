//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

#include <functional>

#include "comms/comms.h"
#include "mqttsn/protocol/Stack.h"
#include "mqtt311/Message.h"
#include "mqtt311/frame/Frame.h"

class TestMsgHandler;

typedef mqttsn::protocol::MessageT<
    comms::option::IdInfoInterface,
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::Handler<TestMsgHandler>,
    comms::option::LengthInfoInterface,
    comms::option::RefreshInterface
> TestMqttsnMessage;

typedef mqtt311::Message<
    comms::option::IdInfoInterface,
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::Handler<TestMsgHandler>,
    comms::option::LengthInfoInterface,
    comms::option::RefreshInterface
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
typedef mqttsn::protocol::message::Unsuback<TestMqttsnMessage> UnsubackMsg_SN;
typedef mqttsn::protocol::message::Pingreq<TestMqttsnMessage> PingreqMsg_SN;
typedef mqttsn::protocol::message::Pingresp<TestMqttsnMessage> PingrespMsg_SN;
typedef mqttsn::protocol::message::Disconnect<TestMqttsnMessage> DisconnectMsg_SN;
typedef mqttsn::protocol::message::Willtopicupd<TestMqttsnMessage> WilltopicupdMsg_SN;
typedef mqttsn::protocol::message::Willtopicresp<TestMqttsnMessage> WilltopicrespMsg_SN;
typedef mqttsn::protocol::message::Willmsgupd<TestMqttsnMessage> WillmsgupdMsg_SN;
typedef mqttsn::protocol::message::Willmsgresp<TestMqttsnMessage> WillmsgrespMsg_SN;
typedef mqttsn::protocol::Stack<TestMqttsnMessage> TestMqttsnProtStack;

typedef mqtt311::message::Connect<TestMqttMessage> ConnectMsg;
typedef mqtt311::message::Connack<TestMqttMessage> ConnackMsg;
typedef mqtt311::message::Publish<TestMqttMessage> PublishMsg;
typedef mqtt311::message::Puback<TestMqttMessage> PubackMsg;
typedef mqtt311::message::Pubrec<TestMqttMessage> PubrecMsg;
typedef mqtt311::message::Pubrel<TestMqttMessage> PubrelMsg;
typedef mqtt311::message::Pubcomp<TestMqttMessage> PubcompMsg;
typedef mqtt311::message::Subscribe<TestMqttMessage> SubscribeMsg;
typedef mqtt311::message::Suback<TestMqttMessage> SubackMsg;
typedef mqtt311::message::Unsubscribe<TestMqttMessage> UnsubscribeMsg;
typedef mqtt311::message::Unsuback<TestMqttMessage> UnsubackMsg;
typedef mqtt311::message::Pingreq<TestMqttMessage> PingreqMsg;
typedef mqtt311::message::Pingresp<TestMqttMessage> PingrespMsg;
typedef mqtt311::message::Disconnect<TestMqttMessage> DisconnectMsg;
typedef mqtt311::frame::Frame<TestMqttMessage> TestMqttProtStack;

using TestMqttsnMsgHandler = comms::GenericHandler<
    TestMqttsnMessage,
    mqttsn::protocol::AllMessages<TestMqttsnMessage>
>;

using TestMqttMsgHandler = comms::GenericHandler<
    TestMqttMessage,
    mqtt311::input::AllMessages<TestMqttMessage>
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

    typedef std::function<void (const PublishMsg_SN&)> PublishSnMsgHandlerFunc;
    PublishSnMsgHandlerFunc setPublishSnMsgHandler(PublishSnMsgHandlerFunc&& func);

    typedef std::function<void (const PubackMsg_SN&)> PubackSnMsgHandlerFunc;
    PubackSnMsgHandlerFunc setPubackSnMsgHandler(PubackSnMsgHandlerFunc&& func);

    typedef std::function<void (const PubrecMsg_SN&)> PubrecSnMsgHandlerFunc;
    PubrecSnMsgHandlerFunc setPubrecSnMsgHandler(PubrecSnMsgHandlerFunc&& func);

    typedef std::function<void (const PubrelMsg_SN&)> PubrelSnMsgHandlerFunc;
    PubrelSnMsgHandlerFunc setPubrelSnMsgHandler(PubrelSnMsgHandlerFunc&& func);

    typedef std::function<void (const PubcompMsg_SN&)> PubcompSnMsgHandlerFunc;
    PubcompSnMsgHandlerFunc setPubcompSnMsgHandler(PubcompSnMsgHandlerFunc&& func);

    typedef std::function<void (const PingreqMsg_SN&)> PingreqSnMsgHandlerFunc;
    PingreqSnMsgHandlerFunc setPingreqSnMsgHandler(PingreqSnMsgHandlerFunc&& func);

    typedef std::function<void (const PingrespMsg_SN&)> PingrespSnMsgHandlerFunc;
    PingrespSnMsgHandlerFunc setPingrespSnMsgHandler(PingrespSnMsgHandlerFunc&& func);

    typedef std::function<void (const SubackMsg_SN&)> SubackSnMsgHandlerFunc;
    SubackSnMsgHandlerFunc setSubackSnMsgHandler(SubackSnMsgHandlerFunc&& func);

    typedef std::function<void (const UnsubackMsg_SN&)> UnsubackSnMsgHandlerFunc;
    UnsubackSnMsgHandlerFunc setUnsubackSnMsgHandler(UnsubackSnMsgHandlerFunc&& func);

    typedef std::function<void (const WilltopicrespMsg_SN&)> WilltopicrespMsgHandlerFunc;
    WilltopicrespMsgHandlerFunc setWilltopicrespMsgHandler(WilltopicrespMsgHandlerFunc&& func);

    typedef std::function<void (const WillmsgrespMsg_SN&)> WillmsgrespMsgHandlerFunc;
    WillmsgrespMsgHandlerFunc setWillmsgrespMsgHandler(WillmsgrespMsgHandlerFunc&& func);


    typedef std::function<void (const ConnectMsg&)> ConnectMsgHandlerFunc;
    ConnectMsgHandlerFunc setConnectMsgHandler(ConnectMsgHandlerFunc&& func);

    typedef std::function<void (const DisconnectMsg&)> DisconnectMsgHandlerFunc;
    DisconnectMsgHandlerFunc setDisconnectMsgHandler(DisconnectMsgHandlerFunc&& func);

    typedef std::function<void (const PingreqMsg&)> PingreqMsgHandlerFunc;
    PingreqMsgHandlerFunc setPingreqMsgHandler(PingreqMsgHandlerFunc&& func);

    typedef std::function<void (const PingrespMsg&)> PingrespMsgHandlerFunc;
    PingrespMsgHandlerFunc setPingrespMsgHandler(PingrespMsgHandlerFunc&& func);

    typedef std::function<void (const PublishMsg&)> PublishMsgHandlerFunc;
    PublishMsgHandlerFunc setPublishMsgHandler(PublishMsgHandlerFunc&& func);

    typedef std::function<void (const PubackMsg&)> PubackMsgHandlerFunc;
    PubackMsgHandlerFunc setPubackMsgHandler(PubackMsgHandlerFunc&& func);

    typedef std::function<void (const PubrecMsg&)> PubrecMsgHandlerFunc;
    PubrecMsgHandlerFunc setPubrecMsgHandler(PubrecMsgHandlerFunc&& func);

    typedef std::function<void (const PubrelMsg&)> PubrelMsgHandlerFunc;
    PubrelMsgHandlerFunc setPubrelMsgHandler(PubrelMsgHandlerFunc&& func);

    typedef std::function<void (const PubcompMsg&)> PubcompMsgHandlerFunc;
    PubcompMsgHandlerFunc setPubcompMsgHandler(PubcompMsgHandlerFunc&& func);

    typedef std::function<void (const SubscribeMsg&)> SubscribeMsgHandlerFunc;
    SubscribeMsgHandlerFunc setSubscribeMsgHandler(SubscribeMsgHandlerFunc&& func);

    typedef std::function<void (const UnsubscribeMsg&)> UnsubscribeMsgHandlerFunc;
    UnsubscribeMsgHandlerFunc setUnsubscribeMsgHandler(UnsubscribeMsgHandlerFunc&& func);

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
    virtual void handle(PubackMsg_SN& msg) override;
    virtual void handle(PubrecMsg_SN& msg) override;
    virtual void handle(PubrelMsg_SN& msg) override;
    virtual void handle(PubcompMsg_SN& msg) override;
    virtual void handle(PingreqMsg_SN& msg) override;
    virtual void handle(PingrespMsg_SN& msg) override;
    virtual void handle(SubackMsg_SN& msg) override;
    virtual void handle(UnsubackMsg_SN& msg) override;
    virtual void handle(WilltopicrespMsg_SN& msg) override;
    virtual void handle(WillmsgrespMsg_SN& msg) override;
    virtual void handle(TestMqttsnMessage& msg) override;

    virtual void handle(ConnectMsg& msg) override;
    virtual void handle(DisconnectMsg& msg) override;
    virtual void handle(PingreqMsg& msg) override;
    virtual void handle(PingrespMsg& msg) override;
    virtual void handle(PublishMsg& msg) override;
    virtual void handle(PubackMsg& msg) override;
    virtual void handle(PubrecMsg& msg) override;
    virtual void handle(PubrelMsg& msg) override;
    virtual void handle(PubcompMsg& msg) override;
    virtual void handle(SubscribeMsg& msg) override;
    virtual void handle(UnsubscribeMsg& msg) override;
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
    DataBuf prepareClientPublish(
        const DataBuf& data,
        std::uint16_t topicId,
        std::uint16_t msgId,
        mqttsn::protocol::field::TopicIdTypeVal topicIdType,
        mqttsn::protocol::field::QosType qos,
        bool retain,
        bool dup);
    DataBuf prepareClientPuback(
        std::uint16_t topicId,
        std::uint16_t msgId,
        mqttsn::protocol::field::ReturnCodeVal rc);
    DataBuf prepareClientPubrec(std::uint16_t msgId);
    DataBuf prepareClientPubrel(std::uint16_t msgId);
    DataBuf prepareClientPubcomp(std::uint16_t msgId);
    DataBuf prepareClientPingreq(const std::string& clientId = std::string());
    DataBuf prepareClientPingresp();
    DataBuf prepareClientSubscribe(
        std::uint16_t topicId,
        std::uint16_t msgId,
        mqttsn::protocol::field::QosType qos);
    DataBuf prepareClientSubscribe(
        const std::string& topic,
        std::uint16_t msgId,
        mqttsn::protocol::field::QosType qos);
    DataBuf prepareClientUnsubscribe(
        std::uint16_t topicId,
        std::uint16_t msgId);
    DataBuf prepareClientUnsubscribe(
        const std::string& topic,
        std::uint16_t msgId);
    DataBuf prepareClientWilltopicupd(
        const std::string& topic,
        mqttsn::protocol::field::QosType qos,
        bool retain);
    DataBuf prepareClientWillmsgupd(const DataBuf& data);

    using ConnackResponseCodeVal = ConnackMsg::Field_returnCode::ValueType;
    DataBuf prepareBrokerConnack(ConnackResponseCodeVal rc, bool sessionPresent = false);
    DataBuf prepareBrokerDisconnect();
    DataBuf prepareBrokerPingreq();
    DataBuf prepareBrokerPingresp();
    DataBuf prepareBrokerPublish(
        const std::string& topic,
        const DataBuf& msgData,
        std::uint16_t packetId,
        mqtt311::field::QosVal qos,
        bool retain,
        bool duplicate);
    DataBuf prepareBrokerPuback(std::uint16_t packetId);
    DataBuf prepareBrokerPubrec(std::uint16_t packetId);
    DataBuf prepareBrokerPubrel(std::uint16_t packetId);
    DataBuf prepareBrokerPubcomp(std::uint16_t packetId);

    using SubackReturnCodeVal = SubackMsg::Field_list::ValueType::value_type::ValueType;
    DataBuf prepareBrokerSuback(std::uint16_t packetId, SubackReturnCodeVal rc);
    DataBuf prepareBrokerUnsuback(std::uint16_t packetId);


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
    PublishSnMsgHandlerFunc m_publishSnMsgHandler;
    PubackSnMsgHandlerFunc m_pubackSnMsgHandler;
    PubrecSnMsgHandlerFunc m_pubrecSnMsgHandler;
    PubrelSnMsgHandlerFunc m_pubrelSnMsgHandler;
    PubcompSnMsgHandlerFunc m_pubcompSnMsgHandler;
    PingreqSnMsgHandlerFunc m_pingreqSnMsgHandler;
    PingrespSnMsgHandlerFunc m_pingrespSnMsgHandler;
    SubackSnMsgHandlerFunc m_subackSnMsgHandler;
    UnsubackSnMsgHandlerFunc m_unsubackSnMsgHandler;
    WilltopicrespMsgHandlerFunc m_willtopicrespMsgHandler;
    WillmsgrespMsgHandlerFunc m_willmsgrespMsgHandler;

    ConnectMsgHandlerFunc m_connectMsgHandler;
    DisconnectMsgHandlerFunc m_disconnectMsgHandler;
    PingreqMsgHandlerFunc m_pingreqMsgHandler;
    PingrespMsgHandlerFunc m_pingrespMsgHandler;
    PublishMsgHandlerFunc m_publishMsgHandler;
    PubackMsgHandlerFunc m_pubackMsgHandler;
    PubrecMsgHandlerFunc m_pubrecMsgHandler;
    PubrelMsgHandlerFunc m_pubrelMsgHandler;
    PubcompMsgHandlerFunc m_pubcompMsgHandler;
    SubscribeMsgHandlerFunc m_subscribeMsgHandler;
    UnsubscribeMsgHandlerFunc m_unsubscribeMsgHandler;
};
