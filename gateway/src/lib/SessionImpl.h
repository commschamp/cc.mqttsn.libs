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

#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <limits>

#include "mqttsn/gateway/Session.h"
#include "MsgHandler.h"
#include "SessionOp.h"
#include "common.h"
#include "comms/util/ScopeGuard.h"

namespace mqttsn
{

namespace gateway
{

class SessionImpl : public MsgHandler
{
    typedef MsgHandler Base;
public:
    typedef Session::NextTickProgramReqCb NextTickProgramReqCb;
    typedef Session::SendDataReqCb SendDataReqCb;
    typedef Session::CancelTickWaitReqCb CancelTickWaitReqCb;
    typedef Session::TerminationReqCb TerminationReqCb;
    typedef Session::BrokerReconnectReqCb BrokerReconnectReqCb;

    SessionImpl();
    ~SessionImpl() = default;


    template <typename TFunc>
    void setNextTickProgramReqCb(TFunc&& func)
    {
        m_nextTickProgramCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setCancelTickWaitReqCb(TFunc&& func)
    {
        m_cancelTickCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setSendDataClientReqCb(TFunc&& func)
    {
        m_sendToClientCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setSendDataBrokerReqCb(TFunc&& func)
    {
        m_sendToBrokerCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setTerminationReqCb(TFunc&& func)
    {
        m_termReqCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setBrokerReconnectReqCb(TFunc&& func)
    {
        m_brokerReconnectReqCb = std::forward<TFunc>(func);
    }

    void setGatewayId(std::uint8_t value)
    {
        m_state.m_gwId = value;
    }

    void setAuthInfo(const std::string& username, const std::uint8_t* password, std::size_t passLen)
    {
        m_state.m_username = username;
        m_state.m_password.assign(password, password + passLen);
    }

    void setAuthInfo(const char* username, const std::uint8_t* password, std::size_t passLen)
    {
        m_state.m_username = username;
        m_state.m_password.assign(password, password + passLen);
    }

    void setRetryPeriod(unsigned value)
    {
        m_state.m_retryPeriod = std::min(std::numeric_limits<unsigned>::max() / 1000, value) * 1000;
    }

    void setRetryCount(unsigned value)
    {
        m_state.m_retryCount = value;
    }

    void setSleepingClientMsgLimit(std::size_t value)
    {
        m_state.m_sleepPubAccLimit = std::min(m_state.m_brokerPubs.max_size(), value);
    }

    void setPubOnlyClientId(const std::string& value)
    {
        m_state.m_pubOnlyClientId = value;
    }

    void setPubOnlyKeepAlive(std::uint16_t value)
    {
        m_state.m_pubOnlyKeepAlive = value;
    }

    bool start()
    {
        if ((m_state.m_running) ||
            (!m_nextTickProgramCb) ||
            (!m_cancelTickCb) ||
            (!m_sendToClientCb) ||
            (!m_sendToBrokerCb) ||
            (!m_termReqCb) ||
            (!m_brokerReconnectReqCb)) {
            return false;
        }

        m_state.m_running = true;
        return true;
    }

    void stop()
    {
        m_state.m_running = false;
    }

    bool isRunning() const
    {
        return m_state.m_running;
    }

    void tick(unsigned ms);

    std::size_t dataFromClient(const std::uint8_t* buf, std::size_t len);
    std::size_t dataFromBroker(const std::uint8_t* buf, std::size_t len);

    void setBrokerConnected(bool connected);
    bool addPredefinedTopic(const std::string& topic, std::uint16_t topicId);
    bool setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal);

private:

    typedef std::vector<SessionOpPtr> OpsList;

    using Base::handle;
    virtual void handle(SearchgwMsg_SN& msg) override;
    virtual void handle(RegisterMsg_SN& msg) override;
    virtual void handle(MqttsnMessage& msg) override;

    virtual void handle(MqttMessage& msg) override;

    template <typename TStack>
    std::size_t processInputData(const std::uint8_t* buf, std::size_t len, TStack& stack);

    template <typename TMsg, typename TStack>
    void sendMessage(const TMsg& msg, TStack& stack, SendDataReqCb& func, DataBuf& buf);

    template <typename TMsg>
    void dispatchToOpsCommon(TMsg& msg);

    void sendToClient(const MqttsnMessage& msg);
    void sendToBroker(const MqttMessage& msg);
    void startOp(SessionOp& op);
    void dispatchToOps(MqttsnMessage& msg);
    void dispatchToOps(MqttMessage& msg);
    void programNextTimeout();
    void updateTimestamp();
    void updateOps();
    void apiCallExit();
    auto apiCall() -> decltype(comms::util::makeScopeGuard(std::bind(&SessionImpl::apiCallExit, this)));

    NextTickProgramReqCb m_nextTickProgramCb;
    CancelTickWaitReqCb m_cancelTickCb;
    SendDataReqCb m_sendToClientCb;
    SendDataReqCb m_sendToBrokerCb;
    TerminationReqCb m_termReqCb;
    BrokerReconnectReqCb m_brokerReconnectReqCb;

    MqttsnProtStack m_mqttsnStack;
    MqttProtStack m_mqttStack;

    DataBuf m_mqttsnMsgData;
    DataBuf m_mqttMsgData;

    OpsList m_ops;

    SessionState m_state;
};

}  // namespace gateway

}  // namespace mqttsn


