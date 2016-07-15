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
#include "mqttsn/client/client.h"
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
    //typedef typename mqttsn::protocol::field::GwAdd<FieldBase, TProtOpts>::ValueType GwAddValueType;
    typedef typename mqttsn::protocol::field::Duration<FieldBase>::ValueType DurationValueType;
    typedef typename mqttsn::protocol::field::WillTopic<FieldBase, TProtOpts>::ValueType WillTopicType;
    typedef typename mqttsn::protocol::field::WillMsg<FieldBase, TProtOpts>::ValueType WillMsgType;
    typedef unsigned long long Timestamp;

    struct GwInfo
    {
        GwInfo() = default;

        Timestamp m_timestamp = 0;
        GwIdValueType m_id = 0;
        //GwAddValueType m_addr;
        DurationValueType m_duration = 0;
    };

    typedef details::GwInfoStorageTypeT<GwInfo, TClientOpts> GwInfoStorage;

    typedef mqttsn::protocol::message::Advertise<Message> AdvertiseMsg;
    typedef mqttsn::protocol::message::Searchgw<Message> SearchgwMsg;
    typedef mqttsn::protocol::message::Gwinfo<Message, TProtOpts> GwinfoMsg;
    typedef mqttsn::protocol::message::Connect<Message, TProtOpts> ConnectMsg;
    typedef mqttsn::protocol::message::Connack<Message> ConnackMsg;
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

    void setResponseTimeoutPeriod(unsigned val)
    {
        m_responseTimeoutPeriod = val;
    }

    unsigned getGwAdvertisePeriod() const
    {
        return m_advertisePeriod;
    }

    void setWillTopic(const char* topic)
    {
        m_willTopic = topic;

        if ((m_currOp != Op::None) ||
            (!m_connected)) {
            return;
        }

        // TODO: update will topic
    }

    void setWillMsg(const std::uint8_t* msg, std::size_t len)
    {
        m_willMsg.assign(msg, msg + len);

        if ((m_currOp != Op::None) ||
            (!m_connected)) {
            return;
        }

        // TODO: update will msg
    }

    void setNextTickProgramCallback(NextTickProgramFn cb, void* data)
    {
        m_nextTickProgramFn = cb;
        m_nextTickProgramData = data;
    }

    void setCancelNextTickWaitCallback(CancelNextTickWaitFn cb, void* data)
    {
        m_cancelNextTickWaitFn = cb;
        m_cancelNextTickWaitData = data;
    }

    void setNewGwReportCallback(NewGwReportFn cb, void* data)
    {
        m_newGwReportFn = cb;
        m_newGwReportData = data;
    }

    bool start()
    {
        if ((m_nextTickProgramFn == nullptr) ||
            (m_cancelNextTickWaitFn == nullptr)) {
            return false;
        }

        sendGwSearchReq();
        programNextTimeout();
        return true;
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

    MqttsnOperationStatus connect(
        const char* clientId,
        unsigned short keepAlivePeriod,
        bool cleanSession,
        ConnectStatusReportFn cb,
        void* data)
    {
        if (m_connected) {
            return MqttsnOperationStatus_InvalidOperation;
        }

        if (m_currOp != Op::None) {
            return MqttsnOperationStatus_Busy;
        }

        m_keepAlivePeriod = keepAlivePeriod * 1000;
        m_currOp = Op::Connect;

        auto* connectOp = new (&m_opStorage) ConnectOp;
        connectOp->m_lastMsgTimestamp = m_timestamp;
        connectOp->m_cb = cb;
        connectOp->m_cbData = data;

        ConnectMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<ConnectMsg::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
        auto& durationField = std::get<ConnectMsg::FieldIdx_duration>(fields);
        auto& clientIdField = std::get<ConnectMsg::FieldIdx_clientId>(fields);

        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_cleanSession, cleanSession);
        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_will, !m_willTopic.empty());

        durationField.value() = keepAlivePeriod;
        clientIdField.value() = clientId;

        sendMessage(msg);        return MqttsnOperationStatus_Success;
    }

    using Base::handle;
    virtual void handle(AdvertiseMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& idField = std::get<AdvertiseMsg::FieldIdx_gwId>(fields);
        auto& durationField = std::get<AdvertiseMsg::FieldIdx_duration>(fields);
        auto iter = findGwInfo(idField.value());
        if (iter != m_gwInfos.end()) {
            iter->m_timestamp = m_timestamp;
            iter->m_duration = durationField.value();
            return;
        }

        if (!addNewGw(idField.value())) {
            return;
        }

        GASSERT(!m_gwInfos.empty());
        auto& newElem = m_gwInfos.back();
        newElem.m_duration = durationField.value();

        reportNewGw(idField.value());
    }

    virtual void handle(GwinfoMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& idField = std::get<GwinfoMsg::FieldIdx_gwId>(fields);
        //auto& addrField = std::get<GwinfoMsg::FieldIdx_gwAdd>(fields);
        auto iter = findGwInfo(idField.value());
        if (iter != m_gwInfos.end()) {
//            if (!addrField.value().empty()) {
//                iter->m_addr = addrField.value();
//            }
            return;
        }

        if (!addNewGw(idField.value())) {
            return;
        }

        GASSERT(!m_gwInfos.empty());
        auto& newElem = m_gwInfos.back();
        newElem.m_duration = m_advertisePeriod;
//        if (!addrField.value().empty()) {
//            iter->m_addr = addrField.value();
//        }

        reportNewGw(idField.value());
    }

    virtual void handle(ConnackMsg& msg) override
    {
        if (m_currOp != Op::Connect) {
            return;
        }

        auto op = finaliseOp<ConnectOp>();
        if (op.m_cb == nullptr) {
            return;
        }

        static_cast<void>(msg);
        static const MqttsnConnectStatus StatusMap[] = {
            /* ReturnCodeVal_Accepted */ MqttsnReturnCode_Connected,
            /* ReturnCodeVal_Conjestion */ MqttsnReturnCode_Conjestion,
            /* ReturnCodeVal_InvalidTopicId */ MqttsnReturnCode_Denied,
            /* ReturnCodeVal_NotSupported */ MqttsnReturnCode_Denied
        };

        static const std::size_t StatusMapSize =
                                std::extent<decltype(StatusMap)>::value;
        static_assert(
            StatusMapSize == mqttsn::protocol::field::ReturnCodeVal_NumOfValues,
            "Incorrect map");

        MqttsnConnectStatus status = MqttsnReturnCode_Denied;
        auto& fields = msg.fields();
        auto& returnCodeField = std::get<ConnackMsg::FieldIdx_returnCode>(fields);
        auto returnCode = returnCodeField.value();
        if (returnCode < StatusMapSize) {
            status = StatusMap[returnCode];
        }

        op.m_cb(op.m_cbData, status);
    }

private:

    enum class Op
    {
        None,
        Connect,
        NumOfValues // must be last
    };

    struct OpBase
    {
        Timestamp m_lastMsgTimestamp = 0;
    };

    struct ConnectOp : public OpBase
    {
        ConnectStatusReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
    };

    typedef typename comms::util::AlignedUnion<
        ConnectOp
    >::Type OpStorageType;

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

        GASSERT(m_cancelNextTickWaitFn != nullptr);
        m_timestamp += m_cancelNextTickWaitFn(m_cancelNextTickWaitData);
        m_timerActive = false;
        checkTimeouts();
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

    unsigned calcSearchGwSendTimeout()
    {
        if (!m_gwInfos.empty()) {
            return std::numeric_limits<unsigned>::max();
        }

        auto nextSearchTimestamp = m_lastGwSearchTimestamp + RepeatSearchGwPeriod;
        if (nextSearchTimestamp <= m_timestamp) {
            return 1U;
        }

        return static_cast<unsigned>(nextSearchTimestamp - m_timestamp);
    }

    unsigned calcCurrentOpTimeout()
    {
        if (m_currOp == Op::None) {
            return std::numeric_limits<unsigned>::max();
        }

        auto* op = reinterpret_cast<OpBase*>(&m_opStorage);
        auto nextOpTimestamp = op->m_lastMsgTimestamp + m_responseTimeoutPeriod;
        if (nextOpTimestamp <= m_timestamp) {
            return 1U;
        }

        return static_cast<unsigned>(nextOpTimestamp - m_timestamp);
    }

    void programNextTimeout()
    {
        static const unsigned DefaultNextTimeout = 5 * 60 * 1000;
        unsigned delay = DefaultNextTimeout;
        delay = std::min(delay, calcGwReleaseTimeout());
        delay = std::min(delay, calcSearchGwSendTimeout());
        delay = std::min(delay, calcCurrentOpTimeout());

        // TODO: other delays
        GASSERT(m_nextTickProgramFn != nullptr);
        m_nextTickProgramFn(m_nextTickProgramData, delay);
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

    void checkGwSearchReq()
    {
        if (m_gwInfos.empty() &&
            ((m_lastGwSearchTimestamp + RepeatSearchGwPeriod) <= m_timestamp)) {
            sendGwSearchReq();
        }
    }

    void checkOpTimeout()
    {
        if (m_currOp == Op::None) {
            return;
        }

        auto* op = reinterpret_cast<OpBase*>(&m_opStorage);
        if (m_timestamp < (op->m_lastMsgTimestamp + m_responseTimeoutPeriod)) {
            return;
        }

        typedef void (Client<THandler, TClientOpts, TProtOpts>::*TimeoutFunc)();
        static const TimeoutFunc OpTimeoutFuncMap[] =
        {
            &Client::connectTimeout
        };
        static const std::size_t OpTimeoutFuncMapSize =
                            std::extent<decltype(OpTimeoutFuncMap)>::value;

        static_assert(OpTimeoutFuncMapSize == (static_cast<std::size_t>(Op::NumOfValues) - 1U),
            "Map above is incorrect");

        auto fn = OpTimeoutFuncMap[static_cast<unsigned>(m_currOp) - 1];
        (this->*(fn))();
    }

    void checkTimeouts()
    {
        if (m_timestamp < m_nextTimeoutTimestamp) {
            return;
        }

        checkAvailableGateways();
        checkGwSearchReq();
        checkOpTimeout();
    }

    bool addNewGw(GwIdValueType id)
    {
        if (m_gwInfos.max_size() <= m_gwInfos.size()) {
            return false;
        }

        m_gwInfos.emplace_back();
        auto& newElem = m_gwInfos.back();
        newElem.m_timestamp = m_timestamp;
        newElem.m_id = id;
        return true;
    }

    void reportNewGw(GwIdValueType id)
    {
        if (m_newGwReportFn != nullptr) {
            m_newGwReportFn(m_newGwReportData, id);
        }
    }

    void sendGwSearchReq()
    {
        SearchgwMsg msg;
        auto& fields = msg.fields();
        auto& radiusField = std::get<SearchgwMsg::FieldIdx_radus>(fields);
        radiusField.value() = m_broadcastRadius;

        sendMessage(msg);
        m_lastGwSearchTimestamp = m_timestamp;
    }

    void connectTimeout()
    {
        GASSERT(m_currOp == Op::Connect);

        auto op = finaliseOp<ConnectOp>();
        if (op.m_cb != nullptr) {
            op.m_cb(op.m_cbData, MqttsnReturnCode_Timeout);
        }
    }

    void sendMessage(const Message& msg)
    {
        static_cast<void>(msg);
        // TODO
    }

    template <typename TOp>
    TOp finaliseOp()
    {
        auto* op = reinterpret_cast<TOp*>(&m_opStorage);
        TOp opTmp = *op;
        op->~TOp();
        m_currOp = Op::None;
        return opTmp;
    }

    ProtStack m_stack;
    GwInfoStorage m_gwInfos;
    Timestamp m_timestamp = 0;
    Timestamp m_nextTimeoutTimestamp = 0;
    Timestamp m_lastGwSearchTimestamp = 0;
    unsigned m_advertisePeriod = DefaultAdvertisePeriod;
    unsigned m_responseTimeoutPeriod = DefaultResponseTimeoutPeriod;
    unsigned m_keepAlivePeriod = 0;
    std::uint8_t m_broadcastRadius = DefaultBroadcastRadius;

    bool m_timerActive = false;
    bool m_connected = false;

    Op m_currOp = Op::None;
    OpStorageType m_opStorage;

    WillTopicType m_willTopic;
    WillMsgType m_willMsg;

    NextTickProgramFn m_nextTickProgramFn = nullptr;
    void* m_nextTickProgramData = nullptr;

    CancelNextTickWaitFn m_cancelNextTickWaitFn = nullptr;
    void* m_cancelNextTickWaitData = nullptr;

    NewGwReportFn m_newGwReportFn = nullptr;
    void* m_newGwReportData = nullptr;

    ConnectStatusReportFn m_connectStatusReportFn = nullptr;
    void* m_connectStatusReportData = nullptr;

    static const unsigned DefaultAdvertisePeriod = 30 * 60 * 1000;
    static const unsigned DefaultResponseTimeoutPeriod = 15 * 1000;
    static const std::uint8_t DefaultBroadcastRadius = 1;
    static const unsigned RepeatSearchGwPeriod = 30 * 1000;
};

}  // namespace client

}  // namespace mqttsn


