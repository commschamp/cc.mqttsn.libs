//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ExtConfig.h"
#include "ObjListType.h"

#include "comms/util/StaticVector.h"
#include "comms/util/type_traits.h"

#include <limits>

namespace cc_mqttsn_client
{

class TimerMgr
{
public:
    using TimeoutCb = void (*)(void*);

    class Timer
    {
    public:
        bool isValid() const
        {
            return m_idx < InvalidIdx;
        }

        ~Timer()
        {
            if (isValid()) {
                m_timerMgr.freeTimer(m_idx);
            }
        }

        void wait(std::uint64_t timeoutMs, TimeoutCb cb, void* data)
        {
            m_timerMgr.timerWait(m_idx, timeoutMs, cb, data);
        }

        void cancel()
        {
            m_timerMgr.timerCancel(m_idx);
        }

        bool isActive() const
        {
            return m_timerMgr.timerIsActive(m_idx);
        }

        void setSuspended(bool suspended)
        {
            if (!isActive()) {
                return;
            }

            m_timerMgr.timerSetSuspended(m_idx, suspended);
        }

        bool isSuspended() const
        {
            if (!isActive()) {
                return false;
            }

            return m_timerMgr.timerIsSuspended(m_idx);
        }

    private:
        Timer (TimerMgr& timerMgr, unsigned idx) :
            m_timerMgr(timerMgr),
            m_idx(idx)
        {
        }

        Timer (TimerMgr& timerMgr) :
            m_timerMgr(timerMgr)
        {
        }        

        TimerMgr& m_timerMgr;
        unsigned m_idx = InvalidIdx;

        friend class TimerMgr;

        static const unsigned InvalidIdx = std::numeric_limits<unsigned>::max();
    };

    Timer allocTimer();
    void tick(unsigned ms);
    unsigned getMinWait() const;
    unsigned allocCount() const;

private:
    struct TimerInfo
    {
        std::uint64_t m_timeoutMs = 0U;
        TimeoutCb m_timeoutCb = nullptr;
        void* m_timeoutData = nullptr;
        bool m_allocated = false;
        bool m_suspended = false;
    };

    using StorageType = ObjListType<TimerInfo, ExtConfig::TimersLimit>;

    friend class Timer;

    void freeTimer(unsigned idx);
    void timerWait(unsigned idx, std::uint64_t timeoutMs, TimeoutCb cb, void* data);
    void timerCancel(unsigned idx);
    bool timerIsActive(unsigned idx) const;
    void timerSetSuspended(unsigned idx, bool suspended);
    bool timerIsSuspended(unsigned idx) const;

    StorageType m_timers;
    unsigned m_allocatedTimers = 0U;
};

} // namespace cc_mqttsn_client
