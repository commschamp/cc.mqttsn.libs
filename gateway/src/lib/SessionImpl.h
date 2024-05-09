//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <limits>

#include "cc_mqttsn_gateway/Session.h"
#include "MsgHandler.h"
#include "SessionOp.h"
#include "session_op/Encapsulate.h"
#include "common.h"
#include "comms/util/ScopeGuard.h"

namespace cc_mqttsn_gateway
{

class SessionImpl : public MsgHandler
{
    using Base = MsgHandler;
public:
    using AuthInfo = Session::AuthInfo;
    using NextTickProgramReqCb = Session::NextTickProgramReqCb;
    using SendDataReqCb = Session::SendDataReqCb;
    using CancelTickWaitReqCb = Session::CancelTickWaitReqCb;
    using TerminationReqCb = Session::TerminationReqCb;
    using BrokerReconnectReqCb = Session::BrokerReconnectReqCb;
    using ClientConnectedReportCb = Session::ClientConnectedReportCb;
    using AuthInfoReqCb = Session::AuthInfoReqCb;
    using FwdEncSessionCreatedReportCb = Session::FwdEncSessionCreatedReportCb;
    using FwdEncSessionDeletedReportCb = Session::FwdEncSessionDeletedReportCb;

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

    template <typename TFunc>
    void setClientConnectedReportCb(TFunc&& func)
    {
        m_clientConnectedCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setAuthInfoReqCb(TFunc&& func)
    {
        m_authInfoReqCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setFwdEncSessionCreatedReportCb(TFunc&& func)
    {
        m_fwdEncSessionCreatedReportCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setFwdEncSessionDeletedReportCb(TFunc&& func)
    {
        m_fwdEncSessionDeletedReportCb = std::forward<TFunc>(func);
    }    

    void setGatewayId(std::uint8_t value)
    {
        m_state.m_gwId = value;
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

    void setDefaultClientId(const std::string& value)
    {
        m_state.m_defaultClientId = value;
    }

    void setPubOnlyKeepAlive(std::uint16_t value)
    {
        m_state.m_pubOnlyKeepAlive = value;
    }

    bool start();

    void stop()
    {
        m_state.m_running = false;
    }

    bool isRunning() const
    {
        return m_state.m_running;
    }

    void tick();

    std::size_t dataFromClient(const std::uint8_t* buf, std::size_t len);
    std::size_t dataFromBroker(const std::uint8_t* buf, std::size_t len);

    void setBrokerConnected(bool connected);
    bool addPredefinedTopic(const std::string& topic, std::uint16_t topicId);
    bool setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal);

    // API used by ops
    SessionState& state()
    {
        return m_state;
    }

    bool hasFwdEncSupport() const
    {
        return static_cast<bool>(m_fwdEncSessionCreatedReportCb);
    }

    bool reportFwdEncSessionCreated(Session* session);
    void reportFwdEncSessionDeleted(Session* session);
    void sendDataToClient(const std::uint8_t* buf, std::size_t bufLen);
    void sendToClient(const MqttsnMessage& msg);
    void sendToBroker(const MqttMessage& msg);
    void termRequest();
    void brokerReconnectRequest();

private:

    using ReturnCodeVal = cc_mqttsn::field::ReturnCodeVal;
    using OpsList = std::vector<SessionOpPtr>;

    using Base::handle;
    virtual void handle(SearchgwMsg_SN& msg) override;
    virtual void handle(RegisterMsg_SN& msg) override;
    virtual void handle(MqttsnMessage& msg) override;

    virtual void handle(MqttMessage& msg) override;

    template <typename TMsg, typename TFrame>
    void sendMessage(const TMsg& msg, TFrame& frame, SendDataReqCb& func, DataBuf& buf);

    template <typename TMsg>
    void dispatchToOpsCommon(TMsg& msg);

    void dispatchToOps(MqttsnMessage& msg);
    void dispatchToOps(MqttMessage& msg);
    void programNextTimeout();
    void updateTimestamp();
    void updateOps();
    void apiCallExit();

#ifdef _MSC_VER
    typedef std::function<void ()> ApiCallGuard;
    auto apiCall() -> decltype(comms::util::makeScopeGuard(std::declval<ApiCallGuard>()));
#else
    auto apiCall() -> decltype(comms::util::makeScopeGuard(std::bind(&SessionImpl::apiCallExit, this)));
#endif

    NextTickProgramReqCb m_nextTickProgramCb;
    CancelTickWaitReqCb m_cancelTickCb;
    SendDataReqCb m_sendToClientCb;
    SendDataReqCb m_sendToBrokerCb;
    TerminationReqCb m_termReqCb;
    BrokerReconnectReqCb m_brokerReconnectReqCb;
    ClientConnectedReportCb m_clientConnectedCb;
    AuthInfoReqCb m_authInfoReqCb;
    FwdEncSessionCreatedReportCb m_fwdEncSessionCreatedReportCb;
    FwdEncSessionDeletedReportCb m_fwdEncSessionDeletedReportCb;

    MqttsnFrame m_mqttsnFrame;
    MqttFrame m_mqttFrame;

    DataBuf m_mqttsnMsgData;
    DataBuf m_mqttMsgData;

    OpsList m_ops;
    session_op::Encapsulate* m_encapsulateOp = nullptr;

    SessionState m_state;
};

}  // namespace cc_mqttsn_gateway


