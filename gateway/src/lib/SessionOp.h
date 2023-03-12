//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
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

class SessionOp : public MsgHandler
{
    typedef MsgHandler Base;
public:

    typedef std::function<void (const MqttsnMessage&)> SendToClientCb;
    typedef std::function<void (const MqttMessage&)> SendToBrokerCb;
    typedef std::function<void ()> SessionTermReqCb;
    typedef std::function<void ()> BrokerReconnectReqCb;
    typedef unsigned long long Timestamp;

    virtual ~SessionOp() = default;

    template <typename TFunc>
    void setSendToClientCb(TFunc&& func)
    {
        m_sendToClientFunc = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setSendToBrokerCb(TFunc&& func)
    {
        m_sendToBrokerFunc = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setSessionTermReqCb(TFunc&& func)
    {
        m_termReqFunc = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setBrokerReconnectReqCb(TFunc&& func)
    {
        m_brokerReconnectReqFunc = std::forward<TFunc>(func);
    }

    void timestampUpdated()
    {
        if ((m_nextTickTimestamp != 0) &&
            (m_nextTickTimestamp <= m_state.m_timestamp)) {
            m_nextTickTimestamp = 0;
            tickImpl();
        }
    }

    unsigned nextTick()
    {
        if (m_nextTickTimestamp == 0) {
            return std::numeric_limits<unsigned>::max();
        }

        if (m_nextTickTimestamp <= m_state.m_timestamp) {
            return 1U;
        }

        return static_cast<unsigned>(m_nextTickTimestamp - m_state.m_timestamp);
    }

    void start()
    {
        startImpl();
    }

    void brokerConnectionUpdated()
    {
        brokerConnectionUpdatedImpl();
    }

protected:
    SessionOp(SessionState& state)
      : m_state(state)
    {
    }

    void sendToClient(const MqttsnMessage& msg)
    {
        assert(m_sendToClientFunc);
        m_sendToClientFunc(msg);
    }

    void sendToBroker(const MqttMessage& msg)
    {
        assert(m_sendToBrokerFunc);
        m_sendToBrokerFunc(msg);
    }

    void termRequest()
    {
        assert(m_termReqFunc);
        m_termReqFunc();
    }

    void brokerReconnectRequest()
    {
        assert(m_brokerReconnectReqFunc);
        m_brokerReconnectReqFunc();
    }

    void nextTickReq(unsigned ms)
    {
        m_nextTickTimestamp = m_state.m_timestamp + ms;
    }

    void cancelTick()
    {
        m_nextTickTimestamp = 0;
    }

    SessionState& state()
    {
        return m_state;
    }

    void sendDisconnectToClient();

    virtual void tickImpl() {};
    virtual void startImpl() {};
    virtual void brokerConnectionUpdatedImpl() {}

private:
    SessionState& m_state;

    SendToClientCb m_sendToClientFunc;
    SendToBrokerCb m_sendToBrokerFunc;
    SessionTermReqCb m_termReqFunc;
    BrokerReconnectReqCb m_brokerReconnectReqFunc;
    Timestamp m_nextTickTimestamp = 0;
};

typedef std::unique_ptr<SessionOp> SessionOpPtr;

}  // namespace cc_mqttsn_gateway
