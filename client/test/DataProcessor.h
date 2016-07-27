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

    virtual ~DataProcessor();

    typedef std::function<void (const SearchgwMsg& msg)> SearchgwMsgReportCallback;
    void setSearchgwMsgReportCallback(SearchgwMsgReportCallback&& func);

    typedef std::function<void (const ConnectMsg& msg)> ConnectMsgReportCallback;
    void setConnectMsgReportCallback(ConnectMsgReportCallback&& func);

    using Base::handle;
    virtual void handle(SearchgwMsg& msg) override;
    virtual void handle(ConnectMsg& msg) override;


    void checkWrittenMsg(const std::uint8_t* buf, std::size_t len);
    DataBuf prepareInput(const TestMessage& msg);

    DataBuf prepareConnack(mqttsn::protocol::field::ReturnCodeVal val);

private:
    typedef mqttsn::protocol::Stack<TestMessage, AllTestMessages> ProtStack;

    ProtStack m_stack;
    SearchgwMsgReportCallback m_searchgwMsgReportCallback;
    ConnectMsgReportCallback m_connectMsgReportCallback;
};


