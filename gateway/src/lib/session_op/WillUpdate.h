//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

#include "SessionOp.h"
#include "common.h"

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

class WillUpdate : public SessionOp
{
    typedef SessionOp Base;

public:
    WillUpdate(SessionState& sessionState);
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

    using ReturnCodeVal = mqttsn::field::ReturnCodeVal;

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

}  // namespace gateway

}  // namespace mqttsn


