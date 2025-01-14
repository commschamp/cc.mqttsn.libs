//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "SessionOp.h"

#include "SessionImpl.h"

namespace cc_mqttsn_gateway
{

void SessionOp::timestampUpdated()
{
    if ((m_nextTickTimestamp != 0) &&
        (m_nextTickTimestamp <= state().m_timestamp)) {
        m_nextTickTimestamp = 0;
        tickImpl();
    }
}    

void SessionOp::sendToClient(const MqttsnMessage& msg)
{
    m_session.sendToClient(msg);
}

void SessionOp::sendToBroker(const MqttMessage& msg)
{
    m_session.sendToBroker(msg);
}

void SessionOp::termRequest()
{
    m_session.termRequest();
}

void SessionOp::brokerReconnectRequest()
{
    m_session.brokerReconnectRequest();
}

unsigned SessionOp::nextTick()
{
    if (m_nextTickTimestamp == 0) {
        return std::numeric_limits<unsigned>::max();
    }

    if (m_nextTickTimestamp <= state().m_timestamp) {
        return 1U;
    }

    return static_cast<unsigned>(m_nextTickTimestamp - state().m_timestamp);
}    

SessionState& SessionOp::state()
{
    return m_session.state();
}    

void SessionOp::sendDisconnectToClient()
{
    DisconnectMsg_SN msg;
    auto& fields = msg.fields();
    auto& durationField = std::get<decltype(msg)::FieldIdx_duration>(fields);
    durationField.setMode(comms::field::OptionalMode::Missing);
    sendToClient(msg);

}

void SessionOp::tickImpl() 
{
}

void SessionOp::startImpl() 
{
}

void SessionOp::brokerConnectionUpdatedImpl() 
{
}

void SessionOp::connStatusUpdatedImpl()
{
}

}  // namespace cc_mqttsn_gateway
