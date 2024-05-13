//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <cassert>
#include <memory>

#include "cc_mqttsn_gateway/Session.h"
#include "MsgHandler.h"
#include "common.h"

namespace cc_mqttsn_gateway
{

class SessionImpl;
class SessionOp : public MsgHandler
{
    typedef MsgHandler Base;
public:

    typedef unsigned long long Timestamp;

    virtual ~SessionOp() = default;

    void timestampUpdated();
    
    unsigned nextTick();

    void start()
    {
        startImpl();
    }

    void brokerConnectionUpdated()
    {
        brokerConnectionUpdatedImpl();
    }

protected:
    SessionOp(SessionImpl& session)
      : m_session(session)
    {
    }

    void sendToClient(const MqttsnMessage& msg);
    void sendToBroker(const MqttMessage& msg);
    void termRequest();
    void brokerReconnectRequest();

    void nextTickReq(unsigned ms)
    {
        m_nextTickTimestamp = state().m_timestamp + ms;
    }

    void cancelTick()
    {
        m_nextTickTimestamp = 0;
    }

    SessionImpl& session()
    {
        return m_session;
    }

    SessionState& state();

    void sendDisconnectToClient();

    virtual void tickImpl();
    virtual void startImpl();
    virtual void brokerConnectionUpdatedImpl();

private:
    SessionImpl& m_session;
    Timestamp m_nextTickTimestamp = 0;
};

typedef std::unique_ptr<SessionOp> SessionOpPtr;

}  // namespace cc_mqttsn_gateway
