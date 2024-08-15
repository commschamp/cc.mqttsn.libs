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

class PubSend final : public SessionOp
{
    typedef SessionOp Base;

public:
    explicit PubSend(SessionImpl& session);
    ~PubSend();

protected:
    virtual void tickImpl() override;
private:
    typedef RegMgr::TopicInfo TopicInfo;
    using ReturnCodeVal = cc_mqttsn::field::ReturnCodeVal;

    using Base::handle;
    virtual void handle(RegackMsg_SN& msg) override;
    virtual void handle(PubackMsg_SN& msg) override;
    virtual void handle(PubrecMsg_SN& msg) override;
    virtual void handle(PubcompMsg_SN& msg) override;
    virtual void handle(PingreqMsg_SN& msg) override;
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
    unsigned m_registerCount = 0U;
    bool m_registered = false;
    bool m_acked = false;
    bool m_ping = false;
    bool m_disconnectAlreadySent = false;
};

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
