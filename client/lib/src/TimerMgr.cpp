//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "TimerMgr.h"

#include "comms/Assert.h"

#include <algorithm>
#include <iterator>
#include <limits>

namespace cc_mqttsn_client
{

TimerMgr::Timer TimerMgr::allocTimer()
{
    auto createTimer = 
        [this](unsigned idx)
        {
            m_timers[idx].m_allocated = true;
            ++m_allocatedTimers;
            return Timer(*this, idx);
        };

    if (m_allocatedTimers < m_timers.size()) {
        auto iter = std::find_if(
            m_timers.begin(), m_timers.end(),
            [](auto& info)
            {
                return !info.m_allocated;
            });

        if (iter != m_timers.end()) {
            auto idx = static_cast<unsigned>(std::distance(m_timers.begin(), iter));
            return createTimer(idx);
        }

        [[maybe_unused]] static constexpr bool Should_not_happen = false;
        COMMS_ASSERT(Should_not_happen);
    } 

    if (m_timers.max_size() <= m_timers.size()) {
        return Timer(*this);
    }      

    auto idx = static_cast<unsigned>(m_timers.size());
    m_timers.resize(m_timers.size() + 1U);
    return createTimer(idx);
}

void TimerMgr::tick(unsigned ms)
{
    struct CbInfo
    {
        TimeoutCb m_timeoutCb = nullptr;
        void* m_timeoutData = nullptr;
    };

    using CbList = ObjListType<CbInfo, ExtConfig::TimersLimit>;
    CbList cbList;

    for (auto idx = 0U; idx < m_timers.size(); ++idx) {
        auto& info = m_timers[idx];
        if ((!info.m_allocated) || 
            (info.m_timeoutCb == nullptr) ||
            (info.m_suspended)) {
            continue;
        }

        if (info.m_timeoutMs <= ms) {
            cbList.push_back({info.m_timeoutCb, info.m_timeoutData});
            timerCancel(idx);
            continue;
        }

        info.m_timeoutMs -= ms;
    }

    for (auto& info : cbList) {
        info.m_timeoutCb(info.m_timeoutData);
    }
}

unsigned TimerMgr::getMinWait() const
{
    if (m_allocatedTimers == 0U) {
        return 0U;
    }

    static constexpr auto Limit = std::numeric_limits<std::uint64_t>::max();
    auto result = Limit;

    for (auto& info : m_timers) {
        if ((!info.m_allocated) || 
            (info.m_timeoutCb == nullptr) || 
            (info.m_suspended)) {
            continue;
        }

        result = std::min(result, info.m_timeoutMs);
    }

    if (result == Limit) {
        return 0U;
    }

    return static_cast<unsigned>(std::min(result, std::uint64_t(std::numeric_limits<unsigned>::max())));
}

unsigned TimerMgr::allocCount() const
{
    return 
        static_cast<unsigned>(
            std::count_if(
                m_timers.begin(), m_timers.end(), 
                [](auto& info)
                {
                    return info.m_allocated;
                }));
}

void TimerMgr::freeTimer(unsigned idx)
{
    COMMS_ASSERT(idx < m_timers.size()); 
    if (m_timers.size() <= idx) {
        return;
    }

    COMMS_ASSERT(m_allocatedTimers > 0U);
    auto& info = m_timers[idx];
    COMMS_ASSERT(info.m_allocated);
    info = TimerInfo();
    --m_allocatedTimers;
}

void TimerMgr::timerWait(unsigned idx, std::uint64_t timeoutMs, TimeoutCb cb, void* data)
{
    COMMS_ASSERT(idx < m_timers.size()); 
    if (m_timers.size() <= idx) {
        return;
    }

    auto& info = m_timers[idx];
    COMMS_ASSERT(info.m_allocated);
    COMMS_ASSERT(cb != nullptr);
    info.m_timeoutMs = timeoutMs;
    info.m_timeoutCb = cb;
    info.m_timeoutData = data;
}

void TimerMgr::timerCancel(unsigned idx)
{
    COMMS_ASSERT(idx < m_timers.size()); 
    if (m_timers.size() <= idx) {
        return;
    }

    auto& info = m_timers[idx];
    COMMS_ASSERT(info.m_allocated);
    info.m_timeoutMs = 0;
    info.m_timeoutCb = nullptr;
    info.m_timeoutData = nullptr;
}

bool TimerMgr::timerIsActive(unsigned idx) const
{
    COMMS_ASSERT(idx < m_timers.size()); 
    if (m_timers.size() <= idx) {
        return false;
    }

    auto& info = m_timers[idx];
    COMMS_ASSERT(info.m_allocated);
    COMMS_ASSERT(info.m_timeoutCb != nullptr || (info.m_timeoutMs == 0U));
    return (info.m_timeoutCb != nullptr);
}

void TimerMgr::timerSetSuspended(unsigned idx, bool suspended)
{
    COMMS_ASSERT(idx < m_timers.size()); 
    if (m_timers.size() <= idx) {
        return;
    }

    auto& info = m_timers[idx];
    COMMS_ASSERT(info.m_allocated);
    info.m_suspended = suspended;
}

bool TimerMgr::timerIsSuspended(unsigned idx) const
{
    COMMS_ASSERT(idx < m_timers.size()); 
    if (m_timers.size() <= idx) {
        return false;
    }

    auto& info = m_timers[idx];
    COMMS_ASSERT(info.m_allocated);
    return info.m_suspended;
}

} // namespace cc_mqttsn_client
