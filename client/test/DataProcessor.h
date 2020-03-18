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

#include <cassert>
#include <vector>
#include <cstdint>
#include <functional>

#include "comms/comms.h"
#include "mqttsn/frame/Frame.h"
#include "mqttsn/Message.h"
#include "mqttsn/client/common.h"

class DataProcessor;

typedef mqttsn::Message<
    comms::option::IdInfoInterface,
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::Handler<DataProcessor>,
    comms::option::LengthInfoInterface
> TestMessage;

typedef mqttsn::input::AllMessages<TestMessage> AllTestMessages;

class DataProcessor : public comms::GenericHandler<TestMessage, AllTestMessages>
{
    typedef comms::GenericHandler<TestMessage, AllTestMessages> Base;
public:
    typedef std::vector<std::uint8_t> DataBuf;
    using TopicIdTypeVal = mqttsn::field::FlagsMembersCommon::TopicIdTypeVal;

    typedef mqttsn::message::Advertise<TestMessage> AdvertiseMsg;
    typedef mqttsn::message::Searchgw<TestMessage> SearchgwMsg;
    typedef mqttsn::message::Gwinfo<TestMessage> GwinfoMsg;
    typedef mqttsn::message::Connect<TestMessage> ConnectMsg;
    typedef mqttsn::message::Connack<TestMessage> ConnackMsg;
    typedef mqttsn::message::Willtopicreq<TestMessage> WilltopicreqMsg;
    typedef mqttsn::message::Willtopic<TestMessage> WilltopicMsg;
    typedef mqttsn::message::Willmsgreq<TestMessage> WillmsgreqMsg;
    typedef mqttsn::message::Willmsg<TestMessage> WillmsgMsg;
    typedef mqttsn::message::Register<TestMessage> RegisterMsg;
    typedef mqttsn::message::Regack<TestMessage> RegackMsg;
    typedef mqttsn::message::Publish<TestMessage> PublishMsg;
    typedef mqttsn::message::Puback<TestMessage> PubackMsg;
    typedef mqttsn::message::Pubrec<TestMessage> PubrecMsg;
    typedef mqttsn::message::Pubrel<TestMessage> PubrelMsg;
    typedef mqttsn::message::Pubcomp<TestMessage> PubcompMsg;
    typedef mqttsn::message::Subscribe<TestMessage> SubscribeMsg;
    typedef mqttsn::message::Suback<TestMessage> SubackMsg;
    typedef mqttsn::message::Unsubscribe<TestMessage> UnsubscribeMsg;
    typedef mqttsn::message::Unsuback<TestMessage> UnsubackMsg;
    typedef mqttsn::message::Pingreq<TestMessage> PingreqMsg;
    typedef mqttsn::message::Pingresp<TestMessage> PingrespMsg;
    typedef mqttsn::message::Disconnect<TestMessage> DisconnectMsg;
    typedef mqttsn::message::Willtopicupd<TestMessage> WilltopicupdMsg;
    typedef mqttsn::message::Willmsgupd<TestMessage> WillmsgupdMsg;
    typedef mqttsn::message::Willtopicresp<TestMessage> WilltopicrespMsg;
    typedef mqttsn::message::Willmsgresp<TestMessage> WillmsgrespMsg;


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
    DataBuf prepareConnackMsg(mqttsn::field::ReturnCodeVal val);
    DataBuf prepareWilltopicreqMsg();
    DataBuf prepareWillmsgreqMsg();
    DataBuf prepareRegisterMsg(
        std::uint16_t topicId,
        std::uint16_t msgId,
        const std::string& topicName);
    DataBuf prepareRegackMsg(
        std::uint16_t topicId,
        std::uint16_t msgId,
        mqttsn::field::ReturnCodeVal retCode);
    DataBuf preparePublishMsg(
        std::uint16_t topicId,
        std::uint16_t msgId,
        const std::vector<std::uint8_t>& data,
        TopicIdTypeVal topicIdType,
        mqttsn::field::QosVal qos,
        bool retain,
        bool duplicate);
    DataBuf preparePubackMsg(
        MqttsnTopicId topicId,
        std::uint16_t msgId,
        mqttsn::field::ReturnCodeVal retCode);
    DataBuf preparePubrecMsg(std::uint16_t msgId);
    DataBuf preparePubrelMsg(std::uint16_t msgId);
    DataBuf preparePubcompMsg(std::uint16_t msgId);
    DataBuf prepareSubackMsg(
        mqttsn::field::QosVal qos,
        MqttsnTopicId topicId,
        std::uint16_t msgId,
        mqttsn::field::ReturnCodeVal retCode);
    DataBuf prepareUnsubackMsg(std::uint16_t msgId);
    DataBuf preparePingreqMsg();
    DataBuf preparePingrespMsg();
    DataBuf prepareDisconnectMsg(std::uint16_t duration = 0);
    DataBuf prepareWilltopicrespMsg(mqttsn::field::ReturnCodeVal retCode);
    DataBuf prepareWillmsgrespMsg(mqttsn::field::ReturnCodeVal retCode);

private:
    typedef mqttsn::frame::Frame<TestMessage, AllTestMessages> ProtStack;

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


