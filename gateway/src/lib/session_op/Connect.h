//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "cc_mqttsn_gateway/Session.h"
#include "SessionOp.h"
#include "common.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

class Connect : public SessionOp
{
    typedef SessionOp Base;

public:
    typedef Session::AuthInfo AuthInfo;

    typedef Session::ClientConnectedReportCb ClientConnectedReportCb;
    typedef Session::AuthInfoReqCb AuthInfoReqCb;

    Connect(SessionState& sessionState);
    ~Connect();

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

protected:
    virtual void tickImpl() override;
    virtual void brokerConnectionUpdatedImpl() override;

private:
    struct State
    {
        unsigned m_attempt = 0;
        bool m_hasClientId = false;
        bool m_hasWillTopic = false;
        bool m_hasWillMsg = false;
        bool m_waitingForReconnect = false;
        bool m_pubOnlyClient = false;
    };

    using ReturnCodeVal = cc_mqttsn::field::ReturnCodeVal;

    using Base::handle;
    virtual void handle(ConnectMsg_SN& msg) override;
    virtual void handle(WilltopicMsg_SN& msg) override;
    virtual void handle(WillmsgMsg_SN& msg) override;
    virtual void handle(PublishMsg_SN& msg) override;
    virtual void handle(ConnackMsg& msg) override;

    void doNextStep();
    void forwardConnectionReq();
    void processAck(ConnackMsg::Field_returnCode::ValueType respCode);
    void clearConnectionInfo(bool clearClientId = false);
    void clearInternalState();

    std::string m_clientId;
    AuthInfo m_authInfo;
    WillInfo m_will;
    std::uint16_t m_keepAlive = 0;
    bool m_clean = false;
    State m_internalState;
    ClientConnectedReportCb m_clientConnectedCb;
    AuthInfoReqCb m_authInfoReqCb;
};

}  // namespace session_op

}  // namespace cc_mqttsn_gateway


