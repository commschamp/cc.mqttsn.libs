//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "SessionOp.h"
#include "common.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

class WillUpdate final : public SessionOp
{
    typedef SessionOp Base;

public:
    explicit WillUpdate(SessionImpl& session);
    ~WillUpdate();

protected:
    virtual void tickImpl() override;
    virtual void brokerConnectionUpdatedImpl() override;

private:
    enum class Op
    {
        None,
        TopicUpd,
        MsgUpd
    };

    using ReturnCodeVal = cc_mqttsn::field::ReturnCodeVal;

    using Base::handle;
    virtual void handle(ConnectMsg_SN& msg) override;
    virtual void handle(DisconnectMsg_SN& msg) override;
    virtual void handle(WilltopicupdMsg_SN& msg) override;
    virtual void handle(WillmsgupdMsg_SN& msg) override;
    virtual void handle(ConnackMsg& msg) override;

    void startOp(Op op);
    void doNextStage();
    void cancelOp();
    void sendTopicResp(ReturnCodeVal rc);
    void sendMsgResp(ReturnCodeVal rc);
    void sendResp(ReturnCodeVal rc);
    void sendConnectMsg();
    void sendFailureAndTerm();

    Op m_op = Op::None;
    WillInfo m_will;
    bool m_reconnectRequested = false;
    bool m_brokerConnectSent = false;
};

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
