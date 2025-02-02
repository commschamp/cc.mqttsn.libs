//
// Copyright 2016 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Disconnect.h"

#include "SessionImpl.h"

#include <cassert>

namespace cc_mqttsn_gateway
{

namespace session_op
{

Disconnect::Disconnect(SessionImpl& session) :
    Base(session)
{
}

Disconnect::~Disconnect() = default;

void Disconnect::brokerConnectionUpdatedImpl()
{
    auto& st = state();
    if (st.m_brokerConnected || state().m_reconnectingBroker) {
        return;
    }

    if (st.m_connStatus == ConnectionStatus::Asleep) {
        st.m_pendingClientDisconnect = true;
        return;
    }

    if (st.m_connStatus == ConnectionStatus::Connected) {
        sendDisconnectSn();
    }

    termRequest();
}

void Disconnect::handle(DisconnectMsg_SN& msg)
{
    if (msg.field_duration().doesExist()) {
        // Monitor disconnect with duration in Asleep op
        return;
    }

    sendToBroker(DisconnectMsg());
    sendDisconnectSn();
    termRequest();
}

void Disconnect::sendDisconnectSn()
{
    Base::sendDisconnectToClient();
    state().m_connStatus = ConnectionStatus::Disconnected;
    session().connStatusUpdated();
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway



