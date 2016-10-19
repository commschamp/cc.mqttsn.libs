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

#include "SessionOp.h"
#include "common.h"

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

class Connect : public SessionOp
{
    typedef SessionOp Base;

public:
    Connect(SessionState& sessionState);
    ~Connect();

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

    using Base::handle;
    virtual void handle(ConnectMsg_SN& msg) override;
    virtual void handle(WilltopicMsg_SN& msg) override;
    virtual void handle(WillmsgMsg_SN& msg) override;
    virtual void handle(PublishMsg_SN& msg) override;
    virtual void handle(ConnackMsg& msg) override;

    void doNextStep();
    void forwardConnectionReq();
    void processAck(mqtt::message::ConnackResponseCode respCode);
    void clearConnectionInfo();
    void clearInternalState();

    std::string m_clientId;
    WillInfo m_will;
    std::uint16_t m_keepAlive = 0;
    bool m_clean = false;
    State m_internalState;
};

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn


