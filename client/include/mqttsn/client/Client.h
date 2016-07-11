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

#include <type_traits>
#include <vector>
#include <algorithm>
#include <iterator>
#include <limits>

#include "comms/comms.h"
#include "mqttsn/protocol/Stack.h"
#include "Message.h"
#include "AllMessages.h"

namespace mqttsn
{

namespace client
{

namespace details
{

template <typename TInfo, typename TOpts, bool THasTrackedGatewaysLimit>
struct GwInfoStorageType;

template <typename TInfo, typename TOpts>
struct GwInfoStorageType<TInfo, TOpts, true>
{
    typedef comms::util::StaticVector<TInfo, TOpts::TrackedGatewaysLimit> Type;
};

template <typename TInfo, typename TOpts>
struct GwInfoStorageType<TInfo, TOpts, false>
{
    typedef std::vector<TInfo> Type;
};

template <typename TInfo, typename TOpts>
using GwInfoStorageTypeT =
    typename GwInfoStorageType<TInfo, TOpts, TOpts::HasTrackedGatewaysLimit>::Type;

}  // namespace details

template <
    typename THandler,
    typename TClientOpts,
    typename TProtOpts>
class Client : public THandler
{
    typedef THandler Base;

public:
    typedef Message::Field FieldBase;
    typedef typename mqttsn::protocol::field::GwId<FieldBase>::ValueType GwIdValueType;
    typedef typename mqttsn::protocol::field::GwAdd<FieldBase, TProtOpts>::ValueType GwAddValueType;
    typedef typename mqttsn::protocol::field::Duration<FieldBase>::ValueType DurationValueType;
    typedef unsigned long long Timestamp;

    struct GwInfo
    {
        GwInfo() = default;

        Timestamp m_timestamp = 0;
        GwIdValueType m_id = 0;
        GwAddValueType m_addr;
        DurationValueType m_duration = 0;
    };

    typedef details::GwInfoStorageTypeT<GwInfo, TClientOpts> GwInfoStorage;

    typedef mqttsn::protocol::message::Advertise<Message> AdvertiseMsg;
    typedef mqttsn::protocol::message::Gwinfo<Message, TProtOpts> GwinfoMsg;
    Client() = default;
    virtual ~Client() = default;

    typedef Message::ReadIterator ReadIterator;

    const GwInfoStorage& gwInfos() const
    {
        return m_gwInfos;
    }

    void setGwAdvertisePeriod(unsigned val)
    {
        m_advertisePeriod = val;
    }

    unsigned getGwAdvertisePeriod() const
    {
        return m_advertisePeriod;
    }

    void tick(unsigned ms)
    {
        m_timerActive = false;
        m_timestamp += ms;

        checkTimeouts();
        programNextTimeout();
    }

    std::size_t processData(ReadIterator& iter, std::size_t len)
    {
        auto iterOrig = iter;
        bool timerWasActive = updateTimestamp();
        while (true) {
            auto iterTmp = iter;
            MsgPtr msg;
            auto es = m_stack.read(msg, iterTmp, len);
            if (es == comms::ErrorStatus::NotEnoughData) {
                break;
            }

            if (es != comms::ErrorStatus::Success) {
                ++iter;
                continue;
            }

            GASSERT(msg);
            msg->dispatch(*this);
            iter = iterTmp;
        }

        if (timerWasActive) {
            programNextTimeout();
        }

        return static_cast<std::size_t>(std::distance(iterOrig, iter));
    }

    using Base::handle;
    virtual void handle(AdvertiseMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& idField = std::get<AdvertiseMsg::FieldIdx_gwId>(fields);
        auto iter = findGwInfo(idField.value());
        if (iter == m_gwInfos.end()) {
            return;
        }

        auto& durationField = std::get<AdvertiseMsg::FieldIdx_duration>(fields);
        iter->m_timestamp = m_timestamp;
        iter->m_duration = durationField.value();
    }

    virtual void handle(GwinfoMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& idField = std::get<GwinfoMsg::FieldIdx_gwId>(fields);
        auto& addrField = std::get<GwinfoMsg::FieldIdx_gwAdd>(fields);
        auto iter = findGwInfo(idField.value());
        if (iter != m_gwInfos.end()) {
            iter->m_addr = addrField.value();
            return;
        }

        if (m_gwInfos.max_size() <= m_gwInfos.size()) {
            return;
        }

        m_gwInfos.emplace_back();
        auto& newElem = m_gwInfos.back();
        newElem.m_timestamp = m_timestamp;
        newElem.m_addr = addrField.value();
        newElem.m_duration = m_advertisePeriod;
    }

private:

    typedef protocol::Stack<Message, AllMessages<TProtOpts>, comms::option::InPlaceAllocation> ProtStack;
    typedef typename ProtStack::MsgPtr MsgPtr;

    typename GwInfoStorage::iterator findGwInfo(GwIdValueType id)
    {
        return
            std::find_if(
                m_gwInfos.begin(), m_gwInfos.end(),
                [id](typename GwInfoStorage::const_reference elem) -> bool
                {
                    return id == elem.m_id;
                });
    }

    bool updateTimestamp()
    {
        if (!m_timerActive) {
            return false;
        }

        checkTimeouts();
        // TODO: m_timestamp = ...getElapsed();
        m_timerActive = false;
        return true;
    }

    unsigned calcGwReleaseTimeout() const
    {
        if (m_gwInfos.empty()) {
            return std::numeric_limits<unsigned>::max();
        }

        auto iter = std::min_element(
            m_gwInfos.begin(), m_gwInfos.end(),
            [](typename GwInfoStorage::const_reference elem1,
               typename GwInfoStorage::const_reference elem2) -> bool
               {
                    return (elem1.m_timestamp + elem1.m_duration) <
                                    (elem2.m_timestamp + elem2.m_duration);
               });

        GASSERT(iter != m_gwInfos.end());
        auto finalTimestamp = iter->m_timestamp + iter->m_duration;
        if (finalTimestamp < m_timestamp) {
            GASSERT(!"Gateways are not cleaned up properly");
            return 0U;
        }

        return static_cast<unsigned>(finalTimestamp - m_timestamp);
    }

    void programNextTimeout()
    {
        static const unsigned DefaultNextTimeout = 5 * 60 * 1000;
        unsigned delay = DefaultNextTimeout;
        delay = std::min(delay, calcGwReleaseTimeout());

        // TODO: calculate next timeout and program
        m_nextTimeoutTimestamp = m_timestamp + delay;
        m_timerActive = true;
    }

    void checkAvailableGateways()
    {
        m_gwInfos.erase(
            std::remove_if(
                m_gwInfos.begin(), m_gwInfos.end(),
                [this](typename GwInfoStorage::const_reference elem) -> bool
                {
                    return (elem.m_duration != 0U) &&
                           (m_timestamp < (elem.m_timestamp + elem.m_duration));
                }),
            m_gwInfos.end());
    }

    void checkTimeouts()
    {
        if (m_timestamp < m_nextTimeoutTimestamp) {
            return;
        }

        checkAvailableGateways();
    }

    ProtStack m_stack;
    GwInfoStorage m_gwInfos;
    Timestamp m_timestamp = 0;
    Timestamp m_nextTimeoutTimestamp = 0;
    unsigned m_advertisePeriod = DefaultAdvertisePeriod;
    bool m_timerActive = false;

    static const unsigned DefaultAdvertisePeriod = 15 * 60 * 1000;
};

}  // namespace client

}  // namespace mqttsn


