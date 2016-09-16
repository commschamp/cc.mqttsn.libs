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

#include "SessionImpl.h"

#include <iterator>
#include <cassert>
#include <algorithm>

#include "session_op/Connect.h"

namespace mqttsn
{

namespace gateway
{

template <typename TStack>
std::size_t SessionImpl::processInputData(const std::uint8_t* buf, std::size_t len, TStack& stack)
{
    const std::uint8_t* bufTmp = buf;
    while (true) {
        typename TStack::ReadIterator iter = bufTmp;
        typename TStack::MsgPtr msg;
        auto consumedBytes =
            static_cast<std::size_t>(std::distance(buf, bufTmp));
        assert(consumedBytes <= len);
        auto remLen = len - consumedBytes;
        if (remLen == 0U) {
            break;
        }

        auto es = stack.read(msg, iter, remLen);
        if (es == comms::ErrorStatus::NotEnoughData) {
            break;
        }

        if (es == comms::ErrorStatus::ProtocolError) {
            ++bufTmp;
            continue;
        }

        if (es == comms::ErrorStatus::Success) {
            assert(msg);
            msg->dispatch(*this);
            cleanCompleteOps();
        }

        bufTmp = iter;
    }

    auto consumed = static_cast<std::size_t>(std::distance(buf, bufTmp));
    assert(consumed <= len);
    return consumed;
}

template <typename TMsg, typename TStack>
void SessionImpl::sendMessage(const TMsg& msg, TStack& stack, SendDataReqCb& func, DataBuf& buf)
{
    if (!func) {
        return;
    }

    typedef typename TStack::WriteIterator WriteIterator;

    buf.resize(std::max(buf.size(), stack.length(msg)));
    WriteIterator iter = &buf[0];
    auto es = stack.write(msg, iter, buf.size());
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    auto writtenCount =
        static_cast<std::size_t>(
            std::distance(WriteIterator(&buf[0]), iter));

    func(&buf[0], writtenCount);
}

template <typename TMsg>
void SessionImpl::dispatchToOpCommon(SessionOp::Type type, TMsg& msg)
{
    for (auto& op : m_ops) {
        if (op->type() == type) {
            msg.dispatch(*op);
        }
    }
}

void SessionImpl::tick(unsigned ms)
{
    static_cast<void>(ms);
    m_timestamp += ms;
    // TODO
    for (auto& op : m_ops) {
        op->updateTimestamp(m_timestamp);
    }
    cleanCompleteOps();
}

std::size_t SessionImpl::dataFromClient(const std::uint8_t* buf, std::size_t len)
{
    return processInputData(buf, len, m_mqttsnStack);
}

std::size_t SessionImpl::dataFromBroker(const std::uint8_t* buf, std::size_t len)
{
    return processInputData(buf, len, m_mqttStack);
}

void SessionImpl::handle(SearchgwMsg_SN& msg)
{
    static_cast<void>(msg);
    GwinfoMsg_SN respMsg;
    auto& fields = respMsg.fields();
    auto& gwIdField = std::get<decltype(respMsg)::FieldIdx_gwId>(fields);
    gwIdField.value() = m_gwId;
    sendToClient(respMsg);
}

void SessionImpl::handle(ConnectMsg_SN& msg)
{
    SessionOp* op = nullptr;
    auto iter = findOp(SessionOp::Type::Connect);
    if (iter != m_ops.end()) {
        op = iter->get();
    }
    else {
        m_ops.emplace_back(new session_op::Connect(m_connInfo));
        op = m_ops.back().get();
        startOp(*op);
    }

    assert(!op->isComplete());
    msg.dispatch(*op);
}

void SessionImpl::handle(WilltopicMsg_SN& msg)
{
    dispatchToOp(SessionOp::Type::Connect, msg);
}

void SessionImpl::handle(WillmsgMsg_SN& msg)
{
    dispatchToOp(SessionOp::Type::Connect, msg);
}

void SessionImpl::handle(ConnackMsg& msg)
{
    dispatchToOp(SessionOp::Type::Connect, msg);
}

void SessionImpl::sendToClient(const MqttsnMessage& msg)
{
    sendMessage(msg, m_mqttsnStack, m_sendToClientCb, m_mqttsnMsgData);
}

void SessionImpl::sendToBroker(const MqttMessage& msg)
{
    sendMessage(msg, m_mqttStack, m_sendToBrokerCb, m_mqttMsgData);
}

void SessionImpl::startOp(SessionOp& op)
{
    op.setSendToClientCb(std::bind(&SessionImpl::sendToClient, this, std::placeholders::_1));
    op.setSendToBrokerCb(std::bind(&SessionImpl::sendToBroker, this, std::placeholders::_1));
    op.setRetryPeriod(m_retryPeriod);
    op.setRetryCount(m_retryCount);
    op.updateTimestamp(m_timestamp);
    op.start();
}

SessionImpl::OpsList::iterator SessionImpl::findOp(SessionOp::Type type)
{
    return
        std::find_if(
            m_ops.begin(), m_ops.end(),
            [type](SessionImpl::OpsList::const_reference elem) -> bool
            {
                return elem->type() == type;
            });
}

void SessionImpl::dispatchToOp(SessionOp::Type type, MqttsnMessage& msg)
{
    dispatchToOpCommon(type, msg);
}

void SessionImpl::dispatchToOp(SessionOp::Type type, MqttMessage& msg)
{
    dispatchToOpCommon(type, msg);
}


void SessionImpl::cleanCompleteOps()
{
    std::list<OpsList::iterator> iters;
    for (auto iter = m_ops.begin(); iter != m_ops.end(); ++iter) {
        auto* op = iter->get();
        if (op->isComplete()) {
            iters.push_back(iter);
        }
    }

    for (auto i : iters) {
        m_ops.erase(i);
    }
}

}  // namespace gateway

}  // namespace mqttsn


