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

DataProcessor::SearchgwMsgReportCallback DataProcessor::setSearchgwMsgReportCallback(SearchgwMsgReportCallback&& func)
{
    SearchgwMsgReportCallback old(std::move(m_searchgwMsgReportCallback));
    m_searchgwMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::ConnectMsgReportCallback DataProcessor::setConnectMsgReportCallback(ConnectMsgReportCallback&& func)
{
    ConnectMsgReportCallback old(std::move(m_connectMsgReportCallback));
    m_connectMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::WilltopicMsgReportCallback DataProcessor::setWilltopicMsgReportCallback(
    WilltopicMsgReportCallback&& func)
{
    WilltopicMsgReportCallback old = std::move(m_willtopicMsgReportCallback);
    m_willtopicMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::WillmsgMsgReportCallback DataProcessor::setWillmsgMsgReportCallback(
    WillmsgMsgReportCallback&& func)
{
    WillmsgMsgReportCallback old = std::move(m_willmsgMsgReportCallback);
    m_willmsgMsgReportCallback = std::move(func);
    return old;
}


void DataProcessor::handle(SearchgwMsg& msg)
{
    if (m_searchgwMsgReportCallback) {
        m_searchgwMsgReportCallback(msg);
    }
}

void DataProcessor::handle(ConnectMsg& msg)
{
    if (m_connectMsgReportCallback) {
        m_connectMsgReportCallback(msg);
    }
}

void DataProcessor::handle(WilltopicMsg& msg)
{
    if (m_willtopicMsgReportCallback) {
        m_willtopicMsgReportCallback(msg);
    }
}

void DataProcessor::handle(WillmsgMsg& msg)
{
    if (m_willmsgMsgReportCallback) {
        m_willmsgMsgReportCallback(msg);
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

DataProcessor::DataBuf DataProcessor::prepareGwinfoMsg(std::uint8_t id)
{
    GwinfoMsg msg;
    auto& fields = msg.fields();
    auto& gwIdField = std::get<GwinfoMsg::FieldIdx_gwId>(fields);
    gwIdField.value() = id;
    assert(msg.length() == 1U);
    auto buf = prepareInput(msg);
    assert(buf.size() == 3U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareAdvertiseMsg(std::uint8_t id, unsigned short duration)
{
    AdvertiseMsg msg;
    auto& fields = msg.fields();
    auto& gwIdField = std::get<AdvertiseMsg::FieldIdx_gwId>(fields);
    auto& durationField = std::get<AdvertiseMsg::FieldIdx_duration>(fields);
    gwIdField.value() = id;
    durationField.value() = duration;
    auto buf = prepareInput(msg);
    assert(buf.size() == 5U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareConnack(mqttsn::protocol::field::ReturnCodeVal val)
{
    ConnackMsg msg;
    auto& fields = msg.fields();
    auto& retCodeField = std::get<ConnackMsg::FieldIdx_returnCode>(fields);
    retCodeField.value() = val;
    auto buf = prepareInput(msg);
    assert(buf.size() == 3U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::preapareWilltopicreq()
{
    return prepareInput(WilltopicreqMsg());
}

DataProcessor::DataBuf DataProcessor::preapareWillmsgreq()
{
    return prepareInput(WillmsgreqMsg());
}
