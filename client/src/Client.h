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
#include "mqttsn/client/common.h"
#include "mqttsn/protocol/Stack.h"
#include "details/WriteBufStorageType.h"
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

//-----------------------------------------------------------

mqttsn::protocol::field::QosType translateQosValue(MqttsnQoS val)
{
    static_assert(
        (int)mqttsn::protocol::field::QosType::NoGwPublish == MqttsnQoS_NoGwPublish,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::protocol::field::QosType::AtMostOnceDelivery == MqttsnQoS_AtMostOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::protocol::field::QosType::AtLeastOnceDelivery == MqttsnQoS_AtLeastOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::protocol::field::QosType::ExactlyOnceDelivery == MqttsnQoS_ExactlyOnceDelivery,
        "Invalid mapping");

    return static_cast<mqttsn::protocol::field::QosType>(val);
}

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
    typedef mqttsn::protocol::message::Willtopicreq<Message> WilltopicreqMsg;
    typedef mqttsn::protocol::message::Willtopic<Message, TProtOpts> WilltopicMsg;
    typedef mqttsn::protocol::message::Willmsgreq<Message> WillmsgreqMsg;
    typedef mqttsn::protocol::message::Willmsg<Message, TProtOpts> WillmsgMsg;
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

    void setRetryPeriod(unsigned val)
    {
        m_retryPeriod = val;
    }

    void setBroadcastRadius(std::uint8_t val)
    {
        m_broadcastRadius = val;
    }

    unsigned getGwAdvertisePeriod() const
    {
        return m_advertisePeriod;
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

    void setSendOutputDataCallback(SendOutputDataFn cb, void* data)
    {
        m_sendOutputDataFn = cb;
        m_sendOutputDataData = data;
    }

    void setGwStatusReportCallback(GwStatusReportFn cb, void* data)
    {
        m_gwStatusReportFn = cb;
        m_gwStatusReportData = data;
    }

    bool start()
    {
        if ((m_nextTickProgramFn == nullptr) ||
            (m_cancelNextTickWaitFn == nullptr) ||
            (m_sendOutputDataFn == nullptr)) {
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

    MqttsnErrorCode connect(
        const char* clientId,
        unsigned short keepAlivePeriod,
        bool cleanSession,
        const MqttsnWillInfo* willInfo,
        ConnectStatusReportFn cb,
        void* data)
    {
        if (m_connected) {
            return MqttsnErrorCode_InvalidOperation;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        m_keepAlivePeriod = keepAlivePeriod * 1000;
        m_currOp = Op::Connect;

        auto* connectOp = new (&m_opStorage) ConnectOp;
        connectOp->m_lastMsgTimestamp = m_timestamp;
        if (willInfo != nullptr) {
            connectOp->m_willInfo = *willInfo;
        }
        connectOp->m_cb = cb;
        connectOp->m_cbData = data;

        ConnectMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<ConnectMsg::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
        auto& durationField = std::get<ConnectMsg::FieldIdx_duration>(fields);
        auto& clientIdField = std::get<ConnectMsg::FieldIdx_clientId>(fields);

        bool hasWill = (willInfo != nullptr) && (willInfo->topic != nullptr) && (willInfo->topic[0] != '\0');
        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_cleanSession, cleanSession);
        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_will, hasWill);

        durationField.value() = keepAlivePeriod;
        clientIdField.value() = clientId;

        sendMessage(msg);
        return MqttsnErrorCode_Success;
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

        if (!addNewGw(idField.value(), durationField.value() * 1000)) {
            return;
        }

        reportGwStatus(idField.value(), MqttsnGwStatus_Available);
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

        if (!addNewGw(idField.value(), m_advertisePeriod)) {
            return;
        }

        GASSERT(!m_gwInfos.empty());
//        if (!addrField.value().empty()) {
//            iter->m_addr = addrField.value();
//        }

        reportGwStatus(idField.value(), MqttsnGwStatus_Available);
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

    virtual void handle(WilltopicreqMsg& msg) override
    {
        static_cast<void>(msg);
        if (m_currOp != Op::Connect) {
            return;
        }

        auto* op = reinterpret_cast<ConnectOp*>(&m_opStorage);
        if ((op->m_willInfo.topic == nullptr) || (op->m_willInfo.topic[0] == '\0')) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        WilltopicMsg outMsg;
        auto& fields = outMsg.fields();
        auto& flagsField = std::get<WilltopicMsg::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
        auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
        auto& topicField = std::get<WilltopicMsg::FieldIdx_willTopic>(fields);

        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_retain, op->m_willInfo.retain);
        qosField.value() = details::translateQosValue(op->m_willInfo.qos);
        topicField.value() = op->m_willInfo.topic;
        sendMessage(outMsg);
    }

    virtual void handle(WillmsgreqMsg& msg) override
    {
        static_cast<void>(msg);
        if (m_currOp != Op::Connect) {
            return;
        }

        auto* op = reinterpret_cast<ConnectOp*>(&m_opStorage);
        if ((op->m_willInfo.topic == nullptr) || (op->m_willInfo.topic[0] == '\0')) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        WillmsgMsg outMsg;
        auto& fields = outMsg.fields();
        auto& willMsgField = std::get<WillmsgMsg::FieldIdx_willMsg>(fields);

        willMsgField.value().assign(op->m_willInfo.msg, op->m_willInfo.msg + op->m_willInfo.msgLen);
        sendMessage(outMsg);
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
        MqttsnWillInfo m_willInfo = MqttsnWillInfo();
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

        auto nextSearchTimestamp = m_lastGwSearchTimestamp + m_retryPeriod;
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
        auto nextOpTimestamp = op->m_lastMsgTimestamp + m_retryPeriod;
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
        auto checkMustRemoveFunc =
            [this](typename GwInfoStorage::const_reference elem) -> bool
            {
                GASSERT(elem.m_duration != 0U);
                return (m_timestamp <= (elem.m_timestamp + elem.m_duration));
            };

        bool mustRemove = false;
        auto iter = m_gwInfos.begin();
        while (iter != m_gwInfos.end()) {
            if (checkMustRemoveFunc(*iter)) {
                mustRemove = true;
                reportGwStatus(iter->m_id, MqttsnGwStatus_TimedOut);
            }
            ++iter;
        }

        if (!mustRemove) {
            return;
        }

        m_gwInfos.erase(
            std::remove_if(
                m_gwInfos.begin(), m_gwInfos.end(),
                checkMustRemoveFunc),
            m_gwInfos.end());
    }

    void checkGwSearchReq()
    {
        if (m_gwInfos.empty() &&
            ((m_lastGwSearchTimestamp + m_retryPeriod) <= m_timestamp)) {
            sendGwSearchReq();
        }
    }

    void checkOpTimeout()
    {
        if (m_currOp == Op::None) {
            return;
        }

        auto* op = reinterpret_cast<OpBase*>(&m_opStorage);
        if (m_timestamp < (op->m_lastMsgTimestamp + m_retryPeriod)) {
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

    bool addNewGw(GwIdValueType id, DurationValueType duration)
    {
        if (m_gwInfos.max_size() <= m_gwInfos.size()) {
            return false;
        }

        m_gwInfos.emplace_back();
        auto& newElem = m_gwInfos.back();
        newElem.m_timestamp = m_timestamp;
        newElem.m_id = id;
        newElem.m_duration = duration;
        return true;
    }

    void reportGwStatus(GwIdValueType id, MqttsnGwStatus status)
    {
        if (m_gwStatusReportFn != nullptr) {
            m_gwStatusReportFn(m_gwStatusReportData, id, status);
        }
    }

    void sendGwSearchReq()
    {
        SearchgwMsg msg;
        auto& fields = msg.fields();
        auto& radiusField = std::get<SearchgwMsg::FieldIdx_radus>(fields);
        radiusField.value() = m_broadcastRadius;

        sendMessage(msg, true);
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

    void sendMessage(const Message& msg, bool broadcast = false)
    {
        if (m_sendOutputDataFn == nullptr) {
            GASSERT(!"Unexpected send");
            return;
        }

        typedef details::WriteBufStorageTypeT<TProtOpts> DataStorage;

        DataStorage data(m_stack.length(msg));
        GASSERT(!data.empty());
        typename ProtStack::WriteIterator writeIter = &data[0];
        auto es = m_stack.write(msg, writeIter, data.size());
        GASSERT(es == comms::ErrorStatus::Success);
        if (es != comms::ErrorStatus::Success) {
            // Buffer is too small
            return;
        }

        auto writtenBytes = static_cast<std::size_t>(
            std::distance(typename ProtStack::WriteIterator(&data[0]), writeIter));

        m_sendOutputDataFn(m_sendOutputDataData, &data[0], writtenBytes, broadcast);
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
    unsigned m_retryPeriod = DefaultRetryPeriod;
    unsigned m_keepAlivePeriod = 0;
    std::uint8_t m_broadcastRadius = DefaultBroadcastRadius;

    bool m_timerActive = false;
    bool m_connected = false;

    Op m_currOp = Op::None;
    OpStorageType m_opStorage;

    NextTickProgramFn m_nextTickProgramFn = nullptr;
    void* m_nextTickProgramData = nullptr;

    CancelNextTickWaitFn m_cancelNextTickWaitFn = nullptr;
    void* m_cancelNextTickWaitData = nullptr;

    SendOutputDataFn m_sendOutputDataFn = nullptr;
    void* m_sendOutputDataData = nullptr;

    GwStatusReportFn m_gwStatusReportFn = nullptr;
    void* m_gwStatusReportData = nullptr;

    ConnectStatusReportFn m_connectStatusReportFn = nullptr;
    void* m_connectStatusReportData = nullptr;

    static const unsigned DefaultAdvertisePeriod = 30 * 60 * 1000;
    static const unsigned DefaultRetryPeriod = 15 * 1000;
    static const std::uint8_t DefaultBroadcastRadius = 0U;
};

}  // namespace client

}  // namespace mqttsn


