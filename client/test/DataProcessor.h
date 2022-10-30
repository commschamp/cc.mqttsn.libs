//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cassert>
#include <vector>
#include <cstdint>
#include <functional>

#include "comms/comms.h"
#include "cc_mqttsn/frame/Frame.h"
#include "cc_mqttsn/Message.h"
#include "mqttsn/client/common.h"

class DataProcessor;

typedef cc_mqttsn::Message<
    comms::option::IdInfoInterface,
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::Handler<DataProcessor>,
    comms::option::LengthInfoInterface
> TestMessage;

typedef cc_mqttsn::input::AllMessages<TestMessage> AllTestMessages;

class DataProcessor : public comms::GenericHandler<TestMessage, AllTestMessages>
{
    typedef comms::GenericHandler<TestMessage, AllTestMessages> Base;
public:
    typedef std::vector<std::uint8_t> DataBuf;
    using TopicIdTypeVal = cc_mqttsn::field::TopicIdTypeVal;

    typedef cc_mqttsn::message::Advertise<TestMessage> AdvertiseMsg;
    typedef cc_mqttsn::message::Searchgw<TestMessage> SearchgwMsg;
    typedef cc_mqttsn::message::Gwinfo<TestMessage> GwinfoMsg;
    typedef cc_mqttsn::message::Connect<TestMessage> ConnectMsg;
    typedef cc_mqttsn::message::Connack<TestMessage> ConnackMsg;
    typedef cc_mqttsn::message::Willtopicreq<TestMessage> WilltopicreqMsg;
    typedef cc_mqttsn::message::Willtopic<TestMessage> WilltopicMsg;
    typedef cc_mqttsn::message::Willmsgreq<TestMessage> WillmsgreqMsg;
    typedef cc_mqttsn::message::Willmsg<TestMessage> WillmsgMsg;
    typedef cc_mqttsn::message::Register<TestMessage> RegisterMsg;
    typedef cc_mqttsn::message::Regack<TestMessage> RegackMsg;
    typedef cc_mqttsn::message::Publish<TestMessage> PublishMsg;
    typedef cc_mqttsn::message::Puback<TestMessage> PubackMsg;
    typedef cc_mqttsn::message::Pubrec<TestMessage> PubrecMsg;
    typedef cc_mqttsn::message::Pubrel<TestMessage> PubrelMsg;
    typedef cc_mqttsn::message::Pubcomp<TestMessage> PubcompMsg;
    typedef cc_mqttsn::message::Subscribe<TestMessage> SubscribeMsg;
    typedef cc_mqttsn::message::Suback<TestMessage> SubackMsg;
    typedef cc_mqttsn::message::Unsubscribe<TestMessage> UnsubscribeMsg;
    typedef cc_mqttsn::message::Unsuback<TestMessage> UnsubackMsg;
    typedef cc_mqttsn::message::Pingreq<TestMessage> PingreqMsg;
    typedef cc_mqttsn::message::Pingresp<TestMessage> PingrespMsg;
    typedef cc_mqttsn::message::Disconnect<TestMessage> DisconnectMsg;
    typedef cc_mqttsn::message::Willtopicupd<TestMessage> WilltopicupdMsg;
    typedef cc_mqttsn::message::Willmsgupd<TestMessage> WillmsgupdMsg;
    typedef cc_mqttsn::message::Willtopicresp<TestMessage> WilltopicrespMsg;
    typedef cc_mqttsn::message::Willmsgresp<TestMessage> WillmsgrespMsg;


    virtual ~DataProcessor();

    typedef std::function<void (const SearchgwMsg& msg)> SearchgwMsgReportCallback;
    SearchgwMsgReportCallback setSearchgwMsgReportCallback(SearchgwMsgReportCallback&& func);

    typedef std::function<void (const ConnectMsg& msg)> ConnectMsgReportCallback;
    ConnectMsgReportCallback setConnectMsgReportCallback(ConnectMsgReportCallback&& func);

    typedef std::function<void (const WilltopicMsg& msg)> WilltopicMsgReportCallback;
    WilltopicMsgReportCallback setWilltopicMsgReportCallback(WilltopicMsgReportCallback&& func);

    typedef std::function<void (const WillmsgMsg& msg)> WillmsgMsgReportCallback;
    WillmsgMsgReportCallback setWillmsgMsgReportCallback(WillmsgMsgReportCallback&& func);

    typedef std::function<void (const RegisterMsg& msg)> RegisterMsgReportCallback;
    RegisterMsgReportCallback setRegisterMsgReportCallback(RegisterMsgReportCallback&& func);

    typedef std::function<void (const RegackMsg& msg)> RegackMsgReportCallback;
    RegackMsgReportCallback setRegackMsgReportCallback(RegackMsgReportCallback&& func);

    typedef std::function<void (const PublishMsg& msg)> PublishMsgReportCallback;
    PublishMsgReportCallback setPublishMsgReportCallback(PublishMsgReportCallback&& func);

    typedef std::function<void (const PubackMsg& msg)> PubackMsgReportCallback;
    PubackMsgReportCallback setPubackMsgReportCallback(PubackMsgReportCallback&& func);

    typedef std::function<void (const PubrecMsg& msg)> PubrecMsgReportCallback;
    PubrecMsgReportCallback setPubrecMsgReportCallback(PubrecMsgReportCallback&& func);

    typedef std::function<void (const PubrelMsg& msg)> PubrelMsgReportCallback;
    PubrelMsgReportCallback setPubrelMsgReportCallback(PubrelMsgReportCallback&& func);

    typedef std::function<void (const PubcompMsg& msg)> PubcompMsgReportCallback;
    PubcompMsgReportCallback setPubcompMsgReportCallback(PubcompMsgReportCallback&& func);

    typedef std::function<void (const SubscribeMsg& msg)> SubscribeMsgReportCallback;
    SubscribeMsgReportCallback setSubscribeMsgReportCallback(SubscribeMsgReportCallback&& func);

    typedef std::function<void (const UnsubscribeMsg& msg)> UnsubscribeMsgReportCallback;
    UnsubscribeMsgReportCallback setUnsubscribeMsgReportCallback(UnsubscribeMsgReportCallback&& func);

    typedef std::function<void (const PingreqMsg& msg)> PingreqMsgReportCallback;
    PingreqMsgReportCallback setPingreqMsgReportCallback(PingreqMsgReportCallback&& func);

    typedef std::function<void (const PingrespMsg& msg)> PingrespMsgReportCallback;
    PingrespMsgReportCallback setPingrespMsgReportCallback(PingrespMsgReportCallback&& func);

    typedef std::function<void (const DisconnectMsg& msg)> DisconnectMsgReportCallback;
    DisconnectMsgReportCallback setDisconnectMsgReportCallback(DisconnectMsgReportCallback&& func);

    typedef std::function<void (const WilltopicupdMsg& msg)> WilltopicupdMsgReportCallback;
    WilltopicupdMsgReportCallback setWilltopicupdMsgReportCallback(WilltopicupdMsgReportCallback&& func);

    typedef std::function<void (const WillmsgupdMsg& msg)> WillmsgupdMsgReportCallback;
    WillmsgupdMsgReportCallback setWillmsgupdMsgReportCallback(WillmsgupdMsgReportCallback&& func);

    using Base::handle;
    virtual void handle(SearchgwMsg& msg) override;
    virtual void handle(ConnectMsg& msg) override;
    virtual void handle(WilltopicMsg& msg) override;
    virtual void handle(WillmsgMsg& msg) override;
    virtual void handle(RegisterMsg& msg) override;
    virtual void handle(RegackMsg& msg) override;
    virtual void handle(PublishMsg& msg) override;
    virtual void handle(PubackMsg& msg) override;
    virtual void handle(PubrecMsg& msg) override;
    virtual void handle(PubrelMsg& msg) override;
    virtual void handle(PubcompMsg& msg) override;
    virtual void handle(SubscribeMsg& msg) override;
    virtual void handle(UnsubscribeMsg& msg) override;
    virtual void handle(PingreqMsg& msg) override;
    virtual void handle(PingrespMsg& msg) override;
    virtual void handle(DisconnectMsg& msg) override;
    virtual void handle(WilltopicupdMsg& msg) override;
    virtual void handle(WillmsgupdMsg& msg) override;
    virtual void handle(TestMessage& msg) override;


    void checkWrittenMsg(const std::uint8_t* buf, std::size_t len);
    void checkWrittenMsg(const DataBuf& data);
    DataBuf prepareInput(const TestMessage& msg);

    DataBuf prepareGwinfoMsg(std::uint8_t id);
    DataBuf prepareAdvertiseMsg(std::uint8_t id, unsigned short duration);
    DataBuf prepareConnackMsg(cc_mqttsn::field::ReturnCodeVal val);
    DataBuf prepareWilltopicreqMsg();
    DataBuf prepareWillmsgreqMsg();
    DataBuf prepareRegisterMsg(
        std::uint16_t topicId,
        std::uint16_t msgId,
        const std::string& topicName);
    DataBuf prepareRegackMsg(
        std::uint16_t topicId,
        std::uint16_t msgId,
        cc_mqttsn::field::ReturnCodeVal retCode);
    DataBuf preparePublishMsg(
        std::uint16_t topicId,
        std::uint16_t msgId,
        const std::vector<std::uint8_t>& data,
        TopicIdTypeVal topicIdType,
        cc_mqttsn::field::QosVal qos,
        bool retain,
        bool duplicate);
    DataBuf preparePubackMsg(
        MqttsnTopicId topicId,
        std::uint16_t msgId,
        cc_mqttsn::field::ReturnCodeVal retCode);
    DataBuf preparePubrecMsg(std::uint16_t msgId);
    DataBuf preparePubrelMsg(std::uint16_t msgId);
    DataBuf preparePubcompMsg(std::uint16_t msgId);
    DataBuf prepareSubackMsg(
        cc_mqttsn::field::QosVal qos,
        MqttsnTopicId topicId,
        std::uint16_t msgId,
        cc_mqttsn::field::ReturnCodeVal retCode);
    DataBuf prepareUnsubackMsg(std::uint16_t msgId);
    DataBuf preparePingreqMsg();
    DataBuf preparePingrespMsg();
    DataBuf prepareDisconnectMsg(std::uint16_t duration = 0);
    DataBuf prepareWilltopicrespMsg(cc_mqttsn::field::ReturnCodeVal retCode);
    DataBuf prepareWillmsgrespMsg(cc_mqttsn::field::ReturnCodeVal retCode);

private:
    typedef cc_mqttsn::frame::Frame<TestMessage, AllTestMessages> ProtStack;

    ProtStack m_stack;
    SearchgwMsgReportCallback m_searchgwMsgReportCallback;
    ConnectMsgReportCallback m_connectMsgReportCallback;
    WilltopicMsgReportCallback m_willtopicMsgReportCallback;
    WillmsgMsgReportCallback m_willmsgMsgReportCallback;
    RegisterMsgReportCallback m_registerMsgReportCallback;
    RegackMsgReportCallback m_regackMsgReportCallback;
    PublishMsgReportCallback m_publishMsgReportCallback;
    PubackMsgReportCallback m_pubackMsgReportCallback;
    PubrecMsgReportCallback m_pubrecMsgReportCallback;
    PubrelMsgReportCallback m_pubrelMsgReportCallback;
    PubcompMsgReportCallback m_pubcompMsgReportCallback;
    SubscribeMsgReportCallback m_subscribeMsgReportCallback;
    UnsubscribeMsgReportCallback m_unsubscribeMsgReportCallback;
    PingreqMsgReportCallback m_pingreqMsgReportCallback;
    PingrespMsgReportCallback m_pingrespMsgReportCallback;
    DisconnectMsgReportCallback m_disconnectMsgReportCallback;
    WilltopicupdMsgReportCallback m_willtopicupdMsgReportCallback;
    WillmsgupdMsgReportCallback m_willmsgupdMsgReportCallback;
};


