//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <list>

#include "comms/util/ScopeGuard.h"
#include "SessionOp.h"
#include "common.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

class Forward : public SessionOp
{
    typedef SessionOp Base;

public:
    Forward(SessionState& sessionState);
    ~Forward();

protected:

private:
    using Base::handle;
    virtual void handle(PublishMsg_SN& msg) override;
    virtual void handle(PubrelMsg_SN& msg) override;
    virtual void handle(PingreqMsg_SN& msg) override;
    virtual void handle(PingrespMsg_SN& msg) override;
    virtual void handle(SubscribeMsg_SN& msg) override;
    virtual void handle(UnsubscribeMsg_SN& msg) override;

    virtual void handle(ConnackMsg& msg) override;
    virtual void handle(PubackMsg& msg) override;
    virtual void handle(PubrecMsg& msg) override;
    virtual void handle(PubcompMsg& msg) override;
    virtual void handle(PingrespMsg& msg) override;
    virtual void handle(SubackMsg& msg) override;
    virtual void handle(UnsubackMsg& msg) override;

    struct SubInfo
    {
        Timestamp m_timestamp = 0U;
        std::uint16_t m_msgId = 0;
        std::uint16_t m_topicId = 0;
    };

    typedef std::list<SubInfo> SubsInProgressList;

    struct NoGwPubInfo
    {
        std::uint16_t m_topicId = 0;
        DataBuf m_data;
    };

    typedef std::list<NoGwPubInfo> NoGwPubInfosList;

    using ReturnCodeVal = cc_mqttsn::field::ReturnCodeVal;
    using TopicIdTypeVal = cc_mqttsn::field::TopicIdTypeVal;
    void sendPubackToClient(
        std::uint16_t topicId,
        std::uint16_t msgId,
        ReturnCodeVal rc);


    std::uint16_t m_lastPubTopicId = 0;
    bool m_pingInProgress = false;
    SubsInProgressList m_subs;
    NoGwPubInfosList m_pubs;
};

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
