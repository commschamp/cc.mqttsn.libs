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


#pragma once

#include <functional>
#include <cassert>
#include <memory>

#include "mqttsn/gateway/Session.h"
#include "MsgHandler.h"
#include "common.h"

namespace mqttsn
{

namespace gateway
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
            (m_nextTickTimestamp <= m_state.m_timestamp) &&
            (!isComplete())) {
            m_nextTickTimestamp = 0;
            tickImpl();
        }
    }

    unsigned nextTick()
    {
        if ((m_nextTickTimestamp == 0) || (isComplete())) {
            return std::numeric_limits<unsigned>::max();
        }

        if (m_nextTickTimestamp <= m_state.m_timestamp) {
            return 1U;
        }

        return m_nextTickTimestamp - m_state.m_timestamp;
    }

    void start()
    {
        startImpl();
    }

    bool isComplete() const
    {
        return m_complete;
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

    void nextTickReq(unsigned ms)
    {
        m_nextTickTimestamp = m_state.m_timestamp + ms;
    }

    void cancelTick()
    {
        m_nextTickTimestamp = 0;
    }

    void setComplete()
    {
        m_complete = true;
    }

    SessionState& state()
    {
        return m_state;
    }

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
    bool m_complete = false;
};

typedef std::unique_ptr<SessionOp> SessionOpPtr;

}  // namespace gateway

}  // namespace mqttsn


