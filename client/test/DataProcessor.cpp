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

#include "DataProcessor.h"

#include <iostream>

DataProcessor::~DataProcessor() = default;

void DataProcessor::setSearchgwMsgReportCallback(SearchgwMsgReportCallback&& func)
{
    m_searchgwMsgReportCallback = std::move(func);
}

void DataProcessor::handle(SearchgwMsg& msg)
{
    if (m_searchgwMsgReportCallback) {
        m_searchgwMsgReportCallback(msg);
    }
}

void DataProcessor::checkWrittenMsg(const std::uint8_t* buf, std::size_t len)
{
    ProtStack::ReadIterator readIter = buf;
    ProtStack::MsgPtr msg;
    auto es = m_stack.read(msg, readIter, len);
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    assert(msg);
    msg->dispatch(*this);
}

DataProcessor::DataBuf DataProcessor::prepareInput(const TestMessage& msg)
{
    DataBuf buf(m_stack.length(msg));
    assert(buf.size() == m_stack.length(msg));
    ProtStack::WriteIterator writeIter = &buf[0];
    auto es = m_stack.write(msg, writeIter, buf.size());
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    return buf;
}
