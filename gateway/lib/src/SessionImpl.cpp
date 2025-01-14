//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "SessionImpl.h"

#include <iterator>
#include <cassert>
#include <algorithm>
#include <limits>

#include "comms/process.h"

#include "session_op/Connect.h"
#include "session_op/Disconnect.h"
#include "session_op/Asleep.h"
#include "session_op/AsleepMonitor.h"
#include "session_op/Encapsulate.h"
#include "session_op/Ping.h"
#include "session_op/PubRecv.h"
#include "session_op/PubSend.h"
#include "session_op/Forward.h"
#include "session_op/WillUpdate.h"

namespace cc_mqttsn_gateway
{

namespace
{

const unsigned NoTimeout = std::numeric_limits<unsigned>::max();

}  // namespace

template <typename TMsg>
void SessionImpl::dispatchToOpsCommon(TMsg& msg)
{
    for (auto& op : m_ops) {
        msg.dispatch(*op);
    }
}

SessionImpl::SessionImpl()
{
    m_ops.emplace_back(new session_op::Connect(*this));
    m_ops.emplace_back(new session_op::Disconnect(*this));
    m_ops.emplace_back(new session_op::Asleep(*this));
    m_ops.emplace_back(new session_op::AsleepMonitor(*this));
    m_ops.emplace_back(new session_op::PubRecv(*this));
    m_ops.emplace_back(new session_op::PubSend(*this));
    m_ops.emplace_back(new session_op::Forward(*this));
    m_ops.emplace_back(new session_op::WillUpdate(*this));
    m_ops.emplace_back(new session_op::Ping(*this));
    m_ops.emplace_back(new session_op::Encapsulate(*this));
    m_encapsulateOp = static_cast<decltype(m_encapsulateOp)>(m_ops.back().get());

    for (auto& op : m_ops) {
        op->start();
    }
}

bool SessionImpl::start()
{
    if ((m_state.m_running) ||
        (!m_nextTickProgramCb) ||
        (!m_cancelTickCb) ||
        (!m_sendToClientCb) ||
        (!m_sendToBrokerCb) ||
        (!m_termReqCb) ||
        (!m_brokerReconnectReqCb)) {
        reportError("Not all \"must have\" callbacks have been set");
        return false;
    }

    if (static_cast<bool>(m_fwdEncSessionCreatedReportCb) != static_cast<bool>(m_fwdEncSessionDeletedReportCb)) {
        reportError("Not all forward encapsulated session management callbacks have been set");
        return false;
    }

    m_state.m_running = true;
    return true;
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
    if ((!isRunning()) || m_state.m_terminating) {
        return 0U;
    }

    auto guard = apiCall();
    const std::uint8_t* bufTmp = buf;
    while (true) {
        auto consumedBytes =
            static_cast<std::size_t>(std::distance(buf, bufTmp));
        assert(consumedBytes <= len);
        auto remLen = len - consumedBytes;
        if (remLen == 0U) {
            m_state.m_encapsulatedMsg = false; // Just in case
            break;
        }

        if (m_state.m_encapsulatedMsg) {
            bufTmp += m_encapsulateOp->encapsulatedData(bufTmp, remLen);
            continue;
        }        

        using MsgPtr = typename MqttsnFrame::MsgPtr;
        using MsgType = typename MsgPtr::element_type;
        MsgPtr msg;
        auto iter = comms::readIteratorFor<MsgType>(bufTmp);
        auto es = m_mqttsnFrame.read(msg, iter, remLen);
        if (es == comms::ErrorStatus::NotEnoughData) {
            break;
        }

        if (es == comms::ErrorStatus::ProtocolError) {
            reportError("Protocol error from the client side is detected");
            return len;
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

std::size_t SessionImpl::dataFromBroker(const std::uint8_t* buf, std::size_t len)
{
    if ((!isRunning()) || m_state.m_terminating) {
        return 0U;
    }

    auto guard = apiCall();
    return comms::processAllWithDispatch(buf, len, m_mqttFrame, *this);
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

bool SessionImpl::reportFwdEncSessionCreated(Session* session)
{
    assert(m_fwdEncSessionCreatedReportCb);
    assert(session != nullptr);
    return m_fwdEncSessionCreatedReportCb(session);
}

void SessionImpl::reportFwdEncSessionDeleted(Session* session)
{
    assert(m_fwdEncSessionDeletedReportCb);
    assert(session != nullptr);
    m_fwdEncSessionDeletedReportCb(session);
}

void SessionImpl::sendDataToClient(const std::uint8_t* buf, std::size_t bufLen, unsigned brokerRadius)
{
    assert(m_sendToClientCb);
    m_sendToClientCb(buf, bufLen, brokerRadius);
}

void SessionImpl::sendToClient(const MqttsnMessage& msg, unsigned broadcastRadius)
{
    if (!m_sendToClientCb) {
        return;
    }

    m_mqttsnMsgData.resize(std::max(m_mqttsnMsgData.size(), m_mqttsnFrame.length(msg)));
    auto iter = comms::writeIteratorFor<MqttsnMessage>(m_mqttsnMsgData.data());
    [[maybe_unused]] auto es = m_mqttsnFrame.write(msg, iter, m_mqttsnMsgData.size());
    assert(es == comms::ErrorStatus::Success);
    auto writtenCount =
        static_cast<std::size_t>(
            std::distance(comms::writeIteratorFor<MqttsnMessage>(m_mqttsnMsgData.data()), iter));

    m_sendToClientCb(m_mqttsnMsgData.data(), writtenCount, broadcastRadius);
}

void SessionImpl::sendToBroker(const MqttMessage& msg)
{
    if (!m_sendToBrokerCb) {
        return;
    }

    m_mqttMsgData.resize(std::max(m_mqttMsgData.size(), m_mqttFrame.length(msg)));
    auto iter = comms::writeIteratorFor<MqttMessage>(m_mqttMsgData.data());
    [[maybe_unused]] auto es = m_mqttFrame.write(msg, iter, m_mqttMsgData.size());
    assert(es == comms::ErrorStatus::Success);
    auto writtenCount =
        static_cast<std::size_t>(
            std::distance(comms::writeIteratorFor<MqttMessage>(m_mqttMsgData.data()), iter));

    m_sendToBrokerCb(m_mqttMsgData.data(), writtenCount);
}

void SessionImpl::termRequest()
{
    if ((!m_termReqCb) || (m_state.m_terminating)) {
        return;
    }

    m_state.m_terminating = true;
}

void SessionImpl::brokerReconnectRequest()
{
    if ((!m_brokerReconnectReqCb) ||
        (m_state.m_reconnectingBroker) ||
        (m_state.m_terminating)) {
        return;
    }

    m_state.m_reconnectingBroker = true;
    m_brokerReconnectReqCb();
}

void SessionImpl::clientConnectedReport(const std::string& clientId)
{
    if (m_clientConnectedCb) {
        m_clientConnectedCb(clientId);
    }
}

void SessionImpl::connStatusUpdated()
{
    for (auto& op : m_ops) {
        op->connStatusUpdated();
    }
}

SessionImpl::AuthInfo SessionImpl::authInfoRequest(const std::string& clientId)
{
    if (!m_authInfoReqCb) {
        return AuthInfo();
    }

    return m_authInfoReqCb(clientId);
}

void SessionImpl::reportError(const char* str)
{
    if (!m_errorReportCb) {
        return;
    }

    m_errorReportCb(str);
}

void SessionImpl::handle(SearchgwMsg_SN& msg)
{
    GwinfoMsg_SN respMsg;
    auto& fields = respMsg.fields();
    auto& gwIdField = std::get<decltype(respMsg)::FieldIdx_gwId>(fields);
    gwIdField.value() = m_state.m_gwId;
    sendToClient(respMsg, msg.field_radius().value());

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

}  // namespace cc_mqttsn_gateway


