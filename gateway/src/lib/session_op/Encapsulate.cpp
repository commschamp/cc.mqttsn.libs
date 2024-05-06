//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Encapsulate.h"

#include <cassert>
#include <tuple>

#include "comms/util/assign.h"
#include "comms/util/ScopeGuard.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

Encapsulate::Encapsulate(SessionState& sessionState)
  : Base(sessionState)
{
}

Encapsulate::~Encapsulate() = default;

std::size_t Encapsulate::encapsulatedData(const std::uint8_t* buf, std::size_t len)
{
    auto& st = state();
    assert(st.m_encapsulatedMsg);
    st.m_encapsulatedMsg = false;

    auto onExit = 
        comms::util::makeScopeGuard(
            [this]()
            {
                m_selectedSession = nullptr;
            });

    if (m_selectedSession == nullptr) {
        return len;
    }

    return m_selectedSession->dataFromClient(buf, len);
    return 0U;
}

void Encapsulate::handle(FwdMsg_SN& msg)
{
    auto& st = state();
    assert(!st.m_encapsulatedMsg);
    st.m_encapsulatedMsg = true;

    auto& nodeIdVec = msg.field_data().value();
    NodeId nodeId;
    comms::util::assign(nodeId, nodeIdVec.begin(), nodeIdVec.end());

    auto iter = m_sessions.find(nodeId);
    do {
        if (iter != m_sessions.end()) {
            break;
        }

        std::tie(iter, std::ignore) = m_sessions.insert(std::make_pair(std::move(nodeId), std::make_unique<Session>()));        
        assert(iter != m_sessions.end());
        auto& session = *(iter->second);

        // TODO: callbacks

        if (!session.start()) {
            // Error failed to start session;
            assert(false); // Should not happen
            m_sessions.erase(iter);
            iter = m_sessions.end();
            break;
        }

        // TODO: report session for app management

    } while (false);

    if (iter != m_sessions.end()) {
        m_selectedSession = iter->second.get();
    }
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
