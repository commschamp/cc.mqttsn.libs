//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/Op.h"

#include "ClientImpl.h"

#include "comms/util/ScopeGuard.h"
#include "comms/cast.h"

#include <algorithm>
#include <type_traits>

namespace cc_mqttsn_client
{

namespace op
{

namespace 
{

// static constexpr char TopicSep = '/';
// static constexpr char MultLevelWildcard = '#';
// static constexpr char SingleLevelWildcard = '+';

} // namespace 



Op::Op(ClientImpl& client) : 
    m_client(client),
    m_responseTimeoutMs(client.configState().m_responseTimeoutMs),
    m_retryCount(client.configState().m_retryCount)
{
}    

void Op::sendMessage(const ProtMessage& msg, unsigned broadcastRadius)
{
    m_client.sendMessage(msg, broadcastRadius);
}

void Op::terminateOpImpl([[maybe_unused]] CC_MqttsnAsyncOpStatus status)
{
    opComplete();
}

void Op::opComplete()
{
    m_client.opComplete(this);
}

// std::uint16_t Op::allocPacketId()
// {
//     static constexpr auto MaxPacketId = std::numeric_limits<std::uint16_t>::max();
//     auto& allocatedPacketIds = m_client.clientState().m_allocatedPacketIds;

//     if ((allocatedPacketIds.max_size() <= allocatedPacketIds.size()) || 
//         (MaxPacketId <= allocatedPacketIds.size())) {
//         errorLog("No more available packet IDs for allocation");
//         return 0U;
//     }

//     auto& lastPacketId = m_client.clientState().m_lastPacketId;
//     auto nextPacketId = static_cast<std::uint16_t>(lastPacketId + 1U);

//     if (nextPacketId == 0U) {
//         nextPacketId = 1U;
//     }
    
//     while (true) {
//         if (allocatedPacketIds.empty() || (allocatedPacketIds.back() < nextPacketId)) {
//             allocatedPacketIds.push_back(nextPacketId);
//             break;
//         }

//         auto iter = std::lower_bound(allocatedPacketIds.begin(), allocatedPacketIds.end(), nextPacketId);
//         if ((iter == allocatedPacketIds.end()) || (*iter != nextPacketId)) {
//             allocatedPacketIds.insert(iter, nextPacketId);
//             break;
//         }

//         ++nextPacketId;
//     } 

//     lastPacketId = static_cast<std::uint16_t>(nextPacketId);
//     return lastPacketId;
// }

// void Op::releasePacketId(std::uint16_t id)
// {
//     if (id == 0U) {
//         return;
//     }
    
//     auto& allocatedPacketIds = m_client.clientState().m_allocatedPacketIds;
//     auto iter = std::lower_bound(allocatedPacketIds.begin(), allocatedPacketIds.end(), id);
//     if ((iter == allocatedPacketIds.end()) || (*iter != id)) {
//         [[maybe_unused]] static constexpr bool ShouldNotHappen = false;
//         COMMS_ASSERT(ShouldNotHappen);
//         return;
//     }    

//     allocatedPacketIds.erase(iter);
// }

void Op::errorLogInternal(const char* msg)
{
    if constexpr (Config::HasErrorLog) {
        m_client.errorLog(msg);
    }    
}

// bool Op::verifySubFilterInternal(const char* filter)
// {
//     if (Config::HasTopicFormatVerification) {
//         if (!m_client.configState().m_verifyOutgoingTopic) {
//             return true;
//         }

//         COMMS_ASSERT(filter != nullptr);
//         if (filter[0] == '\0') {
//             return false;
//         }

//         auto pos = 0U;
//         int lastSep = -1;
//         while (filter[pos] != '\0') {
//             auto incPosGuard = 
//                 comms::util::makeScopeGuard(
//                     [&pos]()
//                     {
//                         ++pos;
//                     });

//             auto ch = filter[pos];

//             if (ch == TopicSep) {
//                 comms::cast_assign(lastSep) = pos;
//                 continue;
//             }   

//             if (ch == MultLevelWildcard) {
                                
//                 if (filter[pos + 1] != '\0') {
//                     errorLog("Multi-level wildcard \'#\' must be last.");
//                     return false;
//                 }

//                 if (pos == 0U) {
//                     return true;
//                 }

//                 if ((lastSep < 0) || (static_cast<decltype(lastSep)>(pos - 1U) != lastSep)) {
//                     errorLog("Multi-level wildcard \'#\' must follow separator.");
//                     return false;
//                 }

//                 return true;
//             }

//             if (ch != SingleLevelWildcard) {
//                 continue;
//             }

//             auto nextCh = filter[pos + 1];
//             if ((nextCh != '\0') && (nextCh != TopicSep)) {
//                 errorLog("Single-level wildcard \'+\' must be last of followed by /.");
//                 return false;                
//             }           

//             if (pos == 0U) {
//                 continue;
//             }

//             if ((lastSep < 0) || (static_cast<decltype(lastSep)>(pos - 1U) != lastSep)) {
//                 errorLog("Single-level wildcard \'+\' must follow separator.");
//                 return false;
//             }            
//         }

//         return true;
//     }
//     else {
//         [[maybe_unused]] static constexpr bool ShouldNotBeCalled = false;
//         COMMS_ASSERT(ShouldNotBeCalled);
//         return false;
//     }
// }

// bool Op::verifyPubTopicInternal(const char* topic, bool outgoing)
// {
//     if (Config::HasTopicFormatVerification) {
//         if (outgoing && (!m_client.configState().m_verifyOutgoingTopic)) {
//             return true;
//         }

//         if ((!outgoing) && (!m_client.configState().m_verifyIncomingTopic)) {
//             return true;
//         }

//         COMMS_ASSERT(topic != nullptr);
//         if (topic[0] == '\0') {
//             return false;
//         }

//         if (outgoing && (topic[0] == '$')) {
//             errorLog("Cannot start topic with \'$\'.");
//             return false;
//         }

//         auto pos = 0U;
//         while (topic[pos] != '\0') {
//             auto incPosGuard = 
//                 comms::util::makeScopeGuard(
//                     [&pos]()
//                     {
//                         ++pos;
//                     });

//             auto ch = topic[pos];

//             if ((ch == MultLevelWildcard) || 
//                 (ch == SingleLevelWildcard)) {
//                 errorLog("Wildcards cannot be used in publish topic");
//                 return false;
//             }
//         }

//         return true;
//     }
//     else {
//         [[maybe_unused]] static constexpr bool ShouldNotBeCalled = false;
//         COMMS_ASSERT(ShouldNotBeCalled);
//         return false;
//     }
// }

} // namespace op

} // namespace cc_mqttsn_client
