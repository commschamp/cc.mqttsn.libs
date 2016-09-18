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
#include <limits>

#include "session_op/Connect.h"

namespace mqttsn
{

namespace gateway
{

namespace
{

const unsigned NoTimeout = std::numeric_limits<unsigned>::max();

}  // namespace

template <typename TStack>
std::size_t SessionImpl::processInputData(const std::uint8_t* buf, std::size_t len, TStack& stack)
{
    auto guard = apiCall();
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
void SessionImpl::dispatchToOpsCommon(TMsg& msg)
{
    for (auto& op : m_ops) {
        msg.dispatch(*op);
    }
}

void SessionImpl::tick(unsigned ms)
{
    m_timestamp += ms;
    updateOps();
    programNextTimeout();
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
    if (m_connStatus != ConnectionStatus::Connecting) {
        std::unique_ptr<session_op::Connect> op(new session_op::Connect(m_clientId, m_connStatus));
        op->setAuth(m_username, m_password);
        startOp(*op);
        m_ops.push_back(std::move(op));
    }

    dispatchToOps(msg);
}

void SessionImpl::handle(MqttsnMessage& msg)
{
    dispatchToOps(msg);
}

void SessionImpl::handle(MqttMessage& msg)
{
    dispatchToOps(msg);
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

void SessionImpl::dispatchToOps(MqttsnMessage& msg)
{
    dispatchToOpsCommon(msg);
}

void SessionImpl::dispatchToOps(MqttMessage& msg)
{
    dispatchToOpsCommon(msg);
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

void SessionImpl::programNextTimeout()
{
    if (!m_running) {
        return;
    }

    assert(!m_timerActive);
    unsigned delay = NoTimeout;
    for (auto& op : m_ops) {
        delay = std::min(delay, op->nextTick());
    }
    // TODO: other delays

    if (delay == NoTimeout) {
        return;
    }

    GASSERT(m_nextTickProgramCb != nullptr);
    m_nextTickProgramCb(delay);
    m_timerActive = true;
}

void SessionImpl::updateTimestamp()
{
    if (!m_timerActive) {
        return;
    }

    GASSERT(m_cancelTickCb);
    m_timestamp += m_cancelTickCb();
    m_timerActive = false;
    updateOps();
    // TODO: check other timers
}

void SessionImpl::updateOps()
{
    for (auto& op : m_ops) {
        op->updateTimestamp(m_timestamp);
    }
    cleanCompleteOps();
}

void SessionImpl::apiCallExit()
{
    GASSERT(0U < m_callStackCount);
    --m_callStackCount;
    if (m_callStackCount == 0U) {
        programNextTimeout();
    }
}

auto SessionImpl::apiCall() -> decltype(comms::util::makeScopeGuard(std::bind(&SessionImpl::apiCallExit, this)))
{
    ++m_callStackCount;
    if (m_callStackCount == 1U) {
        updateTimestamp();
    }

    return
        comms::util::makeScopeGuard(
            std::bind(
                &SessionImpl::apiCallExit,
                this));
}



}  // namespace gateway

}  // namespace mqttsn


