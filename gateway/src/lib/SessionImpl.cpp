//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "SessionImpl.h"

#include <iterator>
#include <cassert>
#include <algorithm>
#include <limits>

#include "session_op/Connect.h"
#include "session_op/Disconnect.h"
#include "session_op/Asleep.h"
#include "session_op/AsleepMonitor.h"
#include "session_op/PubRecv.h"
#include "session_op/PubSend.h"
#include "session_op/Forward.h"
#include "session_op/WillUpdate.h"

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
        typename TStack::MsgPtr msg;

        typedef typename TStack::MsgPtr::element_type MsgType;
        auto iter = comms::readIteratorFor<MsgType>(bufTmp);
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
            m_state.m_lastMsgTimestamp = m_state.m_timestamp;
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

    typedef typename TStack::MsgPtr::element_type MsgType;

    buf.resize(std::max(buf.size(), stack.length(msg)));
    auto iter = comms::writeIteratorFor<MsgType>(&buf[0]);
    auto es = stack.write(msg, iter, buf.size());
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    auto writtenCount =
        static_cast<std::size_t>(
            std::distance(comms::writeIteratorFor<MsgType>(&buf[0]), iter));

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
    std::unique_ptr<session_op::Connect> connectOp(new session_op::Connect(m_state));
    connectOp->setClientConnectedReportCb(
        [this](const std::string& clientId)
        {
            if (m_clientConnectedCb) {
                m_clientConnectedCb(clientId);
            }
        });
    connectOp->setAuthInfoReqCb(
        [this](const std::string& clientId) -> AuthInfo
        {
            if (!m_authInfoReqCb) {
                return AuthInfo();
            }

            return m_authInfoReqCb(clientId);
        });

    m_ops.push_back(std::move(connectOp));
    m_ops.emplace_back(new session_op::Disconnect(m_state));
    m_ops.emplace_back(new session_op::Asleep(m_state));
    m_ops.emplace_back(new session_op::AsleepMonitor(m_state));
    m_ops.emplace_back(new session_op::PubRecv(m_state));
    m_ops.emplace_back(new session_op::PubSend(m_state));
    m_ops.emplace_back(new session_op::Forward(m_state));
    m_ops.emplace_back(new session_op::WillUpdate(m_state));

    for (auto& op : m_ops) {
        startOp(*op);
    }
}

void SessionImpl::tick()
{
    if ((!isRunning()) || m_state.m_terminating) {
        return;
    }

    m_state.m_timestamp += m_state.m_tickReq;
    m_state.m_tickReq = 0U;

    auto guard = apiCall();
    updateOps();
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

bool SessionImpl::addPredefinedTopic(const std::string& topic, std::uint16_t topicId)
{
    return m_state.m_regMgr.regPredefined(topic, topicId);
}

bool SessionImpl::setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal)
{
    return m_state.m_regMgr.setTopicIdAllocationRange(minVal, maxVal);
}

void SessionImpl::handle(SearchgwMsg_SN& msg)
{
    static_cast<void>(msg);
    GwinfoMsg_SN respMsg;
    auto& fields = respMsg.fields();
    auto& gwIdField = std::get<decltype(respMsg)::FieldIdx_gwId>(fields);
    gwIdField.value() = m_state.m_gwId;
    sendToClient(respMsg);

    if (m_state.m_connStatus != ConnectionStatus::Disconnected) {
        sendToBroker(PingreqMsg());
    }
}

void SessionImpl::handle(RegisterMsg_SN& msg)
{
    typedef RegisterMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    auto& topicField = std::get<MsgType::FieldIdx_topicName>(fields);

    RegackMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& respTopicIdField = std::get<decltype(respMsg)::FieldIdx_topicId>(respFields);
    auto& respMsgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);
    auto& respRetCodeField = std::get<decltype(respMsg)::FieldIdx_returnCode>(respFields);

    auto& topicView = topicField.value();
    respTopicIdField.value() =
        m_state.m_regMgr.mapTopicNoInfo(std::string(topicView.data(), topicView.size()));

    respMsgIdField.value() = msgIdField.value();
    assert(respRetCodeField.value() == ReturnCodeVal::Accepted);
    if (respTopicIdField.value() == 0U) {
        respRetCodeField.value() = ReturnCodeVal::NotSupported;
    }
    sendToClient(respMsg);

    if (m_state.m_connStatus != ConnectionStatus::Disconnected) {
        sendToBroker(PingreqMsg());
    }
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

    assert(m_state.m_tickReq == 0U);
    unsigned delay = NoTimeout;
    for (auto& op : m_ops) {
        delay = std::min(delay, op->nextTick());
    }

    if (delay == NoTimeout) {
        return;
    }

    GASSERT(m_nextTickProgramCb != nullptr);
    m_nextTickProgramCb(delay);
    m_state.m_tickReq = delay;
}

void SessionImpl::updateTimestamp()
{
    if (m_state.m_tickReq == 0U) {
        return;
    }

    GASSERT(m_cancelTickCb);
    m_state.m_timestamp += m_cancelTickCb();
    m_state.m_tickReq = 0U;
    updateOps();
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

#ifdef _MSC_VER
// VS compiler
auto SessionImpl::apiCall() -> decltype(comms::util::makeScopeGuard(std::declval<ApiCallGuard>()))
{
    ++m_state.m_callStackCount;
    if (m_state.m_callStackCount == 1U) {
        updateTimestamp();
    }

    return
        comms::util::makeScopeGuard(
            ApiCallGuard(
                [this]()
                {
                    apiCallExit();
                }));
}

#else // #ifdef _MSC_VER
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

#endif // #ifdef _MSC_VER


}  // namespace gateway

}  // namespace mqttsn


