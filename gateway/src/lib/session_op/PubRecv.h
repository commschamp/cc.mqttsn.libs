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

    BrokPubInfosList m_recvMsgs;
};

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn


