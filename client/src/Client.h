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
#include "comms/util/ScopeGuard.h"
#include "mqttsn/client/common.h"
#include "mqttsn/protocol/Stack.h"
#include "details/WriteBufStorageType.h"
#include "mqttsn/protocol/field.h"
#include "Message.h"
#include "AllMessages.h"

//#include <iostream>

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

template <typename TInfo, typename TOpts, bool THasRegisteredTopicsLimit>
struct RegInfoStorageType;

template <typename TInfo, typename TOpts>
struct RegInfoStorageType<TInfo, TOpts, true>
{
    typedef comms::util::StaticVector<TInfo, TOpts::RegisteredTopicsLimit> Type;
};

template <typename TInfo, typename TOpts>
struct RegInfoStorageType<TInfo, TOpts, false>
{
    typedef std::vector<TInfo> Type;
};

template <typename TInfo, typename TOpts>
using RegInfoStorageTypeT =
    typename GwInfoStorageType<TInfo, TOpts, TOpts::HasRegisteredTopicsLimit>::Type;

//-----------------------------------------------------------


mqttsn::protocol::field::QosType translateQosValue(MqttsnQoS val)
{
    static_assert(
        (int)mqttsn::protocol::field::QosType::AtMostOnceDelivery == MqttsnQoS_AtMostOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::protocol::field::QosType::AtLeastOnceDelivery == MqttsnQoS_AtLeastOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::protocol::field::QosType::ExactlyOnceDelivery == MqttsnQoS_ExactlyOnceDelivery,
        "Invalid mapping");

    if (val == MqttsnQoS_NoGwPublish) {
        return mqttsn::protocol::field::QosType::NoGwPublish;
    }

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
    typedef typename mqttsn::protocol::field::WillTopic<FieldBase, TProtOpts>::ValueType WillTopicType;
    typedef typename mqttsn::protocol::field::WillMsg<FieldBase, TProtOpts>::ValueType WillMsgType;
    typedef typename mqttsn::protocol::field::TopicName<FieldBase, TProtOpts>::ValueType TopicNameType;
    typedef typename mqttsn::protocol::field::TopicId<FieldBase>::ValueType TopicIdType;
    typedef unsigned long long Timestamp;

    struct GwInfo
    {
        GwInfo() = default;

        Timestamp m_timestamp = 0;
        GwIdValueType m_id = 0;
        //GwAddValueType m_addr;
        unsigned m_duration = 0;
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
    typedef mqttsn::protocol::message::Register<Message, TProtOpts> RegisterMsg;
    typedef mqttsn::protocol::message::Regack<Message> RegackMsg;
    typedef mqttsn::protocol::message::Publish<Message, TProtOpts> PublishMsg;
    typedef mqttsn::protocol::message::Puback<Message> PubackMsg;
    typedef mqttsn::protocol::message::Pubrec<Message> PubrecMsg;
    typedef mqttsn::protocol::message::Pubrel<Message> PubrelMsg;
    typedef mqttsn::protocol::message::Pubcomp<Message> PubcompMsg;

    typedef mqttsn::protocol::message::Pingreq<Message, TProtOpts> PingreqMsg;
    typedef mqttsn::protocol::message::Pingresp<Message> PingrespMsg;
    typedef mqttsn::protocol::message::Disconnect<Message> DisconnectMsg;

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

    void setRetryCount(unsigned val)
    {
        m_retryCount = std::max(1U, val);
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
        if (cb != nullptr) {
            m_nextTickProgramFn = cb;
            m_nextTickProgramData = data;
        }
    }

    void setCancelNextTickWaitCallback(CancelNextTickWaitFn cb, void* data)
    {
        if (cb != nullptr) {
            m_cancelNextTickWaitFn = cb;
            m_cancelNextTickWaitData = data;
        }
    }

    void setSendOutputDataCallback(SendOutputDataFn cb, void* data)
    {
        if (cb != nullptr) {
            m_sendOutputDataFn = cb;
            m_sendOutputDataData = data;
        }
    }

    void setGwStatusReportCallback(GwStatusReportFn cb, void* data)
    {
        m_gwStatusReportFn = cb;
        m_gwStatusReportData = data;
    }

    void setConnectionStatusReportCallback(ConnectionStatusReportFn cb, void* data)
    {
        if (cb != nullptr) {
            m_connectionStatusReportFn = cb;
            m_connectionStatusReportData = data;
        }
    }

    bool start()
    {
        if ((m_nextTickProgramFn == nullptr) ||
            (m_cancelNextTickWaitFn == nullptr) ||
            (m_sendOutputDataFn == nullptr) ||
            (m_connectionStatusReportFn == nullptr)) {
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
        bool timerWasActive = updateTimestamp();
        std::size_t consumed = 0;
        while (true) {
            auto iterTmp = iter;
            MsgPtr msg;
            auto es = m_stack.read(msg, iterTmp, len - consumed);
            if (es == comms::ErrorStatus::NotEnoughData) {
                break;
            }

            if (es != comms::ErrorStatus::Success) {
                ++iter;
                continue;
            }

            GASSERT(msg);
            m_lastMsgTimestamp = m_timestamp;
            msg->dispatch(*this);

            consumed += static_cast<std::size_t>(std::distance(iter, iterTmp));
            iter = iterTmp;
        }

        if (timerWasActive) {
            programNextTimeout();
        }

        return consumed;
    }

    bool cancel()
    {
        if (m_currOp == Op::None) {
            return false;
        }

        typedef void (Client<THandler, TClientOpts, TProtOpts>::*CancelFunc)();
        static const CancelFunc OpCancelFuncMap[] =
        {
            &Client::connectCancel,
            &Client::disconnectCancel,
            &Client::registerCancel,
            &Client::publishIdCancel,
            &Client::publishCancel
        };
        static const std::size_t OpCancelFuncMapSize =
                            std::extent<decltype(OpCancelFuncMap)>::value;

        static_assert(OpCancelFuncMapSize == (static_cast<std::size_t>(Op::NumOfValues) - 1U),
            "Map above is incorrect");

        auto opIdx = static_cast<unsigned>(m_currOp) - 1;
        GASSERT(opIdx < OpCancelFuncMapSize);
        auto fn = OpCancelFuncMap[opIdx];
        (this->*(fn))();
        return true;
    }

    MqttsnErrorCode connect(
        const char* clientId,
        unsigned short keepAlivePeriod,
        bool cleanSession,
        const MqttsnWillInfo* willInfo)
    {
        if (m_connected) {
            return MqttsnErrorCode_InvalidOperation;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        bool timerWasActive = updateTimestamp();

        m_keepAlivePeriod = keepAlivePeriod * 1000;
        m_currOp = Op::Connect;

        auto* connectOp = newOp<ConnectOp>();
        if (willInfo != nullptr) {
            connectOp->m_willInfo = *willInfo;
        }

        auto& fields = connectOp->m_connectMsg.fields();
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

        bool result = doConnect();
        static_cast<void>(result);
        GASSERT(result);

        if (timerWasActive) {
            programNextTimeout();
        }

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode disconnect()
    {
        if (!m_connected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        bool timerWasActive = updateTimestamp();

        m_currOp = Op::Disconnect;
        auto* disconnectOp = newOp<DisconnectOp>();
        static_cast<void>(disconnectOp);

        bool result = doDisconnect();
        static_cast<void>(result);
        GASSERT(result);

        if (timerWasActive) {
            programNextTimeout();
        }

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode registerTopic(
        const char* topic,
        TopicRegReportFn callback,
        void* data)
    {
        if (!m_connected) {
            return MqttsnErrorCode_NotConnected;
        }


        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if ((topic == nullptr) ||
            (callback == nullptr)) {
            return MqttsnErrorCode_BadParam;
        }

        bool timerWasActive = updateTimestamp();

        m_currOp = Op::Register;
        auto* regOp = newOp<RegisterOp>();
        regOp->m_topic = topic;
        regOp->m_cb = callback;
        regOp->m_cbData = data;

        bool result = doRegister();
        static_cast<void>(result);
        GASSERT(result);

        if (timerWasActive) {
            programNextTimeout();
        }

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode publish(
        MqttsnTopicId topicId,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain,
        PublishCompleteReportFn callback,
        void* data)
    {
        if ((!m_connected) && (qos != MqttsnQoS_NoGwPublish)) {
            return MqttsnErrorCode_NotConnected;
        }


        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if ((qos < MqttsnQoS_NoGwPublish) ||
            (MqttsnQoS_ExactlyOnceDelivery < qos)) {
            return MqttsnErrorCode_BadParam;
        }

        if ((callback == nullptr) && (MqttsnQoS_AtLeastOnceDelivery <= qos)) {
            return MqttsnErrorCode_BadParam;
        }

        bool timerWasActive = updateTimestamp();

        if (MqttsnQoS_AtLeastOnceDelivery <= qos) {
            m_currOp = Op::PublishId;
            auto* pubOp = newOp<PublishIdOp>();
            pubOp->m_topicId = topicId;
            pubOp->m_msg = msg;
            pubOp->m_msgLen = msgLen;
            pubOp->m_qos = qos;
            pubOp->m_retain = retain;
            pubOp->m_cb = callback;
            pubOp->m_cbData = data;

            bool result = doPublishId();
            static_cast<void>(result);
            GASSERT(result);
        }
        else {
            sendPublish(topicId, allocMsgId(), msg, msgLen, qos, retain, false);
            if (callback != nullptr) {
                callback(data, MqttsnAsyncOpStatus_Successful);
            }
        }

        if (timerWasActive) {
            programNextTimeout();
        }

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode publish(
        const char* topic,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain,
        PublishCompleteReportFn callback,
        void* data)
    {
        if ((!m_connected) && (qos != MqttsnQoS_NoGwPublish)) {
            return MqttsnErrorCode_NotConnected;
        }


        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if ((qos < MqttsnQoS_AtMostOnceDelivery) ||
            (MqttsnQoS_ExactlyOnceDelivery < qos) ||
            (callback == nullptr)) {
            return MqttsnErrorCode_BadParam;
        }

        bool timerWasActive = updateTimestamp();

        m_currOp = Op::Publish;
        auto* pubOp = newOp<PublishOp>();
        pubOp->m_topic = topic;
        pubOp->m_msg = msg;
        pubOp->m_msgLen = msgLen;
        pubOp->m_qos = qos;
        pubOp->m_retain = retain;
        pubOp->m_cb = callback;
        pubOp->m_cbData = data;

        bool result = doPublish();
        static_cast<void>(result);
        GASSERT(result);

        if (timerWasActive) {
            programNextTimeout();
        }

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

        if (!addNewGw(idField.value(), durationField.value() * 1000U)) {
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

        auto* op = opPtr<ConnectOp>();
        bool hasWill = (op->m_willInfo.topic != nullptr) && (op->m_willInfo.topic[0] != '\0');
        bool willReported = (op->m_willTopicSent && op->m_willMsgSent);
        if (hasWill && (!willReported)) {
            return;
        }

        finaliseOp<ConnectOp>();

        static_cast<void>(msg);
        static const MqttsnConnectionStatus StatusMap[] = {
            /* ReturnCodeVal_Accepted */ MqttsnConnectionStatus_Connected,
            /* ReturnCodeVal_Conjestion */ MqttsnConnectionStatus_Conjestion,
            /* ReturnCodeVal_InvalidTopicId */ MqttsnConnectionStatus_Denied,
            /* ReturnCodeVal_NotSupported */ MqttsnConnectionStatus_Denied
        };

        static const std::size_t StatusMapSize =
                                std::extent<decltype(StatusMap)>::value;
        static_assert(
            StatusMapSize == mqttsn::protocol::field::ReturnCodeVal_NumOfValues,
            "Incorrect map");

        MqttsnConnectionStatus status = MqttsnConnectionStatus_Denied;
        auto& fields = msg.fields();
        auto& returnCodeField = std::get<ConnackMsg::FieldIdx_returnCode>(fields);
        auto returnCode = returnCodeField.value();
        if (returnCode < StatusMapSize) {
            status = StatusMap[returnCode];
        }

        GASSERT(!m_connected);
        if (status == MqttsnConnectionStatus_Connected) {
            m_connected = true;
        }

        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, status);
    }

    virtual void handle(WilltopicreqMsg& msg) override
    {
        static_cast<void>(msg);
        if (m_currOp != Op::Connect) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
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
        op->m_willTopicSent = true;
        sendMessage(outMsg);
    }

    virtual void handle(WillmsgreqMsg& msg) override
    {
        static_cast<void>(msg);
        if (m_currOp != Op::Connect) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
        if ((op->m_willInfo.topic == nullptr) || (op->m_willInfo.topic[0] == '\0')) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        WillmsgMsg outMsg;
        auto& fields = outMsg.fields();
        auto& willMsgField = std::get<WillmsgMsg::FieldIdx_willMsg>(fields);

        willMsgField.value().assign(op->m_willInfo.msg, op->m_willInfo.msg + op->m_willInfo.msgLen);
        op->m_willMsgSent = true;
        sendMessage(outMsg);
    }

    virtual void handle(RegackMsg& msg) override
    {
        if (m_currOp != Op::Publish) {
            return;
        }

        auto* op = opPtr<PublishOp>();

        if (!op->m_didRegistration) {
            return;
        }

        auto& fields = msg.fields();
        auto& msgIdField = std::get<RegackMsg::FieldIdx_msgId>(fields);

        if (msgIdField.value() != op->m_msgId) {
            return;
        }

        auto& topicIdField = std::get<RegackMsg::FieldIdx_topicId>(fields);
        auto& retCodeField = std::get<RegackMsg::FieldIdx_returnCode>(fields);

        if (retCodeField.value() != mqttsn::protocol::field::ReturnCodeVal_Accepted) {
            static const MqttsnAsyncOpStatus Map[] = {
                /* ReturnCodeVal_Accepted */ MqttsnAsyncOpStatus_Invalid,
                /* ReturnCodeVal_Conjestion */ MqttsnAsyncOpStatus_Conjestion,
                /* ReturnCodeVal_InvalidTopicId */ MqttsnAsyncOpStatus_InvalidId,
                /* ReturnCodeVal_NotSupported */ MqttsnAsyncOpStatus_NotSupported
            };

            static const std::size_t MapSize = std::extent<decltype(Map)>::value;

            static_assert(MapSize == (unsigned)mqttsn::protocol::field::ReturnCodeVal_NumOfValues,
                "Map is incorrect");

            MqttsnAsyncOpStatus status = MqttsnAsyncOpStatus_NotSupported;
            if (static_cast<unsigned>(retCodeField.value()) < MapSize) {
                status = Map[static_cast<unsigned>(retCodeField.value())];
            }

            finalisePublishOp(status);
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        op->m_registered = true;
        op->m_topicId = topicIdField.value();
        op->m_attempt = 0;

        auto startPublishOnReturn =
            comms::util::makeScopeGuard(
                [this]()
                {
                    bool result = doPublish();
                    static_cast<void>(result);
                    assert(result);
                });

        auto iter = std::find_if(
            m_regInfos.begin(), m_regInfos.end(),
            [op](typename RegInfosList::const_reference elem) -> bool
            {
                return elem.m_allocated && (elem.m_topic == op->m_topic);
            });

        if (iter != m_regInfos.end()) {
            iter->m_timestamp = m_timestamp;
            iter->m_topicId = topicIdField.value();
            return;
        }

        iter = std::find_if(
            m_regInfos.begin(), m_regInfos.end(),
            [](typename RegInfosList::const_reference elem) -> bool
            {
                return !elem.m_allocated;
            });

        if (iter != m_regInfos.end()) {
            iter->m_timestamp = m_timestamp;
            iter->m_topic = op->m_topic;
            iter->m_topicId = topicIdField.value();
            iter->m_allocated = true;
            return;
        }

        RegInfo info;
        info.m_timestamp = m_timestamp;
        info.m_topic = op->m_topic;
        info.m_topicId = topicIdField.value();
        info.m_allocated = true;

        if (m_regInfos.size() < m_regInfos.max_size()) {
            m_regInfos.push_back(std::move(info));
            return;
        }

        iter = std::min_element(
            m_regInfos.begin(), m_regInfos.end(),
            [](typename RegInfosList::const_reference elem1,
               typename RegInfosList::const_reference elem2) -> bool
            {
                return elem1.m_timestamp < elem2.m_timestamp;
            });

        GASSERT(iter != m_regInfos.end());
        *iter = info;
    }

    virtual void handle(PubackMsg& msg) override
    {
        if ((m_currOp != Op::Publish) && (m_currOp != Op::PublishId)) {
            return;
        }

        auto* op = opPtr<PublishOpBase>();

        auto& fields = msg.fields();
        auto& topicIdField = std::get<PubackMsg::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<PubackMsg::FieldIdx_msgId>(fields);
        auto& retCodeField = std::get<PubackMsg::FieldIdx_returnCode>(fields);

        if ((topicIdField.value() != op->m_topicId) ||
            (msgIdField.value() != op->m_msgId)) {
            return;
        }

        if ((op->m_qos == MqttsnQoS_ExactlyOnceDelivery) &&
            (retCodeField.value() == mqttsn::protocol::field::ReturnCodeVal_Accepted)) {
            // PUBREC is expected instead
            return;
        }

        do {

            if ((op->m_qos < MqttsnQoS_AtLeastOnceDelivery) ||
                (retCodeField.value() != mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId) ||
                (m_currOp != Op::Publish) ||
                (opPtr<PublishOp>()->m_didRegistration)) {
                break;
            }

            auto iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [op](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == op->m_topicId);
                });

            if (iter == m_regInfos.end()) {
                return;
            }

            iter->m_allocated = false;

            op->m_lastMsgTimestamp = m_timestamp;
            op->m_topicId = 0U;
            op->m_attempt = 0;
            opPtr<PublishOp>()->m_registered = false;
            doPublish();
            return;
        } while (false);

        static const MqttsnAsyncOpStatus Map[] = {
            /* ReturnCodeVal_Accepted */ MqttsnAsyncOpStatus_Successful,
            /* ReturnCodeVal_Conjestion */ MqttsnAsyncOpStatus_Conjestion,
            /* ReturnCodeVal_InvalidTopicId */ MqttsnAsyncOpStatus_InvalidId,
            /* ReturnCodeVal_NotSupported */ MqttsnAsyncOpStatus_NotSupported
        };

        static const std::size_t MapSize = std::extent<decltype(Map)>::value;

        static_assert(MapSize == (unsigned)mqttsn::protocol::field::ReturnCodeVal_NumOfValues,
            "Map is incorrect");

        MqttsnAsyncOpStatus status = MqttsnAsyncOpStatus_NotSupported;
        if (static_cast<unsigned>(retCodeField.value()) < MapSize) {
            status = Map[static_cast<unsigned>(retCodeField.value())];
        }

        finalisePublishOp(status);
    }

    virtual void handle(PubrecMsg& msg) override
    {
        if ((m_currOp != Op::Publish) && (m_currOp != Op::PublishId)) {
            return;
        }

        auto* op = opPtr<PublishOpBase>();

        auto& fields = msg.fields();
        auto& msgIdField = std::get<PubrecMsg::FieldIdx_msgId>(fields);

        if (msgIdField.value() != op->m_msgId) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        op->m_ackReceived = true;
        sendPubrel(op->m_msgId);
    }

    virtual void handle(PubcompMsg& msg) override
    {
        if ((m_currOp != Op::Publish) && (m_currOp != Op::PublishId)) {
            return;
        }

        auto* op = opPtr<PublishOpBase>();

        auto& fields = msg.fields();
        auto& msgIdField = std::get<PubrecMsg::FieldIdx_msgId>(fields);

        if ((msgIdField.value() != op->m_msgId) ||
            (!op->m_ackReceived)) {
            return;
        }

        finalisePublishOp(MqttsnAsyncOpStatus_Successful);
    }

    virtual void handle(PingreqMsg& msg) override
    {
        static_cast<void>(msg);
        PingrespMsg outMsg;
        sendMessage(outMsg);
    }

    virtual void handle(PingrespMsg& msg) override
    {
        static_cast<void>(msg);
        m_pingCount = 0U;
    }

    virtual void handle(DisconnectMsg& msg) override
    {
        static_cast<void>(msg);

        if (m_currOp == Op::Disconnect) {
            finaliseOp<DisconnectOp>();
            reportDisconnected();
            return;
        }

        if (m_currOp == Op::None) {
            reportDisconnected();
            return;
        }

        // TODO: support asleep confirmation

        cancel();
        reportDisconnected();
    }

private:

    enum class Op
    {
        None,
        Connect,
        Disconnect,
        Register,
        PublishId,
        Publish,
        NumOfValues // must be last
    };

    struct OpBase
    {
        Timestamp m_lastMsgTimestamp = 0;
        unsigned m_attempt = 0;
    };

    struct ConnectOp : public OpBase
    {
        MqttsnWillInfo m_willInfo = MqttsnWillInfo();
        ConnectMsg m_connectMsg;
        bool m_willTopicSent = false;
        bool m_willMsgSent = false;
    };

    typedef OpBase DisconnectOp;

    struct RegisterOp : public OpBase
    {
        const char* m_topic = nullptr;
        TopicRegReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
        std::uint8_t m_msgId = 0;
    };

    struct PublishOpBase : public OpBase
    {
        MqttsnTopicId m_topicId = 0U;
        std::uint16_t m_msgId = 0U;
        const std::uint8_t* m_msg = nullptr;
        std::size_t m_msgLen = 0;
        MqttsnQoS m_qos = MqttsnQoS_AtMostOnceDelivery;
        bool m_retain = false;
        PublishCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
        bool m_ackReceived = false;
    };

    struct PublishIdOp : public PublishOpBase
    {
    };

    struct PublishOp : public PublishOpBase
    {
        const char* m_topic = nullptr;
        bool m_registered = false;
        bool m_didRegistration = false;
    };

    typedef typename comms::util::AlignedUnion<
        ConnectOp,
        DisconnectOp,
        RegisterOp,
        PublishIdOp,
        PublishOp
    >::Type OpStorageType;

    typedef protocol::Stack<Message, AllMessages<TProtOpts>, comms::option::InPlaceAllocation> ProtStack;
    typedef typename ProtStack::MsgPtr MsgPtr;

    struct RegInfo
    {
        Timestamp m_timestamp = 0U;
        TopicNameType m_topic;
        TopicIdType m_topicId = 0U;
        bool m_locked = false;
        bool m_allocated = false;
    };

    typedef details::RegInfoStorageTypeT<RegInfo, TClientOpts> RegInfosList;

    template <typename TOp>
    TOp* opPtr()
    {
        return reinterpret_cast<TOp*>(&m_opStorage);
    }

    template <typename TOp>
    TOp* newOp()
    {
        static_assert(sizeof(TOp) <= sizeof(m_opStorage), "Invalid storage size");
        auto op = new (&m_opStorage) TOp();
        op->m_lastMsgTimestamp = m_timestamp;
        return op;
    }

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
            return NoTimeout;
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
            return NoTimeout;
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
            return NoTimeout;
        }

        auto* op = opPtr<OpBase>();
        auto nextOpTimestamp = op->m_lastMsgTimestamp + m_retryPeriod;
        if (nextOpTimestamp <= m_timestamp) {
            return 1U;
        }

        return static_cast<unsigned>(nextOpTimestamp - m_timestamp);
    }

    unsigned calcPingTimeout()
    {
        if (!m_connected) {
            return NoTimeout;
        }

        auto pingTimestamp = m_lastMsgTimestamp + m_keepAlivePeriod;
        if (0 < m_pingCount) {
            pingTimestamp = m_lastPingTimestamp + m_retryPeriod;
        }

        if (pingTimestamp <= m_timestamp) {
            return 1U;
        }

        return static_cast<unsigned>(pingTimestamp - m_timestamp);
    }

    void programNextTimeout()
    {
        unsigned delay = NoTimeout;
        delay = std::min(delay, calcGwReleaseTimeout());
        delay = std::min(delay, calcSearchGwSendTimeout());
        delay = std::min(delay, calcCurrentOpTimeout());
        delay = std::min(delay, calcPingTimeout());

        if (delay == NoTimeout) {
            return;
        }

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
                return ((elem.m_timestamp + elem.m_duration) <= m_timestamp);
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

    void checkPing()
    {
        if (!m_connected) {
            return;
        }

        if (m_pingCount == 0) {
            // first ping;
            if (m_timestamp < (m_lastMsgTimestamp + m_keepAlivePeriod)) {
                return;
            }

            sendPing();
            return;
        }

        GASSERT(0U < m_lastPingTimestamp);
        if (m_timestamp < (m_lastPingTimestamp + m_retryPeriod)) {
            return;
        }

        if (m_retryCount <= m_pingCount) {
            reportDisconnected();
            return;
        }

        sendPing();
    }

    void checkOpTimeout()
    {
        if (m_currOp == Op::None) {
            return;
        }

        auto* op = opPtr<OpBase>();
        if (m_timestamp < (op->m_lastMsgTimestamp + m_retryPeriod)) {
            return;
        }

        typedef void (Client<THandler, TClientOpts, TProtOpts>::*TimeoutFunc)();
        static const TimeoutFunc OpTimeoutFuncMap[] =
        {
            &Client::connectTimeout,
            &Client::disconnectTimeout,
            &Client::registerTimeout,
            &Client::publishIdTimeout,
            &Client::publishTimeout,
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
        checkPing();
        checkOpTimeout();
    }

    bool addNewGw(GwIdValueType id, unsigned duration)
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

        if (doConnect()) {
            opPtr<ConnectOp>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseOp<ConnectOp>();
        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_Timeout);
    }

    void connectCancel()
    {
        GASSERT(m_currOp == Op::Connect);
        finaliseOp<ConnectOp>();
        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_ConnectAborted);
    }

    void disconnectTimeout()
    {
        GASSERT(m_currOp == Op::Disconnect);

        if (doDisconnect()) {
            opPtr<DisconnectOp>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseOp<DisconnectOp>();
        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_Disconnected);
    }

    void disconnectCancel()
    {
        GASSERT(m_currOp == Op::Disconnect);
        finaliseOp<DisconnectOp>();
        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_Disconnected);
    }

    void registerTimeout()
    {
        GASSERT(m_currOp == Op::Register);

        if (doRegister()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseRegisterOp(MqttsnTopicRegStatus_NoResponse);
    }

    void registerCancel()
    {
        GASSERT(m_currOp == Op::Register);
        finaliseRegisterOp(MqttsnTopicRegStatus_Aborted);
    }

    void publishIdTimeout()
    {
        GASSERT(m_currOp == Op::PublishId);

        if (doPublishId()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finalisePublishOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void publishIdCancel()
    {
        GASSERT(m_currOp == Op::PublishId);
        finalisePublishOp(MqttsnAsyncOpStatus_Aborted);
    }

    void publishTimeout()
    {
        GASSERT(m_currOp == Op::Publish);

        if (doPublish()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finalisePublishOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void publishCancel()
    {
        GASSERT(m_currOp == Op::Publish);
        finalisePublishOp(MqttsnAsyncOpStatus_Aborted);
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
    void finaliseOp()
    {
        auto* op = opPtr<TOp>();
        op->~TOp();
        m_currOp = Op::None;
    }

    bool doConnect()
    {
        GASSERT (m_currOp == Op::Connect);

        auto* op = opPtr<ConnectOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;
        sendMessage(op->m_connectMsg);
        return true;
    }

    bool doDisconnect()
    {
        GASSERT (m_currOp == Op::Disconnect);

        auto* op = opPtr<DisconnectOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        DisconnectMsg msg;
        sendMessage(msg);
        return true;
    }

    bool doRegister()
    {
        GASSERT (m_currOp == Op::Register);

        auto* op = opPtr<RegisterOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;
        op->m_msgId = allocMsgId();

        RegisterMsg msg;
        auto& fields = msg.fields();
        auto& msgIdField = std::get<RegisterMsg::FieldIdx_msgId>(fields);
        auto& topicNameField = std::get<RegisterMsg::FieldIdx_topicName>(fields);

        msgIdField.value() = op->m_msgId;
        topicNameField.value() = op->m_topic;

        sendMessage(msg);
        return true;
    }

    bool doPublishId()
    {
        GASSERT (m_currOp == Op::PublishId);

        auto* op = opPtr<PublishIdOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;
        op->m_msgId = allocMsgId();
        sendPublish(
            op->m_topicId,
            op->m_msgId,
            op->m_msg,
            op->m_msgLen,
            op->m_qos,
            op->m_retain,
            1U < op->m_attempt);
        return true;
    }

    bool doPublish()
    {
        GASSERT (m_currOp == Op::Publish);

        auto* op = opPtr<PublishOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;
        op->m_msgId = allocMsgId();

        do {
            if (op->m_registered) {
                break;
            }

            auto iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [op](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topic == op->m_topic);
                });

            if (iter != m_regInfos.end()) {
                op->m_registered = true;
                op->m_topicId = iter->m_topicId;
                iter->m_timestamp = m_timestamp;
                break;
            }

            op->m_didRegistration = true;
            sendRegister(op->m_msgId, op->m_topic);
            return true;
        } while (false);


        assert(op->m_registered);
        sendPublish(
            op->m_topicId,
            op->m_msgId,
            op->m_msg,
            op->m_msgLen,
            op->m_qos,
            op->m_retain,
            1U < op->m_attempt);

        if (op->m_qos <= MqttsnQoS_AtMostOnceDelivery) {
            finalisePublishOp(MqttsnAsyncOpStatus_Successful);
        }

        return true;
    }

    void sendPing()
    {
        ++m_pingCount;
        m_lastPingTimestamp = m_timestamp;
        PingreqMsg msg;
        sendMessage(msg);
    }

    void sendPublish(
        MqttsnTopicId topicId,
        std::uint16_t msgId,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain,
        bool duplicate)
    {
        PublishMsg pubMsg;
        auto& fields = pubMsg.fields();
        auto& flagsField = std::get<PublishMsg::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
        auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
        auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
        auto& topicIdField = std::get<PublishMsg::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<PublishMsg::FieldIdx_msgId>(fields);
        auto& dataField = std::get<PublishMsg::FieldIdx_data>(fields);

        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_retain, retain);
        qosField.value() = details::translateQosValue(qos);
        dupFlagsField.setBitValue(mqttsn::protocol::field::DupFlagsBits_dup, duplicate);
        topicIdField.value() = topicId;
        msgIdField.value() = msgId;
        dataField.value().assign(msg, msg + msgLen);

        sendMessage(pubMsg);
    }

    void sendRegister(
        std::uint16_t msgId,
        const char* topic)
    {
        RegisterMsg msg;
        auto& fields = msg.fields();
        auto& msgIdField = std::get<RegisterMsg::FieldIdx_msgId>(fields);
        auto& topicNameField = std::get<RegisterMsg::FieldIdx_topicName>(fields);

        msgIdField.value() = msgId;
        topicNameField.value() = topic;
        sendMessage(msg);
    }

    void sendPubrel(std::uint16_t msgId)
    {
        PubrelMsg msg;
        auto& fields = msg.fields();
        auto& msgIdField = std::get<PubrelMsg::FieldIdx_msgId>(fields);
        msgIdField.value() = msgId;
        sendMessage(msg);
    }

    void reportDisconnected()
    {
        m_connected = false;
        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_Disconnected);
    }

    std::uint16_t allocMsgId()
    {
        ++m_msgId;
        return m_msgId;
    }

    void finaliseRegisterOp(MqttsnTopicRegStatus status, std::uint16_t topicId = 0U)
    {
        GASSERT(m_currOp == Op::Register);
        auto* op = opPtr<RegisterOp>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        finaliseOp<RegisterOp>();
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status, topicId);
    }

    void finalisePublishOp(MqttsnAsyncOpStatus status)
    {
        GASSERT((m_currOp == Op::Publish) || (m_currOp == Op::PublishId));
        auto* op = opPtr<PublishOpBase>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        if (m_currOp == Op::Publish) {
            finaliseOp<PublishOp>();
        }
        else {
            finaliseOp<PublishIdOp>();
        }
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status);
    }


    ProtStack m_stack;
    GwInfoStorage m_gwInfos;
    Timestamp m_timestamp = 0;
    Timestamp m_nextTimeoutTimestamp = 0;
    Timestamp m_lastGwSearchTimestamp = 0;
    Timestamp m_lastMsgTimestamp = 0;
    Timestamp m_lastPingTimestamp = 0;
    unsigned m_pingCount = 0;
    unsigned m_advertisePeriod = DefaultAdvertisePeriod;
    unsigned m_retryPeriod = DefaultRetryPeriod;
    unsigned m_retryCount = DefaultRetryCount;
    unsigned m_keepAlivePeriod = 0;
    std::uint16_t m_msgId = 0;
    std::uint8_t m_broadcastRadius = DefaultBroadcastRadius;

    bool m_timerActive = false;
    bool m_connected = false;

    Op m_currOp = Op::None;
    OpStorageType m_opStorage;

    RegInfosList m_regInfos;

    NextTickProgramFn m_nextTickProgramFn = nullptr;
    void* m_nextTickProgramData = nullptr;

    CancelNextTickWaitFn m_cancelNextTickWaitFn = nullptr;
    void* m_cancelNextTickWaitData = nullptr;

    SendOutputDataFn m_sendOutputDataFn = nullptr;
    void* m_sendOutputDataData = nullptr;

    GwStatusReportFn m_gwStatusReportFn = nullptr;
    void* m_gwStatusReportData = nullptr;

    ConnectionStatusReportFn m_connectionStatusReportFn = nullptr;
    void* m_connectionStatusReportData = nullptr;

    static const unsigned DefaultAdvertisePeriod = 30 * 60 * 1000;
    static const unsigned DefaultRetryPeriod = 15 * 1000;
    static const unsigned DefaultRetryCount = 3;
    static const std::uint8_t DefaultBroadcastRadius = 0U;

    static const unsigned NoTimeout = std::numeric_limits<unsigned>::max();
};

}  // namespace client

}  // namespace mqttsn


