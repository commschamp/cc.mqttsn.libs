//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Encapsulate.h"

#include <cassert>
#include <tuple>

#include "comms/util/assign.h"
#include "comms/util/ScopeGuard.h"

#include "SessionImpl.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

Encapsulate::Encapsulate(SessionImpl& session) :
    Base(session)
{
}

Encapsulate::~Encapsulate()
{
    for (auto& info : m_sessions) {
        session().reportFwdEncSessionDeleted(info.second.get());
    }
}

std::size_t Encapsulate::encapsulatedData(const std::uint8_t* buf, std::size_t len)
{
    auto& st = state();
    assert(st.m_encapsulatedMsg);
    st.m_encapsulatedMsg = false;

    auto onExit = 
        comms::util::makeScopeGuard(
            [this]()
            {
                m_selectedSession = nullptr;
            });

    if (m_selectedSession == nullptr) {
        return len;
    }

    return m_selectedSession->dataFromClient(buf, len);
}

void Encapsulate::handle(FwdMsg_SN& msg)
{
    auto& st = state();
    assert(!st.m_encapsulatedMsg);
    st.m_encapsulatedMsg = true;

    if (!session().hasFwdEncSupport()) {
        return;
    }

    auto& nodeIdVec = msg.field_nodeId().value();
    NodeId nodeId;
    comms::util::assign(nodeId, nodeIdVec.begin(), nodeIdVec.end());

    auto iter = m_sessions.find(nodeId);
    do {
        if (iter != m_sessions.end()) {
            break;
        }

        std::tie(iter, std::ignore) = m_sessions.insert(std::make_pair(nodeId, std::make_unique<Session>()));        
        assert(iter != m_sessions.end());
        auto sessionPtr = iter->second.get();

        sessionPtr->setGatewayId(st.m_gwId);
        sessionPtr->setRetryPeriod(st.m_retryPeriod);
        sessionPtr->setRetryCount(st.m_retryCount);
        sessionPtr->setSleepingClientMsgLimit(st.m_sleepPubAccLimit);
        sessionPtr->setDefaultClientId(st.m_defaultClientId);
        sessionPtr->setPubOnlyKeepAlive(st.m_pubOnlyKeepAlive);

        if (!session().reportFwdEncSessionCreated(sessionPtr)) {
            m_sessions.erase(iter);
            iter = m_sessions.end();
            break;
        }

        sessionPtr->setSendDataClientReqCb(
            [this, nodeId](const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius)
            {
                sendDataClientReqFromSession(nodeId, buf, bufSize, broadcastRadius);
            });     

        sessionPtr->setTerminationReqCb(
            [this, sessionPtr]()
            {
                terminationReqFromSession(sessionPtr);
            });        

        if ((!sessionPtr->isRunning()) && (!sessionPtr->start())) {
            // Error failed to start session;
            session().reportError("Failed to start forward encapsulated session");
            session().reportFwdEncSessionDeleted(sessionPtr);
            m_sessions.erase(iter);
            iter = m_sessions.end();
            break;
        }

    } while (false);

    if (iter != m_sessions.end()) {
        m_selectedSession = iter->second.get();
    }
}

void Encapsulate::sendDataClientReqFromSession(
    const NodeId& nodeId, 
    const std::uint8_t* buf, 
    std::size_t bufSize, 
    unsigned broadcastRadius)
{
    FwdMsg_SN fwdMsg;
    fwdMsg.field_ctrl().field_radius().setValue(3); // TODO: make it configurable
    comms::util::assign(fwdMsg.field_nodeId().value(), nodeId.begin(), nodeId.end());

    MqttsnFrame frame;
    std::vector<std::uint8_t> data;
    data.resize(frame.length(fwdMsg) + bufSize);
    auto writeIter = comms::writeIteratorFor<FwdMsg_SN>(data.data());
    [[maybe_unused]] auto es = frame.write(fwdMsg, writeIter, data.size());
    assert(es == comms::ErrorStatus::Success);
    std::copy_n(buf, bufSize, writeIter);
    session().sendDataToClient(data.data(), data.size(), broadcastRadius);
}

void Encapsulate::terminationReqFromSession(Session* sessionPtr)
{
    auto iter = 
        std::find_if(
            m_sessions.begin(), m_sessions.end(),
            [sessionPtr](auto& elem)
            {
                return elem.second.get() == sessionPtr;
            });

    assert(iter != m_sessions.end());
    if (iter == m_sessions.end()) {
        return;
    }

    session().reportFwdEncSessionDeleted(sessionPtr);
    m_sessions.erase(iter);
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
