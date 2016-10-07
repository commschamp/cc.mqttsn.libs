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

class PubSend : public SessionOp
{
    typedef SessionOp Base;

public:
    PubSend(SessionState& sessionState);
    ~PubSend();

protected:
    virtual void tickImpl() override;
private:
    typedef RegMgr::TopicInfo TopicInfo;

    using Base::handle;
    virtual void handle(RegackMsg_SN& msg) override;
    virtual void handle(PubackMsg_SN& msg) override;
    virtual void handle(PubrecMsg_SN& msg) override;
    virtual void handle(PubcompMsg_SN& msg) override;
    virtual void handle(MqttsnMessage& msg) override;
    virtual void handle(MqttMessage& msg) override;

    void newSends();
    void sendCurrent();
    void doSend();
    unsigned allocMsgId();
    void sendDisconnect();
    void sendRegister();
    void sendPubrel();
    void checkSend();

    unsigned m_attempt = 0;
    unsigned m_nextMsgId = 0;
    PubInfoPtr m_currPub;
    TopicInfo m_currTopicInfo;
    std::uint16_t m_currMsgId = 0;
    bool m_registered = false;
    bool m_acked = false;
};

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn


