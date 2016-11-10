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

MqttsnQoS translateQosValue(mqttsn::protocol::field::QosType val)
{

    if (val == mqttsn::protocol::field::QosType::NoGwPublish) {
        return MqttsnQoS_NoGwPublish;
    }

    return static_cast<MqttsnQoS>(val);
}

}  // namespace details

template <
    typename THandler,
    typename TClientOpts,
    typename TProtOpts>
class Client : public THandler
{
    typedef THandler Base;

    typedef unsigned long long Timestamp;

    enum class Op
    {
        None,
        Connect,
        Disconnect,
        PublishId,
        Publish,
        SubscribeId,
        Subscribe,
        UnsubscribeId,
        Unsubscribe,
        WillUpdate,
        WillTopicUpdate,
        WillMsgUpdate,
        Sleep,
        CheckMessages,
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
        const char* m_clientId = nullptr;
        std::uint16_t m_keepAlivePeriod = 0;
        bool m_hasWill = false;
        bool m_cleanSession = false;
        bool m_willTopicSent = false;
        bool m_willMsgSent = false;
    };

    typedef OpBase DisconnectOp;

    struct PublishOpBase : public OpBase
    {
        MqttsnTopicId m_topicId = 0U;
        std::uint16_t m_msgId = 0U;
        const std::uint8_t* m_msg = nullptr;
        std::size_t m_msgLen = 0;
        MqttsnQoS m_qos = MqttsnQoS_AtMostOnceDelivery;
        bool m_retain = false;
        MqttsnPublishCompleteReportFn m_cb = nullptr;
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

    struct SubscribeOpBase : public OpBase
    {
        MqttsnQoS m_qos = MqttsnQoS_AtMostOnceDelivery;
        MqttsnSubscribeCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
        std::uint16_t m_msgId = 0U;
    };

    struct SubscribeIdOp : public SubscribeOpBase
    {
        MqttsnTopicId m_topicId = 0U;
    };

    struct SubscribeOp : public SubscribeOpBase
    {
        const char* m_topic = nullptr;
        MqttsnTopicId m_topicId = 0U;
    };

    struct UnsubscribeOpBase : public OpBase
    {
        MqttsnUnsubscribeCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
        std::uint16_t m_msgId = 0U;
    };

    struct UnsubscribeIdOp : public UnsubscribeOpBase
    {
        MqttsnTopicId m_topicId = 0U;
    };

    struct UnsubscribeOp : public UnsubscribeOpBase
    {
        const char* m_topic = nullptr;
        MqttsnTopicId m_topicId = 0U;
    };

    struct WillUpdateOp : public ConnectOp
    {
        MqttsnWillUpdateCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
    };

    struct WillTopicUpdateOp : public OpBase
    {
        const char* m_topic = nullptr;
        MqttsnQoS m_qos;
        bool m_retain;
        MqttsnWillTopicUpdateCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
    };

    struct WillMsgUpdateOp : public OpBase
    {
        const unsigned char* m_msg = nullptr;
        unsigned m_msgLen = 0;
        MqttsnWillMsgUpdateCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
    };

    struct SleepOp : public OpBase
    {
        MqttsnSleepCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
        std::uint16_t m_duration = 0;
    };

    struct CheckMessagesOp : public OpBase
    {
        MqttsnCheckMessagesCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
    };

    enum class ConnectionStatus
    {
        Disconnected,
        Connected,
        Asleep
    };

public:
    typedef Message::Field FieldBase;
    typedef typename mqttsn::protocol::field::GwId<FieldBase>::ValueType GwIdValueType;
    //typedef typename mqttsn::protocol::field::GwAdd<FieldBase, TProtOpts>::ValueType GwAddValueType;
    typedef typename mqttsn::protocol::field::WillTopic<FieldBase, TProtOpts>::ValueType WillTopicType;
    typedef typename mqttsn::protocol::field::WillMsg<FieldBase, TProtOpts>::ValueType WillMsgType;
    typedef typename mqttsn::protocol::field::TopicName<FieldBase, TProtOpts>::ValueType TopicNameType;
    typedef typename mqttsn::protocol::field::Data<FieldBase, TProtOpts>::ValueType DataType;
    typedef typename mqttsn::protocol::field::TopicId<FieldBase>::ValueType TopicIdType;
    typedef typename mqttsn::protocol::field::ClientId<FieldBase>::ValueType ClientIdType;

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
    typedef mqttsn::protocol::message::Subscribe<Message, TProtOpts> SubscribeMsg;
    typedef mqttsn::protocol::message::Suback<Message> SubackMsg;
    typedef mqttsn::protocol::message::Unsubscribe<Message, TProtOpts> UnsubscribeMsg;
    typedef mqttsn::protocol::message::Unsuback<Message> UnsubackMsg;
    typedef mqttsn::protocol::message::Pingreq<Message, TProtOpts> PingreqMsg;
    typedef mqttsn::protocol::message::Pingresp<Message> PingrespMsg;
    typedef mqttsn::protocol::message::Disconnect<Message> DisconnectMsg;
    typedef mqttsn::protocol::message::Willtopicupd<Message, TProtOpts> WilltopicupdMsg;
    typedef mqttsn::protocol::message::Willtopicresp<Message> WilltopicrespMsg;
    typedef mqttsn::protocol::message::Willmsgupd<Message, TProtOpts> WillmsgupdMsg;
    typedef mqttsn::protocol::message::Willmsgresp<Message> WillmsgrespMsg;

    Client() = default;
    virtual ~Client() = default;

    typedef Message::ReadIterator ReadIterator;

    const GwInfoStorage& gwInfos() const
    {
        return m_gwInfos;
    }

    void setGwAdvertisePeriod(unsigned val)
    {
        static const auto MaxVal =
            std::numeric_limits<decltype(m_advertisePeriod)>::max() / 1000;
        m_advertisePeriod = std::min(val * 1000, MaxVal);
    }

    void setRetryPeriod(unsigned val)
    {
        static const auto MaxVal =
            std::numeric_limits<decltype(m_retryPeriod)>::max() / 1000;
        m_retryPeriod = std::min(val * 1000, MaxVal);
    }

    void setRetryCount(unsigned val)
    {
        m_retryCount = std::max(1U, val);
    }

    void setBroadcastRadius(std::uint8_t val)
    {
        m_broadcastRadius = val;
    }

    void setNextTickProgramCallback(MqttsnNextTickProgramFn cb, void* data)
    {
        if (cb != nullptr) {
            m_nextTickProgramFn = cb;
            m_nextTickProgramData = data;
        }
    }

    void setCancelNextTickWaitCallback(MqttsnCancelNextTickWaitFn cb, void* data)
    {
        if (cb != nullptr) {
            m_cancelNextTickWaitFn = cb;
            m_cancelNextTickWaitData = data;
        }
    }

    void setSendOutputDataCallback(MqttsnSendOutputDataFn cb, void* data)
    {
        if (cb != nullptr) {
            m_sendOutputDataFn = cb;
            m_sendOutputDataData = data;
        }
    }

    void setGwStatusReportCallback(MqttsnGwStatusReportFn cb, void* data)
    {
        m_gwStatusReportFn = cb;
        m_gwStatusReportData = data;
    }

    void setConnectionStatusReportCallback(MqttsnConnectionStatusReportFn cb, void* data)
    {
        if (cb != nullptr) {
            m_connectionStatusReportFn = cb;
            m_connectionStatusReportData = data;
        }
    }

    void setMessageReportCallback(MqttsnMessageReportFn cb, void* data)
    {
        m_msgReportFn = cb;
        m_msgReportData = data;
    }

    void setSearchgwEnabled(bool value)
    {
        m_searchgwEnabled = value;
    }

    MqttsnErrorCode start()
    {
        if (m_running) {
            return MqttsnErrorCode_AlreadyStarted;
        }

        if ((m_nextTickProgramFn == nullptr) ||
            (m_cancelNextTickWaitFn == nullptr) ||
            (m_sendOutputDataFn == nullptr) ||
            (m_connectionStatusReportFn == nullptr) ||
            (m_msgReportFn == nullptr)) {
            return MqttsnErrorCode_BadParam;
        }

        m_running = true;

        m_gwInfos.clear();
        m_regInfos.clear();
        m_nextTimeoutTimestamp = 0;
        m_lastGwSearchTimestamp = 0;
        m_lastMsgTimestamp = 0;
        m_lastPingTimestamp = 0;

        m_pingCount = 0;
        m_connectionStatus = ConnectionStatus::Disconnected;

        m_currOp = Op::None;
        m_timerActive = false;

        checkGwSearchReq();
        programNextTimeout();
        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode stop()
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        auto guard = apiCall();
        m_running = false;
        return MqttsnErrorCode_Success;
    }

    void tick(unsigned ms)
    {
        if (!m_running) {
            return;
        }

        GASSERT(m_callStackCount == 0U);
        m_timerActive = false;
        m_timestamp += ms;

        checkTimeouts();
        programNextTimeout();
    }

    std::size_t processData(ReadIterator& iter, std::size_t len)
    {
        if (!m_running) {
            return 0U;
        }

        auto guard = apiCall();
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

        return consumed;
    }

    bool cancel()
    {
        if (m_currOp == Op::None) {
            return false;
        }

        GASSERT(m_running);
        auto guard = apiCall();

        typedef void (Client<THandler, TClientOpts, TProtOpts>::*CancelFunc)();
        static const CancelFunc OpCancelFuncMap[] =
        {
            &Client::connectCancel,
            &Client::disconnectCancel,
            &Client::publishIdCancel,
            &Client::publishCancel,
            &Client::subscribeIdCancel,
            &Client::subscribeCancel,
            &Client::unsubscribeIdCancel,
            &Client::unsubscribeCancel,
            &Client::willUpdateCancel,
            &Client::willTopicUpdateCancel,
            &Client::willMsgUpdateCancel,
            &Client::sleepCancel,
            &Client::checkMessagesCancel
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
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus == ConnectionStatus::Connected) {
            return MqttsnErrorCode_AlreadyConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        auto guard = apiCall();

        bool reportDisconnected = (m_connectionStatus == ConnectionStatus::Asleep);
        m_connectionStatus = ConnectionStatus::Disconnected;
        m_currOp = Op::Connect;

        auto* connectOp = newOp<ConnectOp>();
        if (willInfo != nullptr) {
            connectOp->m_willInfo = *willInfo;
            connectOp->m_hasWill = true;
        }
        connectOp->m_clientId = clientId;
        connectOp->m_keepAlivePeriod = keepAlivePeriod;
        connectOp->m_cleanSession = cleanSession;

        bool result = doConnect();
        static_cast<void>(result);
        GASSERT(result);

        if (reportDisconnected) {
            GASSERT(m_connectionStatusReportFn != nullptr);
            m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_Disconnected);
        }

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode disconnect()
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus == ConnectionStatus::Disconnected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        auto guard = apiCall();
        m_currOp = Op::Disconnect;
        auto* disconnectOp = newOp<DisconnectOp>();
        static_cast<void>(disconnectOp);

        bool result = doDisconnect();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode publish(
        MqttsnTopicId topicId,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain,
        MqttsnPublishCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if ((m_connectionStatus != ConnectionStatus::Connected) && (qos != MqttsnQoS_NoGwPublish)) {
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

        auto guard = apiCall();

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
            sendPublish(
                topicId,
                allocMsgId(),
                msg,
                msgLen,
                mqttsn::protocol::field::TopicIdTypeVal::PreDefined,
                details::translateQosValue(qos),
                retain,
                false);
            if (callback != nullptr) {
                callback(data, MqttsnAsyncOpStatus_Successful);
            }
        }

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode publish(
        const char* topic,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain,
        MqttsnPublishCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if ((m_connectionStatus != ConnectionStatus::Connected) && (qos != MqttsnQoS_NoGwPublish)) {
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

        auto guard = apiCall();
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

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode subscribe(
        MqttsnTopicId topicId,
        MqttsnQoS qos,
        MqttsnSubscribeCompleteReportFn callback,
        void* data
    )
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Connected) {
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

        auto guard = apiCall();

        m_currOp = Op::SubscribeId;
        auto* op = newOp<SubscribeIdOp>();
        op->m_topicId = topicId;
        op->m_qos = qos;
        op->m_cb = callback;
        op->m_cbData = data;
        op->m_msgId = allocMsgId();

        bool result = doSubscribeId();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode subscribe(
        const char* topic,
        MqttsnQoS qos,
        MqttsnSubscribeCompleteReportFn callback,
        void* data
    )
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Connected) {
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

        auto guard = apiCall();

        m_currOp = Op::Subscribe;
        auto* op = newOp<SubscribeOp>();
        op->m_topic = topic;
        op->m_qos = qos;
        op->m_cb = callback;
        op->m_cbData = data;
        op->m_msgId = allocMsgId();

        bool result = doSubscribe();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode unsubscribe(
        MqttsnTopicId topicId,
        MqttsnUnsubscribeCompleteReportFn callback,
        void* data
    )
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Connected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if (callback == nullptr) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::UnsubscribeId;
        auto* op = newOp<UnsubscribeIdOp>();
        op->m_topicId = topicId;
        op->m_cb = callback;
        op->m_cbData = data;
        op->m_msgId = allocMsgId();

        bool result = doUnsubscribeId();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode unsubscribe(
        const char* topic,
        MqttsnUnsubscribeCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Connected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if (callback == nullptr) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::Unsubscribe;
        auto* op = newOp<UnsubscribeOp>();
        op->m_topic = topic;
        op->m_cb = callback;
        op->m_cbData = data;
        op->m_msgId = allocMsgId();

        bool result = doUnsubscribe();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode willUpdate(
        const MqttsnWillInfo* willInfo,
        MqttsnWillUpdateCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Connected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if (callback == nullptr) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::WillUpdate;
        auto* op = newOp<WillUpdateOp>();
        op->m_clientId = m_clientId.c_str();
        op->m_keepAlivePeriod = static_cast<decltype(op->m_keepAlivePeriod)>(m_keepAlivePeriod / 1000);
        if (willInfo != nullptr) {
            op->m_willInfo = *willInfo;
        }
        op->m_hasWill = true;
        op->m_cleanSession = false;
        op->m_cb = callback;
        op->m_cbData = data;

        bool result = doWillUpdate();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode willTopicUpdate(
        const char* topic,
        MqttsnQoS qos,
        bool retain,
        MqttsnWillTopicUpdateCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Connected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if (callback == nullptr) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::WillTopicUpdate;
        auto* op = newOp<WillTopicUpdateOp>();
        op->m_topic = topic;
        op->m_qos = qos;
        op->m_retain = retain;
        op->m_cb = callback;
        op->m_cbData = data;

        bool result = doWillTopicUpdate();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode willMsgUpdate(
        const unsigned char* msg,
        unsigned msgLen,
        MqttsnWillMsgUpdateCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Connected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if (callback == nullptr) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::WillMsgUpdate;
        auto* op = newOp<WillMsgUpdateOp>();
        op->m_msg = msg;
        op->m_msgLen = msgLen;
        op->m_cb = callback;
        op->m_cbData = data;

        bool result = doWillMsgUpdate();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode sleep(
        std::uint16_t duration,
        MqttsnSleepCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus == ConnectionStatus::Disconnected) {
            return MqttsnErrorCode_NotConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if (callback == nullptr) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::Sleep;
        auto* op = newOp<SleepOp>();
        op->m_duration = duration;
        op->m_cb = callback;
        op->m_cbData = data;

        bool result = doSleep();
        static_cast<void>(result);
        GASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode checkMessages(
        MqttsnCheckMessagesCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Asleep) {
            return MqttsnErrorCode_NotSleeping;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        if ((callback == nullptr)) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::CheckMessages;
        auto* op = newOp<CheckMessagesOp>();
        op->m_cb = callback;
        op->m_cbData = data;

        bool result = doCheckMessages();
        static_cast<void>(result);
        GASSERT(result);

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
        if ((m_currOp != Op::Connect) && (m_currOp != Op::WillUpdate)) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
        bool willReported = (op->m_willTopicSent && op->m_willMsgSent);
        if (op->m_hasWill && (!willReported)) {
            return;
        }

        auto& fields = msg.fields();
        auto& retCodeField = std::get<ConnackMsg::FieldIdx_returnCode>(fields);

        if (m_currOp == Op::WillUpdate) {
            finaliseWillUpdateOp(retCodeToStatus(retCodeField.value()));

            if (retCodeField.value() != mqttsn::protocol::field::ReturnCodeVal_Accepted) {
                m_connectionStatus = ConnectionStatus::Disconnected;
                m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_Disconnected);
            }
            return;
        }

        assert(m_currOp == Op::Connect);
        if (op->m_clientId != nullptr) {
            m_clientId = op->m_clientId;
        }
        else {
            m_clientId.clear();
        }

        m_keepAlivePeriod = op->m_keepAlivePeriod * 1000U;
        finaliseOp<ConnectOp>();

        static_cast<void>(msg);
        static const MqttsnConnectionStatus StatusMap[] = {
            /* ReturnCodeVal_Accepted */ MqttsnConnectionStatus_Connected,
            /* ReturnCodeVal_Congestion */ MqttsnConnectionStatus_Congestion,
            /* ReturnCodeVal_InvalidTopicId */ MqttsnConnectionStatus_Denied,
            /* ReturnCodeVal_NotSupported */ MqttsnConnectionStatus_Denied
        };

        static const std::size_t StatusMapSize =
                                std::extent<decltype(StatusMap)>::value;
        static_assert(
            StatusMapSize == mqttsn::protocol::field::ReturnCodeVal_NumOfValues,
            "Incorrect map");

        MqttsnConnectionStatus status = MqttsnConnectionStatus_Denied;
        auto returnCode = retCodeField.value();
        if (returnCode < StatusMapSize) {
            status = StatusMap[returnCode];
        }

        GASSERT(m_connectionStatus != ConnectionStatus::Connected);
        if (status == MqttsnConnectionStatus_Connected) {
            m_connectionStatus = ConnectionStatus::Connected;
        }

        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, status);
    }

    virtual void handle(WilltopicreqMsg& msg) override
    {
        static_cast<void>(msg);
        if ((m_currOp != Op::Connect) && (m_currOp != Op::WillUpdate)) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
        bool emptyTopic =
            (op->m_willInfo.topic == nullptr) || (op->m_willInfo.topic[0] == '\0');

        if ((m_currOp == Op::Connect) && (emptyTopic)) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        op->m_willTopicSent = true;
        if (emptyTopic) {
            op->m_willMsgSent = true;
        }

        sendWilltopic(op->m_willInfo.topic, op->m_willInfo.qos, op->m_willInfo.retain);
    }

    virtual void handle(WillmsgreqMsg& msg) override
    {
        static_cast<void>(msg);
        if ((m_currOp != Op::Connect) && (m_currOp != Op::WillUpdate)) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
        if (!op->m_willTopicSent) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        WillmsgMsg outMsg;
        auto& fields = outMsg.fields();
        auto& willMsgField = std::get<WillmsgMsg::FieldIdx_willMsg>(fields);

        if (op->m_willInfo.msg != nullptr) {
            willMsgField.value().assign(op->m_willInfo.msg, op->m_willInfo.msg + op->m_willInfo.msgLen);
        }

        op->m_willMsgSent = true;
        sendMessage(outMsg);
    }

    virtual void handle(RegisterMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& topicIdField = std::get<RegisterMsg::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<RegisterMsg::FieldIdx_msgId>(fields);
        auto& topicNameField = std::get<RegisterMsg::FieldIdx_topicName>(fields);

        updateRegInfo(topicNameField.value().c_str(), topicIdField.value(), true);

        RegackMsg ackMsg;
        auto& ackFields = ackMsg.fields();
        auto& ackTopicIdField = std::get<decltype(ackMsg)::FieldIdx_topicId>(ackFields);
        auto& ackMsgIdField = std::get<decltype(ackMsg)::FieldIdx_msgId>(ackFields);
        auto& ackRetCodeField = std::get<decltype(ackMsg)::FieldIdx_returnCode>(ackFields);
        ackTopicIdField.value() = topicIdField.value();
        ackMsgIdField.value() = msgIdField.value();
        ackRetCodeField.value() = mqttsn::protocol::field::ReturnCodeVal_Accepted;
        sendMessage(ackMsg);
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
            finalisePublishOp(retCodeToStatus(retCodeField.value()));
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        op->m_registered = true;
        op->m_topicId = topicIdField.value();
        op->m_attempt = 0;

        updateRegInfo(op->m_topic, op->m_topicId);
        bool result = doPublish();
        static_cast<void>(result);
        assert(result);
    }

    virtual void handle(PublishMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& flagsField = std::get<PublishMsg::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
        auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
        auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
        auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
        auto& topicIdField = std::get<PublishMsg::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<PublishMsg::FieldIdx_msgId>(fields);
        auto& dataField = std::get<PublishMsg::FieldIdx_data>(fields);

        auto reportMsgFunc =
            [this, &topicIdField, &dataField, &qosField, &midFlagsField](const char* topicName)
            {
                auto msgInfo = MqttsnMessageInfo();

                msgInfo.topic = topicName;
                if (topicName == nullptr) {
                    msgInfo.topicId = topicIdField.value();
                }
                msgInfo.msg = &(*dataField.value().begin());
                msgInfo.msgLen = dataField.value().size();
                msgInfo.qos = details::translateQosValue(qosField.value());
                msgInfo.retain = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_retain);

                assert(m_msgReportFn != nullptr);
                m_msgReportFn(m_msgReportData, &msgInfo);
            };

        auto iter = m_regInfos.end();
        if (topicIdTypeField.value() != mqttsn::protocol::field::TopicIdTypeVal::PreDefined) {
            iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [&topicIdField](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == topicIdField.value());
                });

            if (iter == m_regInfos.end()) {
                sendPuback(topicIdField.value(), msgIdField.value(), mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId);
            }
        }

        const char* topicName = nullptr;
        if (iter != m_regInfos.end()) {
            topicName = iter->m_topic.c_str();
        }

        if ((qosField.value() < mqttsn::protocol::field::QosType::AtLeastOnceDelivery) ||
            (mqttsn::protocol::field::QosType::ExactlyOnceDelivery < qosField.value())) {

            if ((iter == m_regInfos.end()) &&
                (topicIdTypeField.value() != mqttsn::protocol::field::TopicIdTypeVal::PreDefined)) {
                return;
            }

            reportMsgFunc(topicName);
            return;
        }

        if ((iter == m_regInfos.end()) &&
            (topicIdTypeField.value() != mqttsn::protocol::field::TopicIdTypeVal::PreDefined)) {
            m_lastInMsg = LastInMsgInfo();
            return;
        }

        if (qosField.value() == mqttsn::protocol::field::QosType::AtLeastOnceDelivery) {
            sendPuback(topicIdField.value(), msgIdField.value(), mqttsn::protocol::field::ReturnCodeVal_Accepted);
            reportMsgFunc(topicName);
            return;
        }

        GASSERT(qosField.value() == mqttsn::protocol::field::QosType::ExactlyOnceDelivery);

        bool newMessage =
            ((!dupFlagsField.getBitValue(mqttsn::protocol::field::DupFlagsBits_dup)) ||
             (topicIdField.value() != m_lastInMsg.m_topicId) ||
             (msgIdField.value() != m_lastInMsg.m_msgId) ||
             (m_lastInMsg.m_reported));

        if (newMessage) {
            m_lastInMsg = LastInMsgInfo();

            m_lastInMsg.m_topicId = topicIdField.value();
            m_lastInMsg.m_msgId = msgIdField.value();
            m_lastInMsg.m_retain =
                midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_retain);
        }

        m_lastInMsg.m_msgData = dataField.value();

        PubrecMsg recMsg;
        auto& recFields = recMsg.fields();
        auto& recMsgIdField = std::get<decltype(recMsg)::FieldIdx_msgId>(recFields);

        recMsgIdField.value() = msgIdField.value();
        sendMessage(recMsg);
    }

    virtual void handle(PubackMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& topicIdField = std::get<PubackMsg::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<PubackMsg::FieldIdx_msgId>(fields);
        auto& retCodeField = std::get<PubackMsg::FieldIdx_returnCode>(fields);

        if (retCodeField.value() == mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId) {

            auto iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [&topicIdField](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == topicIdField.value());
                });

            if (iter != m_regInfos.end()) {
                dropRegInfo(iter);
            }
        }

        if ((m_currOp != Op::Publish) && (m_currOp != Op::PublishId)) {
            return;
        }

        auto* op = opPtr<PublishOpBase>();


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

            op->m_lastMsgTimestamp = m_timestamp;
            op->m_topicId = 0U;
            op->m_attempt = 0;
            opPtr<PublishOp>()->m_registered = false;
            doPublish();
            return;
        } while (false);

        finalisePublishOp(retCodeToStatus(retCodeField.value()));
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

    virtual void handle(PubrelMsg& msg) override
    {
        auto& fields = msg.fields();
        auto& msgIdField = std::get<PubrelMsg::FieldIdx_msgId>(fields);

        if (m_lastInMsg.m_msgId != msgIdField.value()) {
            m_lastInMsg = LastInMsgInfo();
            return;
        }

        PubcompMsg compMsg;
        auto& compFields = compMsg.fields();
        auto& compMsgIdField = std::get<decltype(compMsg)::FieldIdx_msgId>(compFields);
        compMsgIdField.value() = msgIdField.value();
        sendMessage(compMsg);

        if (!m_lastInMsg.m_reported) {
            auto iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [this](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == m_lastInMsg.m_topicId);
                });

            auto msgInfo = MqttsnMessageInfo();

            if (iter != m_regInfos.end()) {
                msgInfo.topic = iter->m_topic.c_str();
            }

            msgInfo.topicId = m_lastInMsg.m_topicId;
            msgInfo.msg = &(*m_lastInMsg.m_msgData.begin());
            msgInfo.msgLen = m_lastInMsg.m_msgData.size();
            msgInfo.qos = MqttsnQoS_ExactlyOnceDelivery;
            msgInfo.retain = m_lastInMsg.m_retain;

            m_lastInMsg.m_reported = true;

            assert(m_msgReportFn != nullptr);
            m_msgReportFn(m_msgReportData, &msgInfo);
        }
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

    virtual void handle(SubackMsg& msg) override
    {
        if ((m_currOp != Op::Subscribe) && (m_currOp != Op::SubscribeId)) {
            return;
        }

        auto* op = opPtr<SubscribeOpBase>();

        auto& fields = msg.fields();
        auto& flagsField = std::get<SubackMsg::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
        auto& topicIdField = std::get<SubackMsg::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<SubackMsg::FieldIdx_msgId>(fields);
        auto& retCodeField = std::get<SubackMsg::FieldIdx_returnCode>(fields);

        if (msgIdField.value() != op->m_msgId) {
            return;
        }

        if ((retCodeField.value() == mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId) &&
            (m_currOp == Op::Subscribe) &&
            (opPtr<SubscribeOp>()->m_topicId != 0U)) {

            auto iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [this](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == opPtr<SubscribeOp>()->m_topicId);
                });

            if (iter != m_regInfos.end()) {
                dropRegInfo(iter);
            }

            op->m_attempt = 0;
            bool result = doSubscribe();
            static_cast<void>(result);
            GASSERT(result);
            return;
        }

        if (retCodeField.value() != mqttsn::protocol::field::ReturnCodeVal_Accepted) {
            finaliseSubscribeOp(retCodeToStatus(retCodeField.value()), details::translateQosValue(qosField.value()));
            return;
        }

        if ((m_currOp == Op::SubscribeId) &&
            (opPtr<SubscribeIdOp>()->m_topicId != topicIdField.value())) {
            if (!doSubscribeId()) {
                finaliseSubscribeOp(MqttsnAsyncOpStatus_InvalidId, op->m_qos);
            }
            return;
        }

        if ((m_currOp == Op::Subscribe) && (topicIdField.value() != 0U)) {
            updateRegInfo(opPtr<SubscribeOp>()->m_topic, topicIdField.value(), true);
        }

        finaliseSubscribeOp(MqttsnAsyncOpStatus_Successful, details::translateQosValue(qosField.value()));
    }

    virtual void handle(UnsubackMsg& msg) override
    {
        if ((m_currOp != Op::Unsubscribe) && (m_currOp != Op::UnsubscribeId)) {
            return;
        }

        auto* op = opPtr<UnsubscribeOpBase>();

        auto& fields = msg.fields();
        auto& msgIdField = std::get<UnsubackMsg::FieldIdx_msgId>(fields);

        if (msgIdField.value() != op->m_msgId) {
            return;
        }

        do {
            if (m_currOp != Op::Unsubscribe) {
                break;
            }

            auto* downcastedOp = opPtr<UnsubscribeOp>();

            if (downcastedOp->m_topicId == 0U) {
                break;
            }

            auto iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [downcastedOp](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == downcastedOp->m_topicId);
                });

            if (iter == m_regInfos.end()) {
                break;
            }

            iter->m_locked = false;
        } while (false);

        finaliseUnsubscribeOp(MqttsnAsyncOpStatus_Successful);
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
        bool pinging = (0U < m_pingCount);
        m_pingCount = 0U;

        if (pinging || (m_currOp != Op::CheckMessages)) {
            return;
        }

        // checking messages in asleep mode
        GASSERT(m_connectionStatus == ConnectionStatus::Asleep);
        finaliseCheckMessagesOp(MqttsnAsyncOpStatus_Successful);
    }

    virtual void handle(DisconnectMsg& msg) override
    {
        static_cast<void>(msg);

        if (m_currOp == Op::Disconnect) {
            finaliseOp<DisconnectOp>();
            reportDisconnected();
            return;
        }

        if (m_currOp == Op::Sleep) {
            finaliseSleepOp(MqttsnAsyncOpStatus_Successful);
            reportAsleep();
            return;
        }

        if (m_currOp == Op::None) {
            reportDisconnected();
            return;
        }

        cancel();
        reportDisconnected();
    }

    virtual void handle(WilltopicrespMsg& msg) override
    {
        if (m_currOp != Op::WillTopicUpdate) {
            return;
        }

        auto& fields = msg.fields();
        auto& retCodeField = std::get<WilltopicrespMsg::FieldIdx_returnCode>(fields);
        finaliseWillTopicUpdateOp(retCodeToStatus(retCodeField.value()));
    }

    virtual void handle(WillmsgrespMsg& msg) override
    {
        if (m_currOp != Op::WillMsgUpdate) {
            return;
        }

        auto& fields = msg.fields();
        auto& retCodeField = std::get<WillmsgrespMsg::FieldIdx_returnCode>(fields);
        finaliseWillMsgUpdateOp(retCodeToStatus(retCodeField.value()));
    }

private:

    typedef typename comms::util::AlignedUnion<
        ConnectOp,
        DisconnectOp,
        PublishIdOp,
        PublishOp,
        SubscribeIdOp,
        SubscribeOp,
        UnsubscribeIdOp,
        UnsubscribeOp,
        WillUpdateOp,
        WillTopicUpdateOp,
        WillMsgUpdateOp,
        SleepOp,
        CheckMessagesOp
    >::Type OpStorageType;


    typedef protocol::Stack<Message, AllMessages<TProtOpts>, comms::option::InPlaceAllocation> ProtStack;
    typedef typename ProtStack::MsgPtr MsgPtr;

    struct RegInfo
    {
        Timestamp m_timestamp = 0U;
        TopicNameType m_topic;
        TopicIdType m_topicId = 0U;
        bool m_allocated = false;
        bool m_locked = false;
    };

    typedef details::RegInfoStorageTypeT<RegInfo, TClientOpts> RegInfosList;

    struct LastInMsgInfo
    {
        DataType m_msgData;
        MqttsnTopicId m_topicId = 0;
        std::uint16_t m_msgId = 0;
        bool m_retain = false;
        bool m_reported = false;
    };

    void updateRegInfo(const char* topic, TopicIdType topicId, bool locked = false)
    {
        auto iter = std::find_if(
            m_regInfos.begin(), m_regInfos.end(),
            [topic, topicId](typename RegInfosList::const_reference elem) -> bool
            {
                return elem.m_allocated && (elem.m_topic == topic);
            });

        if (iter != m_regInfos.end()) {
            iter->m_timestamp = m_timestamp;
            iter->m_topicId = topicId;
            iter->m_locked = (iter->m_locked || locked);
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
            iter->m_topic = topic;
            iter->m_topicId = topicId;
            iter->m_allocated = true;
            iter->m_locked = locked;
            return;
        }

        RegInfo info;
        info.m_timestamp = m_timestamp;
        info.m_topic = topic;
        info.m_topicId = topicId;
        info.m_allocated = true;
        info.m_locked = locked;

        if (m_regInfos.size() < m_regInfos.max_size()) {
            m_regInfos.push_back(std::move(info));
            return;
        }

        iter = std::min_element(
            m_regInfos.begin(), m_regInfos.end(),
            [](typename RegInfosList::const_reference elem1,
               typename RegInfosList::const_reference elem2) -> bool
            {
                if (elem1.m_locked == elem2.m_locked) {
                    return (elem1.m_timestamp < elem2.m_timestamp);
                }

                if (elem1.m_locked) {
                    return false;
                }

                return true;
            });

        GASSERT(iter != m_regInfos.end());
        *iter = info;
    }

    void dropRegInfo(typename RegInfosList::iterator iter)
    {
        *iter = RegInfo();
    }

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
        if ((m_gwInfos.empty()) || (m_connectionStatus == ConnectionStatus::Asleep)) {
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
        if ((!m_searchgwEnabled) ||
            (!m_gwInfos.empty()) ||
            (m_connectionStatus == ConnectionStatus::Asleep)) {
            return NoTimeout;
        }

        if (m_lastGwSearchTimestamp == 0) {
            return 0U;
        }

        auto nextSearchTimestamp = m_lastGwSearchTimestamp + m_retryPeriod;
        if (nextSearchTimestamp <= m_timestamp) {
            return 0U;
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
        if (m_connectionStatus != ConnectionStatus::Connected) {
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
        if (!m_running) {
            return;
        }

        unsigned delay = NoTimeout;
        delay = std::min(delay, calcGwReleaseTimeout());
        delay = std::min(delay, std::max(1U, calcSearchGwSendTimeout()));
        delay = std::min(delay, calcCurrentOpTimeout());
        delay = std::min(delay, calcPingTimeout());

        if (delay == NoTimeout) {
            return;
        }

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

        typedef details::GwInfoStorageTypeT<std::uint16_t, TClientOpts> GwIdStorage;
        GwIdStorage idsToRemove;

        for (auto& info : m_gwInfos) {
            if (checkMustRemoveFunc(info)) {
                idsToRemove.push_back(info.m_id);
            }
        }

        if (idsToRemove.empty()) {
            return;
        }

        m_gwInfos.erase(
            std::remove_if(
                m_gwInfos.begin(), m_gwInfos.end(),
                checkMustRemoveFunc),
            m_gwInfos.end());

        for (auto id : idsToRemove) {
            reportGwStatus(id, MqttsnGwStatus_TimedOut);
        }
    }

    void checkGwSearchReq()
    {
        if (calcSearchGwSendTimeout() == 0) {
            sendGwSearchReq();
        }
    }

    void checkPing()
    {
        if (m_connectionStatus != ConnectionStatus::Connected) {
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
            &Client::publishIdTimeout,
            &Client::publishTimeout,
            &Client::subscribeIdTimeout,
            &Client::subscribeTimeout,
            &Client::unsubscribeIdTimeout,
            &Client::unsubscribeTimeout,
            &Client::willUpdateTimeout,
            &Client::willTopicUpdateTimeout,
            &Client::willMsgUpdateTimeout,
            &Client::sleepTimeout,
            &Client::checkMessagesTimeout
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
        auto& radiusField = std::get<SearchgwMsg::FieldIdx_radius>(fields);
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

    void subscribeIdTimeout()
    {
        GASSERT(m_currOp == Op::SubscribeId);

        if (doSubscribeId()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseSubscribeOp(MqttsnAsyncOpStatus_NoResponse, opPtr<SubscribeIdOp>()->m_qos);
    }

    void subscribeIdCancel()
    {
        GASSERT(m_currOp == Op::SubscribeId);
        finaliseSubscribeOp(MqttsnAsyncOpStatus_Aborted, opPtr<SubscribeIdOp>()->m_qos);
    }

    void subscribeTimeout()
    {
        GASSERT(m_currOp == Op::Subscribe);

        if (doSubscribe()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseSubscribeOp(MqttsnAsyncOpStatus_NoResponse, opPtr<SubscribeOp>()->m_qos);
    }

    void subscribeCancel()
    {
        GASSERT(m_currOp == Op::Subscribe);
        finaliseSubscribeOp(MqttsnAsyncOpStatus_Aborted, opPtr<SubscribeOp>()->m_qos);
    }

    void unsubscribeIdTimeout()
    {
        GASSERT(m_currOp == Op::UnsubscribeId);

        if (doUnsubscribeId()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseUnsubscribeOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void unsubscribeIdCancel()
    {
        GASSERT(m_currOp == Op::UnsubscribeId);
        finaliseUnsubscribeOp(MqttsnAsyncOpStatus_Aborted);
    }

    void unsubscribeTimeout()
    {
        GASSERT(m_currOp == Op::Unsubscribe);

        if (doUnsubscribe()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseUnsubscribeOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void unsubscribeCancel()
    {
        GASSERT(m_currOp == Op::Unsubscribe);
        finaliseUnsubscribeOp(MqttsnAsyncOpStatus_Aborted);
    }

    void willUpdateTimeout()
    {
        GASSERT(m_currOp == Op::WillUpdate);

        if (doWillUpdate()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseWillUpdateOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void willUpdateCancel()
    {
        GASSERT(m_currOp == Op::WillUpdate);
        finaliseWillUpdateOp(MqttsnAsyncOpStatus_Aborted);
    }

    void willTopicUpdateTimeout()
    {
        GASSERT(m_currOp == Op::WillTopicUpdate);

        if (doWillTopicUpdate()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseWillTopicUpdateOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void willTopicUpdateCancel()
    {
        GASSERT(m_currOp == Op::WillTopicUpdate);
        finaliseWillTopicUpdateOp(MqttsnAsyncOpStatus_Aborted);
    }


    void willMsgUpdateTimeout()
    {
        GASSERT(m_currOp == Op::WillMsgUpdate);

        if (doWillMsgUpdate()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseWillMsgUpdateOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void willMsgUpdateCancel()
    {
        GASSERT(m_currOp == Op::WillMsgUpdate);
        finaliseWillMsgUpdateOp(MqttsnAsyncOpStatus_Aborted);
    }


    void sleepTimeout()
    {
        GASSERT(m_currOp == Op::Sleep);

        if (doSleep()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseSleepOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void sleepCancel()
    {
        GASSERT(m_currOp == Op::Sleep);
        finaliseSleepOp(MqttsnAsyncOpStatus_Aborted);
    }

    void checkMessagesTimeout()
    {
        GASSERT(m_currOp == Op::CheckMessages);

        if (doCheckMessages()) {
            opPtr<OpBase>()->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        finaliseCheckMessagesOp(MqttsnAsyncOpStatus_NoResponse);
    }

    void checkMessagesCancel()
    {
        GASSERT(m_currOp == Op::CheckMessages);
        finaliseCheckMessagesOp(MqttsnAsyncOpStatus_Aborted);
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
        sendConnect(op->m_clientId, op->m_keepAlivePeriod, op->m_cleanSession, op->m_hasWill);
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
        auto& fields = msg.fields();
        auto& durationField = std::get<decltype(msg)::FieldIdx_duration>(fields);
        durationField.setMode(comms::field::OptionalMode::Missing);
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

        bool firstAttempt = (op->m_attempt == 0U);
        ++op->m_attempt;

        if (firstAttempt) {
            op->m_msgId = allocMsgId();
        }

        if ((!firstAttempt) &&
            (op->m_ackReceived) &&
            (MqttsnQoS_ExactlyOnceDelivery <= op->m_qos)) {
            sendPubrel(op->m_msgId);
            return true;
        }

        sendPublish(
            op->m_topicId,
            op->m_msgId,
            op->m_msg,
            op->m_msgLen,
            mqttsn::protocol::field::TopicIdTypeVal::PreDefined,
            details::translateQosValue(op->m_qos),
            op->m_retain,
            !firstAttempt);

        if (op->m_qos <= MqttsnQoS_AtMostOnceDelivery) {
            finalisePublishOp(MqttsnAsyncOpStatus_Successful);
        }

        return true;
    }

    bool doPublish()
    {
        GASSERT (m_currOp == Op::Publish);

        auto* op = opPtr<PublishOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        bool firstAttempt = (op->m_attempt == 0U);
        ++op->m_attempt;

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
            op->m_msgId = allocMsgId();
            sendRegister(op->m_msgId, op->m_topic);
            return true;
        } while (false);

        if (firstAttempt) {
            op->m_msgId = allocMsgId();
        }

        assert(op->m_registered);

        if ((!firstAttempt) &&
            (op->m_ackReceived) &&
            (MqttsnQoS_ExactlyOnceDelivery <= op->m_qos)) {
            sendPubrel(op->m_msgId);
            return true;
        }

        sendPublish(
            op->m_topicId,
            op->m_msgId,
            op->m_msg,
            op->m_msgLen,
            mqttsn::protocol::field::TopicIdTypeVal::Normal,
            details::translateQosValue(op->m_qos),
            op->m_retain,
            !firstAttempt);

        if (op->m_qos <= MqttsnQoS_AtMostOnceDelivery) {
            finalisePublishOp(MqttsnAsyncOpStatus_Successful);
        }

        return true;
    }

    bool doSubscribeId()
    {
        GASSERT (m_currOp == Op::SubscribeId);

        auto* op = opPtr<SubscribeIdOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        bool firstAttempt = (op->m_attempt == 0U);
        ++op->m_attempt;

        SubscribeMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
        auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
        auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
        auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
        auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);

        topicIdTypeField.value() = mqttsn::protocol::field::TopicIdTypeVal::PreDefined;
        qosField.value() = details::translateQosValue(op->m_qos);
        dupFlagsField.setBitValue(mqttsn::protocol::field::DupFlagsBits_dup, !firstAttempt);
        msgIdField.value() = op->m_msgId;
        topicIdField.field().value() = op->m_topicId;
        msg.refresh();
        GASSERT(topicIdField.getMode() == comms::field::OptionalMode::Exists);
        sendMessage(msg);
        return true;
    }

    bool doSubscribe()
    {
        GASSERT (m_currOp == Op::Subscribe);

        auto* op = opPtr<SubscribeOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        bool firstAttempt = (op->m_attempt == 0U);
        ++op->m_attempt;

        auto iter = std::find_if(
            m_regInfos.begin(), m_regInfos.end(),
            [op](typename RegInfosList::const_reference elem) -> bool
            {
                return elem.m_allocated && (elem.m_topic == op->m_topic);
            });

        if (iter != m_regInfos.end()) {
            op->m_topicId = iter->m_topicId;
        }
        else {
            op->m_topicId = 0U;
        }

        SubscribeMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
        auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
        auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
        auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
        auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);
        auto& topicNameField = std::get<decltype(msg)::FieldIdx_topicName>(fields);

        if (op->m_topicId != 0U) {
            topicIdTypeField.value() = mqttsn::protocol::field::TopicIdTypeVal::Normal;
            topicIdField.field().value() = iter->m_topicId;
        }
        else {
            topicIdTypeField.value() = mqttsn::protocol::field::TopicIdTypeVal::Name;
            topicNameField.field().value() = op->m_topic;
        }

        qosField.value() = details::translateQosValue(op->m_qos);
        dupFlagsField.setBitValue(mqttsn::protocol::field::DupFlagsBits_dup, !firstAttempt);
        msgIdField.value() = op->m_msgId;
        msg.refresh();
        GASSERT((op->m_topicId != 0U) || (topicNameField.getMode() == comms::field::OptionalMode::Exists));
        GASSERT((op->m_topicId != 0U) || (topicIdField.getMode() == comms::field::OptionalMode::Missing));
        GASSERT((op->m_topicId == 0U) || (topicNameField.getMode() == comms::field::OptionalMode::Missing));
        GASSERT((op->m_topicId == 0U) || (topicIdField.getMode() == comms::field::OptionalMode::Exists));

        sendMessage(msg);
        return true;
    }

    bool doUnsubscribeId()
    {
        GASSERT (m_currOp == Op::UnsubscribeId);

        auto* op = opPtr<UnsubscribeIdOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        UnsubscribeMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
        auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
        auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);

        topicIdTypeField.value() = mqttsn::protocol::field::TopicIdTypeVal::PreDefined;
        msgIdField.value() = op->m_msgId;
        topicIdField.field().value() = op->m_topicId;
        msg.refresh();
        GASSERT(topicIdField.getMode() == comms::field::OptionalMode::Exists);
        sendMessage(msg);
        return true;
    }

    bool doUnsubscribe()
    {
        GASSERT (m_currOp == Op::Unsubscribe);

        auto* op = opPtr<UnsubscribeOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        auto iter = std::find_if(
            m_regInfos.begin(), m_regInfos.end(),
            [op](typename RegInfosList::const_reference elem) -> bool
            {
                return elem.m_allocated && (elem.m_topic == op->m_topic);
            });

        if (iter != m_regInfos.end()) {
            op->m_topicId = iter->m_topicId;
        }
        else {
            op->m_topicId = 0U;
        }

        UnsubscribeMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
        auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
        auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);
        auto& topicNameField = std::get<decltype(msg)::FieldIdx_topicName>(fields);

        if (op->m_topicId != 0U) {
            topicIdTypeField.value() = mqttsn::protocol::field::TopicIdTypeVal::Normal;
            topicIdField.field().value() = iter->m_topicId;
        }
        else {
            topicIdTypeField.value() = mqttsn::protocol::field::TopicIdTypeVal::Name;
            topicNameField.field().value() = op->m_topic;
        }

        msgIdField.value() = op->m_msgId;
        msg.refresh();
        GASSERT((op->m_topicId != 0U) || (topicNameField.getMode() == comms::field::OptionalMode::Exists));
        GASSERT((op->m_topicId != 0U) || (topicIdField.getMode() == comms::field::OptionalMode::Missing));
        GASSERT((op->m_topicId == 0U) || (topicNameField.getMode() == comms::field::OptionalMode::Missing));
        GASSERT((op->m_topicId == 0U) || (topicIdField.getMode() == comms::field::OptionalMode::Exists));

        sendMessage(msg);
        return true;
    }

    bool doWillUpdate()
    {
        GASSERT (m_currOp == Op::WillUpdate);

        auto* op = opPtr<WillUpdateOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        sendConnect(op->m_clientId, op->m_keepAlivePeriod, false, true);
        return true;
    }

    bool doWillTopicUpdate()
    {
        GASSERT (m_currOp == Op::WillTopicUpdate);

        auto* op = opPtr<WillTopicUpdateOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;
        WilltopicupdMsg msg;
        GASSERT(std::get<decltype(msg)::FieldIdx_flags>(msg.fields()).getMode() == comms::field::OptionalMode::Missing);
        if ((op->m_topic != nullptr) && (op->m_topic[0] != '\0')) {
            auto& fields = msg.fields();
            auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
            auto& flagsMembers = flagsField.field().value();
            auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
            auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
            auto& topicField = std::get<decltype(msg)::FieldIdx_willTopic>(fields);

            qosField.value() = details::translateQosValue(op->m_qos);
            midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_retain, op->m_retain);
            topicField.value() = op->m_topic;

            msg.refresh();
            GASSERT(flagsField.getMode() == comms::field::OptionalMode::Exists);
        }

        sendMessage(msg);
        return true;
    }

    bool doWillMsgUpdate()
    {
        GASSERT (m_currOp == Op::WillMsgUpdate);

        auto* op = opPtr<WillMsgUpdateOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        WillmsgupdMsg msg;
        auto& fields = msg.fields();
        auto& msgBodyField = std::get<WillmsgupdMsg::FieldIdx_willMsg>(fields);
        auto* msgBodyBeg = op->m_msg;
        if (msgBodyBeg != nullptr) {
            auto* msgBodyEnd = msgBodyBeg + op->m_msgLen;
            msgBodyField.value().assign(msgBodyBeg, msgBodyEnd);
        }
        sendMessage(msg);
        return true;
    }

    bool doSleep()
    {
        GASSERT (m_currOp == Op::Sleep);

        auto* op = opPtr<SleepOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        DisconnectMsg msg;
        auto& fields = msg.fields();
        auto& durationField = std::get<decltype(msg)::FieldIdx_duration>(fields);

        durationField.setMode(comms::field::OptionalMode::Exists);
        durationField.field().value() = op->m_duration;
        sendMessage(msg);
        return true;
    }

    bool doCheckMessages()
    {
        GASSERT (m_currOp == Op::CheckMessages);

        auto* op = opPtr<CheckMessagesOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        PingreqMsg msg;
        auto& fields = msg.fields();
        auto& clientIdField = std::get<decltype(msg)::FieldIdx_clientId>(fields);

        clientIdField.value() = m_clientId;
        sendMessage(msg);
        return true;
    }

    void sendConnect(
        const char* clientId,
        std::uint16_t keepAlivePeriod,
        bool cleanSession,
        bool hasWill)
    {
        ConnectMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
        auto& durationField = std::get<decltype(msg)::FieldIdx_duration>(fields);
        auto& clientIdField = std::get<decltype(msg)::FieldIdx_clientId>(fields);

        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_cleanSession, cleanSession);
        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_will, hasWill);

        durationField.value() = keepAlivePeriod;
        if (clientId != nullptr) {
            clientIdField.value() = clientId;
        }
        sendMessage(msg);
    }

    void sendWilltopic(
        const char* topic,
        MqttsnQoS qos,
        bool retain)
    {
        WilltopicMsg msg;
        if (topic != nullptr) {
            auto& fields = msg.fields();
            auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
            auto& flagsMembers = flagsField.field().value();
            auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
            auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
            auto& topicField = std::get<decltype(msg)::FieldIdx_willTopic>(fields);

            midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_retain, retain);
            qosField.value() = details::translateQosValue(qos);
            topicField.value() = topic;
        }
        msg.refresh();
        sendMessage(msg);
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
        mqttsn::protocol::field::TopicIdTypeVal topicIdType,
        mqttsn::protocol::field::QosType qos,
        bool retain,
        bool duplicate)
    {
        PublishMsg pubMsg;
        auto& fields = pubMsg.fields();
        auto& flagsField = std::get<PublishMsg::FieldIdx_flags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& topicIdTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
        auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
        auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
        auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
        auto& topicIdField = std::get<PublishMsg::FieldIdx_topicId>(fields);
        auto& msgIdField = std::get<PublishMsg::FieldIdx_msgId>(fields);
        auto& dataField = std::get<PublishMsg::FieldIdx_data>(fields);

        topicIdTypeField.value() = topicIdType;
        midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_retain, retain);
        qosField.value() = qos;
        dupFlagsField.setBitValue(mqttsn::protocol::field::DupFlagsBits_dup, duplicate);
        topicIdField.value() = topicId;
        msgIdField.value() = msgId;
        dataField.value().assign(msg, msg + msgLen);

        sendMessage(pubMsg);
    }

    void sendPuback(
        MqttsnTopicId topicId,
        std::uint16_t msgId,
        mqttsn::protocol::field::ReturnCodeVal retCode)
    {
        PubackMsg ackMsg;
        auto& ackFields = ackMsg.fields();
        auto& ackTopicIdField = std::get<decltype(ackMsg)::FieldIdx_topicId>(ackFields);
        auto& ackMsgIdField = std::get<decltype(ackMsg)::FieldIdx_msgId>(ackFields);
        auto& ackRetCodeField = std::get<decltype(ackMsg)::FieldIdx_returnCode>(ackFields);

        ackTopicIdField.value() = topicId;
        ackMsgIdField.value() = msgId;
        ackRetCodeField.value() = retCode;
        sendMessage(ackMsg);
    }

    void sendRegister(
        std::uint16_t msgId,
        const char* topic)
    {
        RegisterMsg msg;
        auto& fields = msg.fields();
        auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
        auto& topicNameField = std::get<decltype(msg)::FieldIdx_topicName>(fields);

        msgIdField.value() = msgId;
        topicNameField.value() = topic;
        sendMessage(msg);
    }

    void sendPubrel(std::uint16_t msgId)
    {
        PubrelMsg msg;
        auto& fields = msg.fields();
        auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
        msgIdField.value() = msgId;
        sendMessage(msg);
    }

    void reportDisconnected()
    {
        m_connectionStatus = ConnectionStatus::Disconnected;
        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_Disconnected);
    }

    void reportAsleep()
    {
        m_connectionStatus = ConnectionStatus::Asleep;
        GASSERT(m_connectionStatusReportFn != nullptr);
        m_connectionStatusReportFn(m_connectionStatusReportData, MqttsnConnectionStatus_DisconnectedAsleep);
    }

    std::uint16_t allocMsgId()
    {
        ++m_msgId;
        return m_msgId;
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

    void finaliseSubscribeOp(MqttsnAsyncOpStatus status, MqttsnQoS qos)
    {
        GASSERT((m_currOp == Op::Subscribe) || (m_currOp == Op::SubscribeId));
        auto* op = opPtr<SubscribeOpBase>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        if (m_currOp == Op::Subscribe) {
            finaliseOp<SubscribeOp>();
        }
        else {
            finaliseOp<SubscribeIdOp>();
        }
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status, qos);
    }

    void finaliseUnsubscribeOp(MqttsnAsyncOpStatus status)
    {
        GASSERT((m_currOp == Op::Unsubscribe) || (m_currOp == Op::UnsubscribeId));
        auto* op = opPtr<UnsubscribeOpBase>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        if (m_currOp == Op::Unsubscribe) {
            finaliseOp<UnsubscribeOp>();
        }
        else {
            finaliseOp<UnsubscribeIdOp>();
        }
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status);
    }

    void finaliseWillUpdateOp(MqttsnAsyncOpStatus status)
    {
        GASSERT(m_currOp == Op::WillUpdate);
        auto* op = opPtr<WillUpdateOp>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        finaliseOp<WillUpdateOp>();
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status);
    }

    void finaliseWillTopicUpdateOp(MqttsnAsyncOpStatus status)
    {
        GASSERT(m_currOp == Op::WillTopicUpdate);
        auto* op = opPtr<WillTopicUpdateOp>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        finaliseOp<WillTopicUpdateOp>();
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status);
    }

    void finaliseWillMsgUpdateOp(MqttsnAsyncOpStatus status)
    {
        GASSERT(m_currOp == Op::WillMsgUpdate);
        auto* op = opPtr<WillMsgUpdateOp>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        finaliseOp<WillMsgUpdateOp>();
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status);
    }

    void finaliseSleepOp(MqttsnAsyncOpStatus status)
    {
        GASSERT(m_currOp == Op::Sleep);
        auto* op = opPtr<SleepOp>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        finaliseOp<SleepOp>();
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status);
    }

    void finaliseCheckMessagesOp(MqttsnAsyncOpStatus status)
    {
        GASSERT(m_currOp == Op::CheckMessages);
        auto* op = opPtr<CheckMessagesOp>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        finaliseOp<CheckMessagesOp>();
        GASSERT(m_currOp == Op::None);
        GASSERT(cb != nullptr);
        cb(cbData, status);
    }

    MqttsnAsyncOpStatus retCodeToStatus(mqttsn::protocol::field::ReturnCodeVal val)
    {
        static const MqttsnAsyncOpStatus Map[] = {
            /* ReturnCodeVal_Accepted */ MqttsnAsyncOpStatus_Successful,
            /* ReturnCodeVal_Congestion */ MqttsnAsyncOpStatus_Congestion,
            /* ReturnCodeVal_InvalidTopicId */ MqttsnAsyncOpStatus_InvalidId,
            /* ReturnCodeVal_NotSupported */ MqttsnAsyncOpStatus_NotSupported
        };

        static const std::size_t MapSize = std::extent<decltype(Map)>::value;

        static_assert(MapSize == (unsigned)mqttsn::protocol::field::ReturnCodeVal_NumOfValues,
            "Map is incorrect");

        MqttsnAsyncOpStatus status = MqttsnAsyncOpStatus_NotSupported;
        if (static_cast<unsigned>(val) < MapSize) {
            status = Map[static_cast<unsigned>(val)];
        }

        return status;
    }

    void apiCallExit()
    {
        GASSERT(0U < m_callStackCount);
        --m_callStackCount;
        if (m_callStackCount == 0U) {
            programNextTimeout();
        }
    }

    auto apiCall() -> decltype(comms::util::makeScopeGuard(std::bind(&Client<THandler, TClientOpts, TProtOpts>::apiCallExit, this)))
    {
        ++m_callStackCount;
        if (m_callStackCount == 1U) {
            updateTimestamp();
        }

        return
            comms::util::makeScopeGuard(
                std::bind(
                    &Client<THandler, TClientOpts, TProtOpts>::apiCallExit,
                    this));
    }


    ProtStack m_stack;
    GwInfoStorage m_gwInfos;
    Timestamp m_timestamp = DefaultStartTimestamp;
    Timestamp m_nextTimeoutTimestamp = 0;
    Timestamp m_lastGwSearchTimestamp = 0;
    Timestamp m_lastMsgTimestamp = 0;
    Timestamp m_lastPingTimestamp = 0;
    ClientIdType m_clientId;

    unsigned m_callStackCount = 0U;
    unsigned m_pingCount = 0;
    unsigned m_advertisePeriod = DefaultAdvertisePeriod;
    unsigned m_retryPeriod = DefaultRetryPeriod;
    unsigned m_retryCount = DefaultRetryCount;
    unsigned m_keepAlivePeriod = 0;
    ConnectionStatus m_connectionStatus = ConnectionStatus::Disconnected;
    std::uint16_t m_msgId = 0;
    std::uint8_t m_broadcastRadius = DefaultBroadcastRadius;

    bool m_running = false;
    bool m_timerActive = false;
    bool m_searchgwEnabled = true;

    Op m_currOp = Op::None;
    OpStorageType m_opStorage;

    RegInfosList m_regInfos;

    LastInMsgInfo m_lastInMsg;

    MqttsnNextTickProgramFn m_nextTickProgramFn = nullptr;
    void* m_nextTickProgramData = nullptr;

    MqttsnCancelNextTickWaitFn m_cancelNextTickWaitFn = nullptr;
    void* m_cancelNextTickWaitData = nullptr;

    MqttsnSendOutputDataFn m_sendOutputDataFn = nullptr;
    void* m_sendOutputDataData = nullptr;

    MqttsnGwStatusReportFn m_gwStatusReportFn = nullptr;
    void* m_gwStatusReportData = nullptr;

    MqttsnConnectionStatusReportFn m_connectionStatusReportFn = nullptr;
    void* m_connectionStatusReportData = nullptr;

    MqttsnMessageReportFn m_msgReportFn = nullptr;
    void* m_msgReportData = nullptr;

    static const unsigned DefaultAdvertisePeriod = 30 * 60 * 1000;
    static const unsigned DefaultRetryPeriod = 15 * 1000;
    static const unsigned DefaultRetryCount = 3;
    static const std::uint8_t DefaultBroadcastRadius = 0U;

    static const unsigned NoTimeout = std::numeric_limits<unsigned>::max();
    static const Timestamp DefaultStartTimestamp = 100;
};

}  // namespace client

}  // namespace mqttsn


