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

#include <cassert>
#include <vector>
#include <cstdint>

#include "comms/comms.h"
#include "mqttsn/protocol/Stack.h"

class DataProcessor;

typedef mqttsn::protocol::MessageT<
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::WriteIterator<std::uint8_t*>,
    comms::option::Handler<DataProcessor>
> TestMessage;

typedef mqttsn::protocol::AllMessages<TestMessage> AllTestMessages;

class DataProcessor : public comms::GenericHandler<TestMessage, AllTestMessages>
{
    typedef comms::GenericHandler<TestMessage, AllTestMessages> Base;
public:
    typedef std::vector<std::uint8_t> DataBuf;

    typedef mqttsn::protocol::message::Advertise<TestMessage> AdvertiseMsg;
    typedef mqttsn::protocol::message::Searchgw<TestMessage> SearchgwMsg;
    typedef mqttsn::protocol::message::Gwinfo<TestMessage> GwinfoMsg;
    typedef mqttsn::protocol::message::Connect<TestMessage> ConnectMsg;
    typedef mqttsn::protocol::message::Connack<TestMessage> ConnackMsg;
    typedef mqttsn::protocol::message::Willtopicreq<TestMessage> WilltopicreqMsg;
    typedef mqttsn::protocol::message::Willtopic<TestMessage> WilltopicMsg;
    typedef mqttsn::protocol::message::Willmsgreq<TestMessage> WillmsgreqMsg;
    typedef mqttsn::protocol::message::Willmsg<TestMessage> WillmsgMsg;
    typedef mqttsn::protocol::message::Register<TestMessage> RegisterMsg;
    typedef mqttsn::protocol::message::Regack<TestMessage> RegackMsg;
    typedef mqttsn::protocol::message::Publish<TestMessage> PublishMsg;
    typedef mqttsn::protocol::message::Puback<TestMessage> PubackMsg;
    typedef mqttsn::protocol::message::Pubrec<TestMessage> PubrecMsg;
    typedef mqttsn::protocol::message::Pubrel<TestMessage> PubrelMsg;
    typedef mqttsn::protocol::message::Pubcomp<TestMessage> PubcompMsg;

    typedef mqttsn::protocol::message::Pingreq<TestMessage> PingreqMsg;
    typedef mqttsn::protocol::message::Pingresp<TestMessage> PingrespMsg;
    typedef mqttsn::protocol::message::Disconnect<TestMessage> DisconnectMsg;


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

    typedef std::function<void (const PublishMsg& msg)> PublishMsgReportCallback;
    PublishMsgReportCallback setPublishMsgReportCallback(PublishMsgReportCallback&& func);

    typedef std::function<void (const PingreqMsg& msg)> PingreqMsgReportCallback;
    PingreqMsgReportCallback setPingreqMsgReportCallback(PingreqMsgReportCallback&& func);

    typedef std::function<void (const PingrespMsg& msg)> PingrespMsgReportCallback;
    PingrespMsgReportCallback setPingrespMsgReportCallback(PingrespMsgReportCallback&& func);

    typedef std::function<void (const DisconnectMsg& msg)> DisconnectMsgReportCallback;
    DisconnectMsgReportCallback setDisconnectMsgReportCallback(DisconnectMsgReportCallback&& func);


    using Base::handle;
    virtual void handle(SearchgwMsg& msg) override;
    virtual void handle(ConnectMsg& msg) override;
    virtual void handle(WilltopicMsg& msg) override;
    virtual void handle(WillmsgMsg& msg) override;
    virtual void handle(RegisterMsg& msg) override;
    virtual void handle(PublishMsg& msg) override;
    virtual void handle(PingreqMsg& msg) override;
    virtual void handle(PingrespMsg& msg) override;
    virtual void handle(DisconnectMsg& msg) override;


    void checkWrittenMsg(const std::uint8_t* buf, std::size_t len);
    void checkWrittenMsg(const DataBuf& data);
    DataBuf prepareInput(const TestMessage& msg);

    DataBuf prepareGwinfoMsg(std::uint8_t id);
    DataBuf prepareAdvertiseMsg(std::uint8_t id, unsigned short duration);
    DataBuf prepareConnackMsg(mqttsn::protocol::field::ReturnCodeVal val);
    DataBuf prepareWilltopicreqMsg();
    DataBuf prepareWillmsgreqMsg();
    DataBuf prepareRegackMsg(
        std::uint16_t topicId,
        std::uint16_t msgId,
        mqttsn::protocol::field::ReturnCodeVal retCode);
    DataBuf preparePingreqMsg();
    DataBuf preparePingrespMsg();
    DataBuf prepareDisconnectMsg();

private:
    typedef mqttsn::protocol::Stack<TestMessage, AllTestMessages> ProtStack;

    ProtStack m_stack;
    SearchgwMsgReportCallback m_searchgwMsgReportCallback;
    ConnectMsgReportCallback m_connectMsgReportCallback;
    WilltopicMsgReportCallback m_willtopicMsgReportCallback;
    WillmsgMsgReportCallback m_willmsgMsgReportCallback;
    RegisterMsgReportCallback m_registerMsgReportCallback;
    PublishMsgReportCallback m_publishMsgReportCallback;
    PingreqMsgReportCallback m_pingreqMsgReportCallback;
    PingrespMsgReportCallback m_pingrespMsgReportCallback;
    DisconnectMsgReportCallback m_disconnectMsgReportCallback;
};


