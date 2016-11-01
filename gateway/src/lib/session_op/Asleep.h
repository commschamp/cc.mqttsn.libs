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

class Asleep : public SessionOp
{
    typedef SessionOp Base;

public:
    Asleep(SessionState& sessionState);
    ~Asleep();

protected:
    virtual void tickImpl() override;
    virtual void brokerConnectionUpdatedImpl() override;

private:
    using Base::handle;
    virtual void handle(DisconnectMsg_SN& msg) override;
    virtual void handle(MqttsnMessage& msg) override;
    virtual void handle(PingrespMsg& msg) override;
    virtual void handle(MqttMessage& msg) override;

    void doPing();
    void reqNextTick();

    unsigned m_attempt = 0;
    Timestamp m_lastReq = 0;
    Timestamp m_lastResp = 0;
};

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn


