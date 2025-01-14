//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "comms/comms.h"
#include "cc_mqttsn/Message.h"
#include "cc_mqttsn/input/AllMessages.h"
#include "cc_mqttsn/frame/Frame.h"
#include "cc_mqtt311/Message.h"
#include "cc_mqtt311/input/AllMessages.h"
#include "cc_mqtt311/frame/Frame.h"

class TestMsgHandler;

using TestMqttsnMessage =
    cc_mqttsn::Message<
        comms::option::IdInfoInterface,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>,
        comms::option::Handler<TestMsgHandler>,
        comms::option::LengthInfoInterface,
        comms::option::RefreshInterface
    >;

using TestMqttMessage =
    cc_mqtt311::Message<
        comms::option::IdInfoInterface,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>,
        comms::option::Handler<TestMsgHandler>,
        comms::option::LengthInfoInterface,
        comms::option::RefreshInterface
    >;

CC_MQTTSN_ALIASES_FOR_ALL_MESSAGES_DEFAULT_OPTIONS(, Msg_SN, TestMqttsnMessage)    
CC_MQTT311_ALIASES_FOR_ALL_MESSAGES_DEFAULT_OPTIONS(, Msg, TestMqttMessage)   

using TestMqttsnFrame = cc_mqttsn::frame::Frame<TestMqttsnMessage>;
using TestMqttFrame = cc_mqtt311::frame::Frame<TestMqttMessage>;

using TestMqttsnMsgHandler = comms::GenericHandler<
    TestMqttsnMessage,
    cc_mqttsn::input::AllMessages<TestMqttsnMessage>
>;

using TestMqttMsgHandler = comms::GenericHandler<
    TestMqttMessage,
    cc_mqtt311::input::AllMessages<TestMqttMessage>
>;


class TestMsgHandler : public TestMqttsnMsgHandler, public TestMqttMsgHandler
{
    using MqttsnBase = TestMqttsnMsgHandler;
    using MqttBase = TestMqttMsgHandler;
public:
    using DataBuf = std::vector<std::uint8_t>;

    TestMsgHandler();
    ~TestMsgHandler();

    using GwinfoMsgHandlerFunc = std::function<void (const GwinfoMsg_SN& msg)>;
    GwinfoMsgHandlerFunc setGwinfoMsgHandler(GwinfoMsgHandlerFunc&& func);

    using ConnackMsgHandlerFunc = std::function<void (const ConnackMsg_SN&)>;
    ConnackMsgHandlerFunc setConnackMsgHandler(ConnackMsgHandlerFunc&& func);

    using WilltopicreqMsgHandlerFunc = std::function<void (const WilltopicreqMsg_SN&)>;
    WilltopicreqMsgHandlerFunc setWilltopicreqMsgHandler(WilltopicreqMsgHandlerFunc&& func);

    using WillmsgreqMsgHandlerFunc = std::function<void (const WillmsgreqMsg_SN&)>;
    WillmsgreqMsgHandlerFunc setWillmsgreqMsgHandler(WillmsgreqMsgHandlerFunc&& func);

    using DisconnectSnMsgHandlerFunc = std::function<void (const DisconnectMsg_SN&)>;
    DisconnectSnMsgHandlerFunc setDisconnectSnMsgHandler(DisconnectSnMsgHandlerFunc&& func);

    using RegisterMsgHandlerFunc = std::function<void (const RegisterMsg_SN&)>;
    RegisterMsgHandlerFunc setRegisterMsgHandler(RegisterMsgHandlerFunc&& func);

    using RegackMsgHandlerFunc = std::function<void (const RegackMsg_SN&)>;
    RegackMsgHandlerFunc setRegackMsgHandler(RegackMsgHandlerFunc&& func);

    using PublishSnMsgHandlerFunc = std::function<void (const PublishMsg_SN&)>;
    PublishSnMsgHandlerFunc setPublishSnMsgHandler(PublishSnMsgHandlerFunc&& func);

    using PubackSnMsgHandlerFunc = std::function<void (const PubackMsg_SN&)>;
    PubackSnMsgHandlerFunc setPubackSnMsgHandler(PubackSnMsgHandlerFunc&& func);

    using PubrecSnMsgHandlerFunc = std::function<void (const PubrecMsg_SN&)>;
    PubrecSnMsgHandlerFunc setPubrecSnMsgHandler(PubrecSnMsgHandlerFunc&& func);

    using PubrelSnMsgHandlerFunc = std::function<void (const PubrelMsg_SN&)>;
    PubrelSnMsgHandlerFunc setPubrelSnMsgHandler(PubrelSnMsgHandlerFunc&& func);

    using PubcompSnMsgHandlerFunc = std::function<void (const PubcompMsg_SN&)>;
    PubcompSnMsgHandlerFunc setPubcompSnMsgHandler(PubcompSnMsgHandlerFunc&& func);

    using PingreqSnMsgHandlerFunc = std::function<void (const PingreqMsg_SN&)>;
    PingreqSnMsgHandlerFunc setPingreqSnMsgHandler(PingreqSnMsgHandlerFunc&& func);

    using PingrespSnMsgHandlerFunc = std::function<void (const PingrespMsg_SN&)>;
    PingrespSnMsgHandlerFunc setPingrespSnMsgHandler(PingrespSnMsgHandlerFunc&& func);

    using SubackSnMsgHandlerFunc = std::function<void (const SubackMsg_SN&)>;
    SubackSnMsgHandlerFunc setSubackSnMsgHandler(SubackSnMsgHandlerFunc&& func);

    using UnsubackSnMsgHandlerFunc = std::function<void (const UnsubackMsg_SN&)>;
    UnsubackSnMsgHandlerFunc setUnsubackSnMsgHandler(UnsubackSnMsgHandlerFunc&& func);

    using WilltopicrespMsgHandlerFunc = std::function<void (const WilltopicrespMsg_SN&)>;
    WilltopicrespMsgHandlerFunc setWilltopicrespMsgHandler(WilltopicrespMsgHandlerFunc&& func);

    using WillmsgrespMsgHandlerFunc = std::function<void (const WillmsgrespMsg_SN&)>;
    WillmsgrespMsgHandlerFunc setWillmsgrespMsgHandler(WillmsgrespMsgHandlerFunc&& func);

    using FwdMsgHandlerFunc = std::function<void (const FwdMsg_SN&)>;
    FwdMsgHandlerFunc setFwdMsgHandler(FwdMsgHandlerFunc&& func);

    using ConnectMsgHandlerFunc = std::function<void (const ConnectMsg&)>;
    ConnectMsgHandlerFunc setConnectMsgHandler(ConnectMsgHandlerFunc&& func);

    using DisconnectMsgHandlerFunc = std::function<void (const DisconnectMsg&)>;
    DisconnectMsgHandlerFunc setDisconnectMsgHandler(DisconnectMsgHandlerFunc&& func);

    using PingreqMsgHandlerFunc = std::function<void (const PingreqMsg&)>;
    PingreqMsgHandlerFunc setPingreqMsgHandler(PingreqMsgHandlerFunc&& func);

    using PingrespMsgHandlerFunc = std::function<void (const PingrespMsg&)>;
    PingrespMsgHandlerFunc setPingrespMsgHandler(PingrespMsgHandlerFunc&& func);

    using PublishMsgHandlerFunc = std::function<void (const PublishMsg&)>;
    PublishMsgHandlerFunc setPublishMsgHandler(PublishMsgHandlerFunc&& func);

    using PubackMsgHandlerFunc = std::function<void (const PubackMsg&)>;
    PubackMsgHandlerFunc setPubackMsgHandler(PubackMsgHandlerFunc&& func);

    using PubrecMsgHandlerFunc = std::function<void (const PubrecMsg&)>;
    PubrecMsgHandlerFunc setPubrecMsgHandler(PubrecMsgHandlerFunc&& func);

    using PubrelMsgHandlerFunc = std::function<void (const PubrelMsg&)>;
    PubrelMsgHandlerFunc setPubrelMsgHandler(PubrelMsgHandlerFunc&& func);

    using PubcompMsgHandlerFunc = std::function<void (const PubcompMsg&)>;
    PubcompMsgHandlerFunc setPubcompMsgHandler(PubcompMsgHandlerFunc&& func);

    using SubscribeMsgHandlerFunc = std::function<void (const SubscribeMsg&)>;
    SubscribeMsgHandlerFunc setSubscribeMsgHandler(SubscribeMsgHandlerFunc&& func);

    using UnsubscribeMsgHandlerFunc = std::function<void (const UnsubscribeMsg&)>;
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
    virtual void handle(FwdMsg_SN& msg) override;
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

    using TopicIdTypeVal = cc_mqttsn::field::TopicIdTypeVal;

    void processDataForClient(const DataBuf& data);
    void processDataForBroker(const DataBuf& data);

    DataBuf prepareInput(const TestMqttsnMessage& msg);
    DataBuf prepareInput(const TestMqttMessage& msg);

    DataBuf prepareSearchgw(std::uint8_t radius = 0);
    DataBuf prepareClientConnect(const std::string& id, std::uint16_t keepAlive, bool hasWill, bool clean);
    DataBuf prepareClientWilltopic(
        const std::string& topic,
        cc_mqttsn::field::QosVal qos,
        bool retain);
    DataBuf prepareClientWillmsg(const DataBuf& data);
    DataBuf prepareClientDisconnect(std::uint16_t duration = 0);
    DataBuf prepareClientRegister(const std::string& topic, std::uint16_t msgId);
    DataBuf prepareClientRegack(
        std::uint16_t topicId,
        std::uint16_t msgId,
        cc_mqttsn::field::ReturnCodeVal rc);
    DataBuf prepareClientPublish(
        const DataBuf& data,
        std::uint16_t topicId,
        std::uint16_t msgId,
        TopicIdTypeVal topicIdType,
        cc_mqttsn::field::QosVal qos,
        bool retain,
        bool dup);
    DataBuf prepareClientPuback(
        std::uint16_t topicId,
        std::uint16_t msgId,
        cc_mqttsn::field::ReturnCodeVal rc);
    DataBuf prepareClientPubrec(std::uint16_t msgId);
    DataBuf prepareClientPubrel(std::uint16_t msgId);
    DataBuf prepareClientPubcomp(std::uint16_t msgId);
    DataBuf prepareClientPingreq(const std::string& clientId = std::string());
    DataBuf prepareClientPingresp();
    DataBuf prepareClientSubscribe(
        std::uint16_t topicId,
        std::uint16_t msgId,
        cc_mqttsn::field::QosVal qos);
    DataBuf prepareClientSubscribe(
        const std::string& topic,
        std::uint16_t msgId,
        cc_mqttsn::field::QosVal qos);
    DataBuf prepareClientUnsubscribe(
        std::uint16_t topicId,
        std::uint16_t msgId);
    DataBuf prepareClientUnsubscribe(
        const std::string& topic,
        std::uint16_t msgId);
    DataBuf prepareClientWilltopicupd(
        const std::string& topic,
        cc_mqttsn::field::QosVal qos,
        bool retain);
    DataBuf prepareClientWillmsgupd(const DataBuf& data);
    DataBuf prepareClientFwd(std::uint8_t nodeId, const DataBuf& data);

    using ConnackResponseCodeVal = ConnackMsg::Field_returnCode::ValueType;
    DataBuf prepareBrokerConnack(ConnackResponseCodeVal rc, bool sessionPresent = false);
    DataBuf prepareBrokerDisconnect();
    DataBuf prepareBrokerPingreq();
    DataBuf prepareBrokerPingresp();
    DataBuf prepareBrokerPublish(
        const std::string& topic,
        const DataBuf& msgData,
        std::uint16_t packetId,
        cc_mqtt311::field::QosVal qos,
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

    TestMqttsnFrame m_mqttsnFrame;
    TestMqttFrame m_mqttFrame;

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
    FwdMsgHandlerFunc m_fwdMsgHandler;

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
