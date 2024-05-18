//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include "cc_mqttsn_gateway/Session.h"

#include "SessionOp.h"
#include "common.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

class Encapsulate : public SessionOp
{
    using Base = SessionOp;

public:
    explicit Encapsulate(SessionImpl& session);
    ~Encapsulate();

    std::size_t encapsulatedData(const std::uint8_t* buf, std::size_t len);

private:
    using NodeId = std::vector<std::uint8_t>;
    using SessionPtr = std::unique_ptr<Session>;
    using SessionMap = std::map<NodeId, SessionPtr>;

    using Base::handle;
    virtual void handle(FwdMsg_SN& msg) override;

    void sendDataClientReqFromSession(const NodeId& nodeId, const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius);
    void terminationReqFromSession(Session* sessionPtr);

    SessionMap m_sessions;
    Session* m_selectedSession = nullptr;
};

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
