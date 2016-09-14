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

namespace mqttsn
{

namespace gateway
{

class SessionOp : public MsgHandler
{
    typedef MsgHandler Base;
public:
    enum class Type
    {
        Connect
    };

    typedef std::function<void (const MqttsnMessage&)> SendToClientCb;
    typedef std::function<void (const MqttMessage&)> SendToBrokerCb;

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

    void tick(unsigned ms)
    {
        m_timestamp += ms;
        if ((m_nextTickTimestamp != 0) && (m_nextTickTimestamp <= m_timestamp)) {
            tickImpl();
        }
    }

    unsigned nextTick()
    {
        return m_nextTickTimestamp - m_timestamp;
    }

    Type type() const
    {
        return typeImpl();
    }

    void start()
    {
        startImpl();
    }

    bool isComplete() const
    {
        return m_complete;
    }

    void setRetryPeriod(unsigned val)
    {
        m_retryPeriod = val;
    }

    void setRetryCount(unsigned val)
    {
        m_retryCount = val;
    }

protected:
    SessionOp() = default;
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

    void nextTickReq(unsigned ms)
    {
        m_nextTickTimestamp = m_timestamp + ms;
    }

    void setComplete()
    {
        m_complete = true;
    }

    unsigned retryPeriod() const
    {
        return m_retryPeriod;
    }

    unsigned retryCount() const
    {
        return m_retryCount;
    }

    virtual void tickImpl() {};
    virtual Type typeImpl() const = 0;
    virtual void startImpl() {};

private:
    SendToClientCb m_sendToClientFunc;
    SendToBrokerCb m_sendToBrokerFunc;
    unsigned m_timestamp = 0;
    unsigned m_nextTickTimestamp = 0;
    unsigned m_retryPeriod = 0;
    unsigned m_retryCount = 0;
    bool m_complete = false;
};

typedef std::unique_ptr<SessionOp> SessionOpPtr;

}  // namespace gateway

}  // namespace mqttsn


