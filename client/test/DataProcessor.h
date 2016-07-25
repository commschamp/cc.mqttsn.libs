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

#include "comms/comms.h"
#include "mqttsn/protocol/Stack.h"

class DataProcessor;

typedef mqttsn::protocol::MessageT<
    comms::option::ReadIterator<const std::uint8_t*>,
    comms::option::Handler<DataProcessor>
> OutputMessage;

typedef mqttsn::protocol::AllMessages<OutputMessage> AllOutputMessages;

class DataProcessor : public comms::GenericHandler<OutputMessage, AllOutputMessages>
{
    typedef comms::GenericHandler<OutputMessage, AllOutputMessages> Base;
public:
    typedef mqttsn::protocol::message::Searchgw<OutputMessage> SearchgwMsg;

    virtual ~DataProcessor();

    typedef std::function<void (const SearchgwMsg& msg)> SearchgwMsgReportCallback;
    void setSearchgwMsgReportCallback(SearchgwMsgReportCallback&& func);

    using Base::handle;
    virtual void handle(SearchgwMsg& msg) override;
    void checkWrittenMsg(const std::uint8_t* buf, std::size_t len);
private:
    typedef mqttsn::protocol::Stack<OutputMessage, AllOutputMessages> ProtStack;

    ProtStack m_stack;
    SearchgwMsgReportCallback m_searchgwMsgReportCallback;
};


