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
#include "session_op/Disconnect.h"

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
    if ((!isRunning()) || m_state.m_terminating) {
        return 0U;
    }

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

SessionImpl::SessionImpl()
{
    m_ops.emplace_back(new session_op::Connect(m_state));
    m_ops.emplace_back(new session_op::Disconnect(m_state));

    for (auto& op : m_ops) {
        startOp(*op);
    }
}

void SessionImpl::tick(unsigned ms)
{
    if ((!isRunning()) || m_state.m_terminating) {
        return;
    }

    m_state.m_timerActive = false;
    m_state.m_timestamp += ms;
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

void SessionImpl::setBrokerConnected(bool connected)
{
    if ((!isRunning()) || (m_state.m_brokerConnected == connected)) {
        return;
    }

    auto guard = apiCall();
    if (connected && m_state.m_reconnectingBroker) {
        m_state.m_reconnectingBroker = false;
    }

    m_state.m_brokerConnected = connected;
    for (auto& op : m_ops) {
        op->brokerConnectionUpdated();
    }
}

void SessionImpl::handle(SearchgwMsg_SN& msg)
{
    static_cast<void>(msg);
    GwinfoMsg_SN respMsg;
    auto& fields = respMsg.fields();
    auto& gwIdField = std::get<decltype(respMsg)::FieldIdx_gwId>(fields);
    gwIdField.value() = m_state.m_gwId;
    sendToClient(respMsg);
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
    op.setSessionTermReqCb(
        [this]()
        {
            if ((!m_termReqCb) || (m_state.m_terminating)) {
                return;
            }

            m_state.m_terminating = true;
        });

    op.setBrokerReconnectReqCb(
        [this]()
        {
            if ((!m_brokerReconnectReqCb) ||
                (m_state.m_reconnectingBroker) ||
                (m_state.m_terminating)) {
                return;
            }

            m_state.m_reconnectingBroker = true;
            m_brokerReconnectReqCb();
        });
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

void SessionImpl::programNextTimeout()
{
    if (!isRunning()) {
        return;
    }

    assert(!m_state.m_timerActive);
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
    m_state.m_timerActive = true;
}

void SessionImpl::updateTimestamp()
{
    if (!m_state.m_timerActive) {
        return;
    }

    GASSERT(m_cancelTickCb);
    m_state.m_timestamp += m_cancelTickCb();
    m_state.m_timerActive = false;
    updateOps();
    // TODO: check other timers
}

void SessionImpl::updateOps()
{
    for (auto& op : m_ops) {
        op->timestampUpdated();
    }
}

void SessionImpl::apiCallExit()
{
    GASSERT(0U < m_state.m_callStackCount);
    --m_state.m_callStackCount;

    if (m_state.m_terminating) {
        assert(m_termReqCb);
        m_termReqCb();
        return;
    }

    if (m_state.m_callStackCount == 0U) {
        programNextTimeout();
    }
}

auto SessionImpl::apiCall() -> decltype(comms::util::makeScopeGuard(std::bind(&SessionImpl::apiCallExit, this)))
{
    ++m_state.m_callStackCount;
    if (m_state.m_callStackCount == 1U) {
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


