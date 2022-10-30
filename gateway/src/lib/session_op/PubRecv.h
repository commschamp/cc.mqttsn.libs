//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "SessionOp.h"
#include "common.h"

namespace cc_mqttsn
{

namespace gateway
{

namespace session_op
{

class PubRecv : public SessionOp
{
    typedef SessionOp Base;

public:
    PubRecv(SessionState& sessionState);
    ~PubRecv();

protected:

private:
    using Base::handle;
    virtual void handle(PublishMsg& msg) override;
    virtual void handle(PubrelMsg& msg) override;

    struct BrokPubInfo
    {
        std::string m_topic;
        DataBuf m_msg;
        bool m_dup = false;
        bool m_retain = false;
        std::uint16_t m_packetId = 0U;
        Timestamp m_timestamp = 0U;
    };

    typedef std::list<BrokPubInfo> BrokPubInfosList;

    void addPubInfo(PubInfoPtr info);

    BrokPubInfosList m_recvMsgs;
};

}  // namespace session_op

}  // namespace gateway

}  // namespace cc_mqttsn


