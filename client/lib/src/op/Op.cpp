//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "op/Op.h"

#include "ClientImpl.h"
#include "TopicFilterDefs.h"

#include "comms/util/ScopeGuard.h"
#include "comms/cast.h"

#include <algorithm>
#include <limits>
#include <type_traits>

namespace cc_mqttsn_client
{

namespace op
{

namespace 
{

static constexpr char TopicSep = '/';
static constexpr char MultLevelWildcard = '#';
static constexpr char SingleLevelWildcard = '+';

} // namespace 

bool Op::isValidTopicId(CC_MqttsnTopicId id)
{
    return (id != 0U) && (id != 0xffff);
}

Op::Op(ClientImpl& client) : 
    m_client(client),
    m_retryPeriod(client.configState().m_retryPeriod),
    m_retryCount(client.configState().m_retryCount)
{
}    

void Op::terminateOpImpl([[maybe_unused]] CC_MqttsnAsyncOpStatus status)
{
    opComplete();
}

CC_MqttsnAsyncOpStatus Op::translateErrorCodeToAsyncOpStatus(CC_MqttsnErrorCode ec)
{
    static const CC_MqttsnAsyncOpStatus Map[] = {
        /* CC_MqttsnErrorCode_Success */ CC_MqttsnAsyncOpStatus_Complete,
        /* CC_MqttsnErrorCode_InternalError */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_NotIntitialized */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_Busy */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_NotConnected */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_BadParam */ CC_MqttsnAsyncOpStatus_BadParam,
        /* CC_MqttsnErrorCode_InsufficientConfig */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_OutOfMemory */ CC_MqttsnAsyncOpStatus_OutOfMemory,
        /* CC_MqttsnErrorCode_BufferOverflow */ CC_MqttsnAsyncOpStatus_OutOfMemory,
        /* CC_MqttsnErrorCode_NotSupported */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_RetryLater */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_Disconnecting */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_NotSleeping */ CC_MqttsnAsyncOpStatus_InternalError,
        /* CC_MqttsnErrorCode_PreparationLocked */ CC_MqttsnAsyncOpStatus_InternalError,
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnErrorCode_ValuesLimit);

    auto idx = static_cast<unsigned>(ec);
    if (MapSize <= idx) {
        COMMS_ASSERT(false); // Should not happen
        return CC_MqttsnAsyncOpStatus_InternalError;
    }

    return Map[idx];
}

CC_MqttsnErrorCode Op::sendMessage(const ProtMessage& msg, unsigned broadcastRadius)
{
    return m_client.sendMessage(msg, broadcastRadius);
}

void Op::opComplete()
{
    m_client.opComplete(this);
}

std::uint16_t Op::allocPacketId()
{
    static constexpr auto MaxPacketId = std::numeric_limits<std::uint16_t>::max();
    auto& allocatedPacketIds = m_client.clientState().m_allocatedPacketIds;

    if ((allocatedPacketIds.max_size() <= allocatedPacketIds.size()) || 
        (MaxPacketId <= allocatedPacketIds.size())) {
        errorLog("No more available packet IDs for allocation");
        return 0U;
    }

    auto& lastPacketId = m_client.clientState().m_lastPacketId;
    auto nextPacketId = static_cast<std::uint16_t>(lastPacketId + 1U);

    while (true) {
        if ((nextPacketId == std::numeric_limits<decltype(nextPacketId)>::max()) ||
            (nextPacketId == 0U)) {
            nextPacketId = 1U;
        }
                
        if (allocatedPacketIds.empty() || (allocatedPacketIds.back() < nextPacketId)) {
            allocatedPacketIds.push_back(nextPacketId);
            break;
        }

        auto iter = std::lower_bound(allocatedPacketIds.begin(), allocatedPacketIds.end(), nextPacketId);
        if ((iter == allocatedPacketIds.end()) || (*iter != nextPacketId)) {
            allocatedPacketIds.insert(iter, nextPacketId);
            break;
        }

        ++nextPacketId;
    } 

    lastPacketId = static_cast<std::uint16_t>(nextPacketId);
    return lastPacketId;
}

void Op::releasePacketId(std::uint16_t id)
{
    if (id == 0U) {
        return;
    }
    
    auto& allocatedPacketIds = m_client.clientState().m_allocatedPacketIds;
    auto iter = std::lower_bound(allocatedPacketIds.begin(), allocatedPacketIds.end(), id);
    if ((iter == allocatedPacketIds.end()) || (*iter != id)) {
        [[maybe_unused]] static constexpr bool ShouldNotHappen = false;
        COMMS_ASSERT(ShouldNotHappen);
        return;
    }    

    allocatedPacketIds.erase(iter);
}

void Op::decRetryCount()
{
    COMMS_ASSERT(m_retryCount > 0U);
    --m_retryCount;
}

bool Op::isShortTopic(const char* topic)
{
    COMMS_ASSERT(topic != nullptr);
    for (auto idx = 0U; idx < sizeof(std::uint16_t); ++idx) {
        if (topic[idx] == '\0') {
            return false;
        }
    }

    return topic[sizeof(std::uint16_t)] == '\0';
}

void Op::errorLogInternal(const char* msg)
{
    if constexpr (Config::HasErrorLog) {
        m_client.errorLog(msg);
    }    
}

bool Op::verifySubFilterInternal(const char* filter)
{
    if (Config::HasTopicFormatVerification) {
        if (!m_client.configState().m_verifyOutgoingTopic) {
            return true;
        }

        COMMS_ASSERT(filter != nullptr);
        if (filter[0] == '\0') {
            return false;
        }

        auto pos = 0U;
        int lastSep = -1;
        while (filter[pos] != '\0') {
            auto incPosGuard = 
                comms::util::makeScopeGuard(
                    [&pos]()
                    {
                        ++pos;
                    });

            auto ch = filter[pos];

            if (ch == TopicSep) {
                comms::cast_assign(lastSep) = pos;
                continue;
            }   

            if (ch == MultLevelWildcard) {
                                
                if (filter[pos + 1] != '\0') {
                    errorLog("Multi-level wildcard \'#\' must be last.");
                    return false;
                }

                if (pos == 0U) {
                    return true;
                }

                if ((lastSep < 0) || (static_cast<decltype(lastSep)>(pos - 1U) != lastSep)) {
                    errorLog("Multi-level wildcard \'#\' must follow separator.");
                    return false;
                }

                return true;
            }

            if (ch != SingleLevelWildcard) {
                continue;
            }

            auto nextCh = filter[pos + 1];
            if ((nextCh != '\0') && (nextCh != TopicSep)) {
                errorLog("Single-level wildcard \'+\' must be last of followed by /.");
                return false;                
            }           

            if (pos == 0U) {
                continue;
            }

            if ((lastSep < 0) || (static_cast<decltype(lastSep)>(pos - 1U) != lastSep)) {
                errorLog("Single-level wildcard \'+\' must follow separator.");
                return false;
            }            
        }

        return true;
    }
    else {
        [[maybe_unused]] static constexpr bool ShouldNotBeCalled = false;
        COMMS_ASSERT(ShouldNotBeCalled);
        return false;
    }
}

} // namespace op

} // namespace cc_mqttsn_client
