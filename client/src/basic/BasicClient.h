//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as ed by
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
#include <cstring>

#include "comms/comms.h"
#include "comms/util/ScopeGuard.h"
#include "mqttsn/client/common.h"
#include "mqttsn/Message.h"
#include "mqttsn/frame/Frame.h"
#include "mqttsn/input/ClientInputMessages.h"
#include "mqttsn/options/ClientDefaultOptions.h"
#include "details/WriteBufStorageType.h"

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
    typename RegInfoStorageType<TInfo, TOpts, TOpts::HasRegisteredTopicsLimit>::Type;

//-----------------------------------------------------------

template <typename TOpts, bool THasStaticSize>
struct TopicNameStorageOpt;

template <typename TOpts>
struct TopicNameStorageOpt<TOpts, true>
{
    typedef comms::option::FixedSizeStorage<TOpts::TopicNameStaticStorageSize> Type;
};

template <typename TOpts>
struct TopicNameStorageOpt<TOpts, false>
{
    typedef comms::option::EmptyOption Type;
};

template <typename TOpts>
using TopicNameStorageOptT =
    typename TopicNameStorageOpt<TOpts, TOpts::HasTopicNameStaticStorageSize>::Type;

//-----------------------------------------------------------

template <typename TOpts, bool THasStaticSize>
struct DataStorageOpt;

template <typename TOpts>
struct DataStorageOpt<TOpts, true>
{
    typedef comms::option::FixedSizeStorage<TOpts::MessageDataStaticStorageSize> Type;
};

template <typename TOpts>
struct DataStorageOpt<TOpts, false>
{
    typedef comms::option::EmptyOption Type;
};

template <typename TOpts>
using DataStorageOptT =
    typename DataStorageOpt<TOpts, TOpts::HasMessageDataStaticStorageSize>::Type;

//-----------------------------------------------------------

template <typename TOpts, bool THasStaticSize>
struct ClientIdStorageOpt;

template <typename TOpts>
struct ClientIdStorageOpt<TOpts, true>
{
    typedef comms::option::FixedSizeStorage<TOpts::TopicNameStaticStorageSize> Type;
};

template <typename TOpts>
struct ClientIdStorageOpt<TOpts, false>
{
    typedef comms::option::EmptyOption Type;
};

template <typename TOpts>
using ClientIdStorageOptT =
    typename ClientIdStorageOpt<TOpts, TOpts::HasTopicNameStaticStorageSize>::Type;

//-----------------------------------------------------------

mqttsn::field::QosVal translateQosValue(MqttsnQoS val)
{
    static_assert(
        (int)mqttsn::field::QosVal::AtMostOnceDelivery == MqttsnQoS_AtMostOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::field::QosVal::AtLeastOnceDelivery == MqttsnQoS_AtLeastOnceDelivery,
        "Invalid mapping");

    static_assert(
        (int)mqttsn::field::QosVal::ExactlyOnceDelivery == MqttsnQoS_ExactlyOnceDelivery,
        "Invalid mapping");

    if (val == MqttsnQoS_NoGwPublish) {
        return mqttsn::field::QosVal::NoGwPublish;
    }

    return static_cast<mqttsn::field::QosVal>(val);
}

MqttsnQoS translateQosValue(mqttsn::field::QosVal val)
{

    if (val == mqttsn::field::QosVal::NoGwPublish) {
        return MqttsnQoS_NoGwPublish;
    }

    return static_cast<MqttsnQoS>(val);
}

}  // namespace details

template <typename TClientOpts>
class BasicClient
{
    typedef details::WriteBufStorageTypeT<TClientOpts> WriteBufStorage;

    typedef mqttsn::Message<
        comms::option::IdInfoInterface,
        comms::option::ReadIterator<const std::uint8_t*>,
        comms::option::WriteIterator<std::uint8_t*>,
        comms::option::Handler<BasicClient<TClientOpts> >,
        comms::option::LengthInfoInterface
    > Message;

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

    struct AsyncOpBase : public OpBase
    {
        MqttsnAsyncOpCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
    };


    struct ConnectOp : public AsyncOpBase
    {
        MqttsnWillInfo m_willInfo = MqttsnWillInfo();
        const char* m_clientId = nullptr;
        std::uint16_t m_keepAlivePeriod = 0;
        bool m_hasWill = false;
        bool m_cleanSession = false;
        bool m_willTopicSent = false;
        bool m_willMsgSent = false;
    };

    typedef AsyncOpBase DisconnectOp;

    struct PublishOpBase : public AsyncOpBase
    {
        MqttsnTopicId m_topicId = 0U;
        std::uint16_t m_msgId = 0U;
        const std::uint8_t* m_msg = nullptr;
        std::size_t m_msgLen = 0;
        MqttsnQoS m_qos = MqttsnQoS_AtMostOnceDelivery;
        bool m_retain = false;
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
        bool m_shortName = false;
    };

    struct SubscribeOpBase : public OpBase
    {
        MqttsnQoS m_qos = MqttsnQoS_AtMostOnceDelivery;
        MqttsnSubscribeCompleteReportFn m_cb = nullptr;
        void* m_cbData = nullptr;
        std::uint16_t m_msgId = 0U;
        MqttsnTopicId m_topicId = 0U;
    };

    struct SubscribeIdOp : public SubscribeOpBase
    {
    };

    struct SubscribeOp : public SubscribeOpBase
    {
       const char* m_topic = nullptr;
       bool m_usingShortTopicName = false;
    };

    struct UnsubscribeOpBase : public AsyncOpBase
    {
        std::uint16_t m_msgId = 0U;
        MqttsnTopicId m_topicId = 0U;
    };

    struct UnsubscribeIdOp : public UnsubscribeOpBase
    {
    };

    struct UnsubscribeOp : public UnsubscribeOpBase
    {
        const char* m_topic = nullptr;
        bool m_usingShortTopicName = false;
    };

    struct WillTopicUpdateOp : public AsyncOpBase
    {
        const char* m_topic = nullptr;
        MqttsnQoS m_qos;
        bool m_retain;
    };

    struct WillMsgUpdateOp : public AsyncOpBase
    {
        const unsigned char* m_msg = nullptr;
        unsigned m_msgLen = 0;
    };

    struct SleepOp : public AsyncOpBase
    {
        std::uint16_t m_duration = 0;
    };

    typedef AsyncOpBase CheckMessagesOp;

    enum class ConnectionStatus
    {
        Disconnected,
        Connected,
        Asleep
    };

    typedef void (BasicClient<TClientOpts>::*FinaliseFunc)(MqttsnAsyncOpStatus);

    using TopicIdTypeVal = mqttsn::field::FlagsMembersCommon::TopicIdTypeVal;
    using ReturnCodeVal = mqttsn::field::ReturnCodeVal;

//    struct NoOrigDataViewProtOpts : TProtOpts
//    {
//        static const bool HasOrigDataView = false;
//    };

    class ProtOpts : public mqttsn::options::ClientDefaultOptions
    {
        using Base = mqttsn::options::ClientDefaultOptions;

    public:

        struct field : public Base::field
        {
            using ClientId = comms::option::app::OrigDataView;
            using Data = comms::option::app::OrigDataView;
            using GwAdd = comms::option::app::OrigDataView;
            using TopicName = comms::option::app::OrigDataView;
            using WillMsg = comms::option::app::OrigDataView;
            using WillTopic = comms::option::app::OrigDataView;
        };

        struct frame : public Base::frame
        {
            struct FrameLayers : public Base::frame::FrameLayers
            {
                using Id = comms::option::app::InPlaceAllocation;
            }; // struct FrameLayers

        }; // struct frame
    };

    class StorageOptions : public mqttsn::options::ClientDefaultOptions
    {
        using Base = mqttsn::options::ClientDefaultOptions;
    public:
        struct field : public Base::field
        {
            using ClientId = details::ClientIdStorageOptT<TClientOpts>;
            using Data = details::DataStorageOptT<TClientOpts>;
            using TopicName = details::TopicNameStorageOptT<TClientOpts>;
        };


    };

public:
    typedef typename Message::Field FieldBase;
    typedef typename mqttsn::field::GwId<ProtOpts>::ValueType GwIdValueType;
    typedef typename mqttsn::field::TopicName<StorageOptions>::ValueType TopicNameType;
    typedef typename mqttsn::field::Data<StorageOptions>::ValueType DataType;
    typedef typename mqttsn::field::TopicId<StorageOptions>::ValueType TopicIdType;
    typedef typename mqttsn::field::ClientId<StorageOptions>::ValueType ClientIdType;

    struct GwInfo
    {
        GwInfo() = default;

        Timestamp m_timestamp = 0;
        GwIdValueType m_id = 0;
        //GwAddValueType m_addr;
        unsigned m_duration = 0;
    };

    typedef details::GwInfoStorageTypeT<GwInfo, TClientOpts> GwInfoStorage;

    typedef mqttsn::message::Advertise<Message, ProtOpts> AdvertiseMsg;
    typedef mqttsn::message::Searchgw<Message, ProtOpts> SearchgwMsg;
    typedef mqttsn::message::Gwinfo<Message, ProtOpts> GwinfoMsg;
    typedef mqttsn::message::Connect<Message, ProtOpts> ConnectMsg;
    typedef mqttsn::message::Connack<Message, ProtOpts> ConnackMsg;
    typedef mqttsn::message::Willtopicreq<Message, ProtOpts> WilltopicreqMsg;
    typedef mqttsn::message::Willtopic<Message, ProtOpts> WilltopicMsg;
    typedef mqttsn::message::Willmsgreq<Message, ProtOpts> WillmsgreqMsg;
    typedef mqttsn::message::Willmsg<Message, ProtOpts> WillmsgMsg;
    typedef mqttsn::message::Register<Message, ProtOpts> RegisterMsg;
    typedef mqttsn::message::Regack<Message, ProtOpts> RegackMsg;
    typedef mqttsn::message::Publish<Message, ProtOpts> PublishMsg;
    typedef mqttsn::message::Puback<Message, ProtOpts> PubackMsg;
    typedef mqttsn::message::Pubrec<Message, ProtOpts> PubrecMsg;
    typedef mqttsn::message::Pubrel<Message, ProtOpts> PubrelMsg;
    typedef mqttsn::message::Pubcomp<Message, ProtOpts> PubcompMsg;
    typedef mqttsn::message::Subscribe<Message, ProtOpts> SubscribeMsg;
    typedef mqttsn::message::Suback<Message, ProtOpts> SubackMsg;
    typedef mqttsn::message::Unsubscribe<Message, ProtOpts> UnsubscribeMsg;
    typedef mqttsn::message::Unsuback<Message, ProtOpts> UnsubackMsg;
    typedef mqttsn::message::Pingreq<Message, ProtOpts> PingreqMsg;
    typedef mqttsn::message::Pingresp<Message, ProtOpts> PingrespMsg;
    typedef mqttsn::message::Disconnect<Message, ProtOpts> DisconnectMsg;
    typedef mqttsn::message::Willtopicupd<Message, ProtOpts> WilltopicupdMsg;
    typedef mqttsn::message::Willtopicresp<Message, ProtOpts> WilltopicrespMsg;
    typedef mqttsn::message::Willmsgupd<Message, ProtOpts> WillmsgupdMsg;
    typedef mqttsn::message::Willmsgresp<Message, ProtOpts> WillmsgrespMsg;

    BasicClient() = default;
    ~BasicClient() noexcept = default;

    typedef typename Message::ReadIterator ReadIterator;

    const GwInfoStorage& gwInfos() const
    {
        return m_gwInfos;
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

    void setGwDisconnectReportCallback(MqttsnGwDisconnectReportFn cb, void* data)
    {
        m_gwDisconnectReportFn = cb;
        m_gwDisconnectReportData = data;
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

    void sendSearchGw()
    {
        sendGwSearchReq();
    }

    void discardGw(std::uint8_t gwId)
    {
        auto guard = apiCall();
        auto iter =
            std::find_if(
                m_gwInfos.begin(), m_gwInfos.end(),
                [gwId](typename GwInfoStorage::const_reference elem) -> bool
                {
                    return elem.m_id == gwId;
                });

        if (iter == m_gwInfos.end()) {
            return;
        }

        m_gwInfos.erase(iter);
        reportGwStatus(gwId, MqttsnGwStatus_Discarded);
    }

    void discardAllGw()
    {
        auto guard = apiCall();

        if (m_gwStatusReportFn == nullptr) {
            m_gwInfos.clear();
            return;
        }

        typedef details::GwInfoStorageTypeT<std::uint8_t, TClientOpts> GwIdStorage;
        GwIdStorage ids;
        ids.reserve(m_gwInfos.size());
        std::transform(
            m_gwInfos.begin(), m_gwInfos.end(), std::back_inserter(ids),
            [](typename GwInfoStorage::const_reference elem) -> std::uint8_t
            {
                return elem.m_id;
            });

        m_gwInfos.clear();
        for (auto gwId : ids) {
            reportGwStatus(gwId, MqttsnGwStatus_Discarded);
        }
    }

    MqttsnErrorCode start()
    {
        if (m_running) {
            return MqttsnErrorCode_AlreadyStarted;
        }

        if ((m_nextTickProgramFn == nullptr) ||
            (m_cancelNextTickWaitFn == nullptr) ||
            (m_sendOutputDataFn == nullptr) ||
            (m_msgReportFn == nullptr)) {
            return MqttsnErrorCode_BadParam;
        }

        m_running = true;

        m_gwInfos.clear();
        m_regInfos.clear();
        m_nextTimeoutTimestamp = 0;
        m_lastGwSearchTimestamp = 0;
        m_lastRecvMsgTimestamp = 0;
        m_lastSentMsgTimestamp = 0;
        m_lastPingTimestamp = 0;

        m_pingCount = 0;
        m_connectionStatus = ConnectionStatus::Disconnected;

        m_currOp = Op::None;
        m_tickDelay = 0U;

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

    void tick()
    {
        if (!m_running) {
            return;
        }

        COMMS_ASSERT(m_callStackCount == 0U);
        m_timestamp += m_tickDelay;
        m_tickDelay = 0U;

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

            if (es == comms::ErrorStatus::ProtocolError) {
                ++iter;
                continue;
            }

            if (es == comms::ErrorStatus::Success) {
                COMMS_ASSERT(msg);
                m_lastRecvMsgTimestamp = m_timestamp;
                msg->dispatch(*this);
            }

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

        COMMS_ASSERT(m_running);
        auto guard = apiCall();

        auto fn = getFinaliseFunc();
        COMMS_ASSERT(fn != nullptr);
        (this->*(fn))(MqttsnAsyncOpStatus_Aborted);
        return true;
    }

    MqttsnErrorCode connect(
        const char* clientId,
        unsigned short keepAlivePeriod,
        bool cleanSession,
        const MqttsnWillInfo* willInfo,
        MqttsnAsyncOpCompleteReportFn callback,
        void* data)
    {
        if (!m_running) {
            return MqttsnErrorCode_NotStarted;
        }

        if (m_connectionStatus != ConnectionStatus::Disconnected) {
            return MqttsnErrorCode_AlreadyConnected;
        }

        if (m_currOp != Op::None) {
            return MqttsnErrorCode_Busy;
        }

        auto guard = apiCall();

        m_currOp = Op::Connect;

        auto* connectOp = newAsyncOp<ConnectOp>(callback, data);

        if (willInfo != nullptr) {
            connectOp->m_willInfo = *willInfo;
            connectOp->m_hasWill = true;
        }
        connectOp->m_clientId = clientId;
        connectOp->m_keepAlivePeriod = keepAlivePeriod;
        connectOp->m_cleanSession = cleanSession;

        bool result = doConnect();
        static_cast<void>(result);
        COMMS_ASSERT(result);
        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode reconnect(
        MqttsnAsyncOpCompleteReportFn callback,
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

        m_currOp = Op::Connect;
        auto* op = newAsyncOp<ConnectOp>(callback, data);
        op->m_clientId = m_clientId.c_str();
        op->m_keepAlivePeriod = static_cast<decltype(op->m_keepAlivePeriod)>(m_keepAlivePeriod / 1000);
        op->m_hasWill = false;
        op->m_cleanSession = false;

        bool result = doConnect();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode disconnect(
        MqttsnAsyncOpCompleteReportFn callback,
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

        auto guard = apiCall();
        m_currOp = Op::Disconnect;
        auto* op = newAsyncOp<DisconnectOp>(callback, data);
        static_cast<void>(op);

        bool result = doDisconnect();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode publish(
        MqttsnTopicId topicId,
        const std::uint8_t* msg,
        std::size_t msgLen,
        MqttsnQoS qos,
        bool retain,
        MqttsnAsyncOpCompleteReportFn callback,
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
            auto* pubOp = newAsyncOp<PublishIdOp>(callback, data);
            pubOp->m_topicId = topicId;
            pubOp->m_msg = msg;
            pubOp->m_msgLen = msgLen;
            pubOp->m_qos = qos;
            pubOp->m_retain = retain;

            bool result = doPublishId();
            static_cast<void>(result);
            COMMS_ASSERT(result);
        }
        else {
            sendPublish(
                topicId,
                allocMsgId(),
                msg,
                msgLen,
                TopicIdTypeVal::PredefinedTopicId,
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
        MqttsnAsyncOpCompleteReportFn callback,
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
        pubOp->m_shortName = isShortTopicName(topic);
        if (pubOp->m_shortName) {
            pubOp->m_topicId = shortTopicToTopicId(topic);
        }

        bool result = doPublish();
        static_cast<void>(result);
        COMMS_ASSERT(result);

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
        COMMS_ASSERT(result);

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
            (topic == nullptr) ||
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
        op->m_usingShortTopicName = isShortTopicName(topic);
        if (op->m_usingShortTopicName) {
            op->m_topicId = shortTopicToTopicId(topic);
        }

        bool result = doSubscribe();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode unsubscribe(
        MqttsnTopicId topicId,
        MqttsnAsyncOpCompleteReportFn callback,
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
        auto* op = newAsyncOp<UnsubscribeIdOp>(callback, data);
        op->m_topicId = topicId;
        op->m_msgId = allocMsgId();

        bool result = doUnsubscribeId();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode unsubscribe(
        const char* topic,
        MqttsnAsyncOpCompleteReportFn callback,
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
        auto* op = newAsyncOp<UnsubscribeOp>(callback, data);
        op->m_topic = topic;
        op->m_msgId = allocMsgId();
        op->m_usingShortTopicName = isShortTopicName(topic);
        if (op->m_usingShortTopicName) {
            op->m_topicId = shortTopicToTopicId(topic);
        }

        bool result = doUnsubscribe();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode willUpdate(
        const MqttsnWillInfo* willInfo,
        MqttsnAsyncOpCompleteReportFn callback,
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

        m_currOp = Op::Connect;
        auto* op = newAsyncOp<ConnectOp>(callback, data);
        op->m_clientId = m_clientId.c_str();
        op->m_keepAlivePeriod = static_cast<decltype(op->m_keepAlivePeriod)>(m_keepAlivePeriod / 1000);
        if (willInfo != nullptr) {
            op->m_willInfo = *willInfo;
        }
        op->m_hasWill = true;
        op->m_cleanSession = false;

        bool result = doConnect();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode willTopicUpdate(
        const char* topic,
        MqttsnQoS qos,
        bool retain,
        MqttsnAsyncOpCompleteReportFn callback,
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
        auto* op = newAsyncOp<WillTopicUpdateOp>(callback, data);
        op->m_topic = topic;
        op->m_qos = qos;
        op->m_retain = retain;

        bool result = doWillTopicUpdate();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode willMsgUpdate(
        const unsigned char* msg,
        unsigned msgLen,
        MqttsnAsyncOpCompleteReportFn callback,
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
        auto* op = newAsyncOp<WillMsgUpdateOp>(callback, data);
        op->m_msg = msg;
        op->m_msgLen = msgLen;

        bool result = doWillMsgUpdate();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode sleep(
        std::uint16_t duration,
        MqttsnAsyncOpCompleteReportFn callback,
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
        auto* op = newAsyncOp<SleepOp>(callback, data);
        op->m_duration = duration;

        bool result = doSleep();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    MqttsnErrorCode checkMessages(
        MqttsnAsyncOpCompleteReportFn callback,
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

        if (callback == nullptr) {
            return MqttsnErrorCode_BadParam;
        }

        auto guard = apiCall();

        m_currOp = Op::CheckMessages;
        auto* op = newAsyncOp<CheckMessagesOp>(callback, data);
        static_cast<void>(op);

        bool result = doCheckMessages();
        static_cast<void>(result);
        COMMS_ASSERT(result);

        return MqttsnErrorCode_Success;
    }

    void handle(AdvertiseMsg& msg)
    {
        auto durationVal =  msg.field_duration().value() * 3000U;

        auto iter = findGwInfo(msg.field_gwId().value());
        if (iter != m_gwInfos.end()) {
            iter->m_timestamp = m_timestamp;
            iter->m_duration = durationVal;
            return;
        }

        if (!addNewGw(msg.field_gwId().value(), durationVal)) {
            return;
        }

        reportGwStatus(msg.field_gwId().value(), MqttsnGwStatus_Available);
    }

    void handle(GwinfoMsg& msg)
    {
        //auto& addrField = std::get<GwinfoMsg::FieldIdx_gwAdd>(fields);
        auto iter = findGwInfo(msg.field_gwId().value());
        if (iter != m_gwInfos.end()) {
            iter->m_timestamp = m_timestamp;
//            if (!addrField.value().empty()) {
//                iter->m_addr = addrField.value();
//            }
            return;
        }

        if (!addNewGw(msg.field_gwId().value(), std::numeric_limits<std::uint16_t>::max() * 1000)) {
            return;
        }

        COMMS_ASSERT(!m_gwInfos.empty());
//        if (!addrField.value().empty()) {
//            iter->m_addr = addrField.value();
//        }

        reportGwStatus(msg.field_gwId().value(), MqttsnGwStatus_Available);
    }

    void handle(ConnackMsg& msg)
    {
        if (m_currOp != Op::Connect) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
        bool willReported = (op->m_willTopicSent && op->m_willMsgSent);
        if (op->m_hasWill && (!willReported)) {
            return;
        }

        auto returnCode = msg.field_returnCode().value();

        if (returnCode == ReturnCodeVal::Accepted) {
            m_connectionStatus = ConnectionStatus::Connected;
        }

        do {
            if (op->m_clientId == nullptr) {
                m_clientId.clear();
                break;
            }

            if (op->m_clientId == m_clientId.c_str()) {
                break;
            }

            m_clientId = op->m_clientId;
        } while (false);

        m_keepAlivePeriod = op->m_keepAlivePeriod * 1000U;
        finaliseConnectOp(retCodeToStatus(returnCode));
    }

    void handle(WilltopicreqMsg& msg)
    {
        static_cast<void>(msg);
        if (m_currOp != Op::Connect) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
        bool emptyTopic =
            (op->m_willInfo.topic == nullptr) || (op->m_willInfo.topic[0] == '\0');

        op->m_lastMsgTimestamp = m_timestamp;
        op->m_willTopicSent = true;
        if (emptyTopic) {
            op->m_willMsgSent = true;
        }

        sendWilltopic(op->m_willInfo.topic, op->m_willInfo.qos, op->m_willInfo.retain);
    }

    void handle(WillmsgreqMsg& msg)
    {
        static_cast<void>(msg);
        if (m_currOp != Op::Connect) {
            return;
        }

        auto* op = opPtr<ConnectOp>();
        if (!op->m_willTopicSent) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        WillmsgMsg outMsg;

        if (op->m_willInfo.msg != nullptr) {
            auto& dataStorage = outMsg.field_willMsg().value();
            using DataStorage = typename std::decay<decltype(dataStorage)>::type;
            dataStorage = DataStorage(op->m_willInfo.msg, op->m_willInfo.msgLen);
        }

        op->m_willMsgSent = true;
        sendMessage(outMsg);
    }

    void handle(RegisterMsg& msg)
    {
        auto& topic = msg.field_topicName().value();
        updateRegInfo(topic.data(), topic.size(), msg.field_topicId().value(), true);

        RegackMsg ackMsg;
        ackMsg.field_topicId().value() = msg.field_topicId().value();
        ackMsg.field_msgId().value() = msg.field_msgId().value();
        ackMsg.field_returnCode().value() = ReturnCodeVal::Accepted;
        sendMessage(ackMsg);
    }

    void handle(RegackMsg& msg)
    {
        if (m_currOp != Op::Publish) {
            return;
        }

        auto* op = opPtr<PublishOp>();

        if (!op->m_didRegistration) {
            return;
        }

        if (msg.field_msgId().value() != op->m_msgId) {
            return;
        }

        auto retCodeValue = msg.field_returnCode().value();

        if (retCodeValue != ReturnCodeVal::Accepted) {
            finalisePublishOp(retCodeToStatus(retCodeValue));
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        op->m_registered = true;
        op->m_topicId = msg.field_topicId().value();
        op->m_attempt = 0;

        updateRegInfo(op->m_topic, std::strlen(op->m_topic), op->m_topicId);
        bool result = doPublish();
        static_cast<void>(result);
        COMMS_ASSERT(result);
    }

    void handle(PublishMsg& msg)
    {
        auto reportMsgFunc =
            [this, &msg](const char* topicName)
            {
                auto msgInfo = MqttsnMessageInfo();

                msgInfo.topic = topicName;
                if (topicName == nullptr) {
                    msgInfo.topicId = msg.field_topicId().value();
                }

                msgInfo.msg = &(*msg.field_data().value().begin());
                msgInfo.msgLen = msg.field_data().value().size();
                msgInfo.qos = details::translateQosValue(msg.field_flags().field_qos().value());
                msgInfo.retain = msg.field_flags().field_mid().getBitValue_Retain();

                COMMS_ASSERT(m_msgReportFn != nullptr);
                m_msgReportFn(m_msgReportData, &msgInfo);
            };

        auto iter = m_regInfos.end();
        if (msg.field_flags().field_topicIdType().value() == TopicIdTypeVal::Normal) {
            iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [&msg](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == msg.field_topicId().value());
                });

            if (iter == m_regInfos.end()) {
                sendPuback(msg.field_topicId().value(), msg.field_msgId().value(), ReturnCodeVal::InvalidTopicId);
            }
        }

        const char* topicName = nullptr;
        if (iter != m_regInfos.end()) {
            topicName = iter->m_topic.c_str();
        }

        char shortTopicName[3] = {0};
        bool usingShortTopic =
            msg.field_flags().field_topicIdType().value() == TopicIdTypeVal::ShortTopicName;
        if (usingShortTopic) {
            COMMS_ASSERT(iter == m_regInfos.end());
            topicIdToShortTopic(msg.field_topicId().value(), &shortTopicName[0]);
            topicName = &shortTopicName[0];
        }

        if ((msg.field_flags().field_qos().value() < mqttsn::field::QosVal::AtLeastOnceDelivery) ||
            (mqttsn::field::QosVal::ExactlyOnceDelivery < msg.field_flags().field_qos().value())) {

            if ((topicName == nullptr) &&
                (msg.field_flags().field_topicIdType().value() != TopicIdTypeVal::PredefinedTopicId)) {
                return;
            }

            reportMsgFunc(topicName);
            return;
        }

        if ((topicName == nullptr) &&
            (msg.field_flags().field_topicIdType().value() != TopicIdTypeVal::PredefinedTopicId)) {
            m_lastInMsg = LastInMsgInfo();
            return;
        }

        if (msg.field_flags().field_qos().value() == mqttsn::field::QosVal::AtLeastOnceDelivery) {
            sendPuback(msg.field_topicId().value(), msg.field_msgId().value(), ReturnCodeVal::Accepted);
            reportMsgFunc(topicName);
            return;
        }

        COMMS_ASSERT(msg.field_flags().field_qos().value() == mqttsn::field::QosVal::ExactlyOnceDelivery);

        bool newMessage =
            ((!msg.field_flags().field_high().getBitValue_Dup()) ||
             (msg.field_topicId().value() != m_lastInMsg.m_topicId) ||
             (msg.field_msgId().value() != m_lastInMsg.m_msgId) ||
             (m_lastInMsg.m_reported) ||
             (usingShortTopic != m_lastInMsg.m_usingShortTopicName));

        if (newMessage) {
            m_lastInMsg = LastInMsgInfo();

            m_lastInMsg.m_topicId = msg.field_topicId().value();
            m_lastInMsg.m_msgId = msg.field_msgId().value();
            m_lastInMsg.m_retain = msg.field_flags().field_mid().getBitValue_Retain();
            m_lastInMsg.m_usingShortTopicName = usingShortTopic;
            if (usingShortTopic) {
                std::copy(std::begin(shortTopicName), std::end(shortTopicName), std::begin(m_lastInMsg.m_shortTopic));
            }
        }

        auto& msgData = msg.field_data().value();
        m_lastInMsg.m_msgData.assign(msgData.begin(), msgData.end());

        PubrecMsg recMsg;
        recMsg.field_msgId().value() = msg.field_msgId().value();
        sendMessage(recMsg);
    }

    void handle(PubackMsg& msg)
    {
        auto retCodeValue = msg.field_returnCode().value();

        if (retCodeValue == ReturnCodeVal::InvalidTopicId) {

            auto iter = std::find_if(
                m_regInfos.begin(), m_regInfos.end(),
                [&msg](typename RegInfosList::const_reference elem) -> bool
                {
                    return elem.m_allocated && (elem.m_topicId == msg.field_topicId().value());
                });

            if (iter != m_regInfos.end()) {
                dropRegInfo(iter);
            }
        }

        if ((m_currOp != Op::Publish) && (m_currOp != Op::PublishId)) {
            return;
        }

        auto* op = opPtr<PublishOpBase>();


        if ((msg.field_topicId().value() != op->m_topicId) ||
            (msg.field_msgId().value() != op->m_msgId)) {
            return;
        }

        if ((op->m_qos == MqttsnQoS_ExactlyOnceDelivery) &&
            (retCodeValue == ReturnCodeVal::Accepted)) {
            // PUBREC is expected instead
            return;
        }

        do {
            if ((op->m_qos < MqttsnQoS_AtLeastOnceDelivery) ||
                (retCodeValue != ReturnCodeVal::InvalidTopicId) ||
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

        finalisePublishOp(retCodeToStatus(retCodeValue));
    }

    void handle(PubrecMsg& msg)
    {
        if ((m_currOp != Op::Publish) && (m_currOp != Op::PublishId)) {
            return;
        }

        auto* op = opPtr<PublishOpBase>();

        if (msg.field_msgId().value() != op->m_msgId) {
            return;
        }

        op->m_lastMsgTimestamp = m_timestamp;
        op->m_ackReceived = true;
        sendPubrel(op->m_msgId);
    }

    void handle(PubrelMsg& msg)
    {
        if (m_lastInMsg.m_msgId != msg.field_msgId().value()) {
            m_lastInMsg = LastInMsgInfo();
            return;
        }

        PubcompMsg compMsg;
        compMsg.field_msgId().value() = msg.field_msgId().value();
        sendMessage(compMsg);

        if (!m_lastInMsg.m_reported) {
            auto msgInfo = MqttsnMessageInfo();

            if (m_lastInMsg.m_usingShortTopicName) {
                msgInfo.topic = &m_lastInMsg.m_shortTopic[0];
            }
            else {
                auto iter = std::find_if(
                    m_regInfos.begin(), m_regInfos.end(),
                    [this](typename RegInfosList::const_reference elem) -> bool
                    {
                        return elem.m_allocated && (elem.m_topicId == m_lastInMsg.m_topicId);
                    });

                if (iter != m_regInfos.end()) {
                    msgInfo.topic = iter->m_topic.c_str();
                }

                msgInfo.topicId = m_lastInMsg.m_topicId;
            }

            msgInfo.msg = &(*m_lastInMsg.m_msgData.begin());
            msgInfo.msgLen = m_lastInMsg.m_msgData.size();
            msgInfo.qos = MqttsnQoS_ExactlyOnceDelivery;
            msgInfo.retain = m_lastInMsg.m_retain;

            m_lastInMsg.m_reported = true;

            COMMS_ASSERT(m_msgReportFn != nullptr);
            m_msgReportFn(m_msgReportData, &msgInfo);
        }
    }

    void handle(PubcompMsg& msg)
    {
        if ((m_currOp != Op::Publish) && (m_currOp != Op::PublishId)) {
            return;
        }

        auto* op = opPtr<PublishOpBase>();

        if ((msg.field_msgId().value() != op->m_msgId) ||
            (!op->m_ackReceived)) {
            return;
        }

        finalisePublishOp(MqttsnAsyncOpStatus_Successful);
    }

    void handle(SubackMsg& msg)
    {
        if ((m_currOp != Op::Subscribe) && (m_currOp != Op::SubscribeId)) {
            return;
        }

        // NOTE: Without "volatile" keyword below g++-9 in Release mode
        // optimizes away assignments to some internal fields, like qos
        // before call to finaliseSubscribeOp(), which results
        // in incorrect values reported in the callback.
        volatile auto* op = opPtr<SubscribeOpBase>();

        if (msg.field_msgId().value() != op->m_msgId) {
            return;
        }

        auto retCodeValue = msg.field_returnCode().value();
        if ((retCodeValue == ReturnCodeVal::InvalidTopicId) &&
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
            COMMS_ASSERT(result);
            return;
        }

        auto qosValue = details::translateQosValue(msg.field_flags().field_qos().value());
        if (retCodeValue != ReturnCodeVal::Accepted) {
            op->m_qos = qosValue;
            finaliseSubscribeOp(retCodeToStatus(retCodeValue));
            return;
        }

        if ((m_currOp == Op::SubscribeId) &&
            (opPtr<SubscribeIdOp>()->m_topicId != msg.field_topicId().value())) {
            if (!doSubscribeId()) {
                finaliseSubscribeOp(MqttsnAsyncOpStatus_InvalidId);
            }
            return;
        }

        if ((m_currOp == Op::Subscribe) && (msg.field_topicId().value() != 0U)) {
            auto* topicStr = opPtr<SubscribeOp>()->m_topic;
            updateRegInfo(topicStr, std::strlen(topicStr), msg.field_topicId().value(), true);
        }

        op->m_qos = qosValue;
        finaliseSubscribeOp(MqttsnAsyncOpStatus_Successful);
    }

    void handle(UnsubackMsg& msg)
    {
        if ((m_currOp != Op::Unsubscribe) && (m_currOp != Op::UnsubscribeId)) {
            return;
        }

        auto* op = opPtr<UnsubscribeOpBase>();

        if (msg.field_msgId().value() != op->m_msgId) {
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

    void handle(PingreqMsg& msg)
    {
        static_cast<void>(msg);
        PingrespMsg outMsg;
        sendMessage(outMsg);
    }

    void handle(PingrespMsg& msg)
    {
        static_cast<void>(msg);
        bool pinging = (0U < m_pingCount);
        m_pingCount = 0U;

        if (pinging || (m_currOp != Op::CheckMessages)) {
            return;
        }

        // checking messages in asleep mode
        COMMS_ASSERT(m_connectionStatus == ConnectionStatus::Asleep);
        finaliseCheckMessagesOp(MqttsnAsyncOpStatus_Successful);
    }

    void handle(DisconnectMsg& msg)
    {
        static_cast<void>(msg);

        if (m_currOp == Op::Disconnect) {
            finaliseDisconnectOp(MqttsnAsyncOpStatus_Successful);
            return;
        }

        if (m_currOp == Op::Sleep) {
            m_connectionStatus = ConnectionStatus::Asleep;
            finaliseSleepOp(MqttsnAsyncOpStatus_Successful);
            return;
        }

        if (m_currOp == Op::None) {
            reportGwDisconnected();
            return;
        }

        cancel();
        reportGwDisconnected();
    }

    void handle(WilltopicrespMsg& msg)
    {
        if (m_currOp != Op::WillTopicUpdate) {
            return;
        }

        finaliseWillTopicUpdateOp(retCodeToStatus(msg.field_returnCode().value()));
    }

    void handle(WillmsgrespMsg& msg)
    {
        if (m_currOp != Op::WillMsgUpdate) {
            return;
        }

        finaliseWillMsgUpdateOp(retCodeToStatus(msg.field_returnCode().value()));
    }

    void handle(Message& msg)
    {
        static_cast<void>(msg);
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
        WillTopicUpdateOp,
        WillMsgUpdateOp,
        SleepOp,
        CheckMessagesOp
    >::Type OpStorageType;

    using InputMessages = mqttsn::input::ClientInputMessages<Message, ProtOpts>;
    typedef mqttsn::frame::Frame<Message, InputMessages, ProtOpts> ProtStack;
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
        char m_shortTopic[3] = {0};
        bool m_retain = false;
        bool m_reported = false;
        bool m_usingShortTopicName = false;
    };

    void updateRegInfo(const char* topic, std::size_t topicLen, TopicIdType topicId, bool locked = false)
    {
        auto iter = std::find_if(
            m_regInfos.begin(), m_regInfos.end(),
            [topic, topicLen](typename RegInfosList::const_reference elem) -> bool
            {
                return
                    elem.m_allocated &&
                    elem.m_topic.size() == topicLen &&
                    std::equal(elem.m_topic.begin(), elem.m_topic.end(), topic);
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
            iter->m_topic.assign(topic, topic + topicLen);
            iter->m_topicId = topicId;
            iter->m_allocated = true;
            iter->m_locked = locked;
            return;
        }

        RegInfo info;
        info.m_timestamp = m_timestamp;
        info.m_topic.assign(topic, topicLen);
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

        COMMS_ASSERT(iter != m_regInfos.end());
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

    template <typename TOp>
    TOp* newAsyncOp(MqttsnAsyncOpCompleteReportFn cb, void* cbData)
    {
        static_assert(std::is_base_of<AsyncOpBase, TOp>::value, "Invalid base");

        auto op = newOp<TOp>();
        op->m_cb = cb;
        op->m_cbData = cbData;
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
        if (!isTimerActive()) {
            return false;
        }

        COMMS_ASSERT(m_cancelNextTickWaitFn != nullptr);
        m_timestamp += m_cancelNextTickWaitFn(m_cancelNextTickWaitData);
        m_tickDelay = 0U;
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

        COMMS_ASSERT(iter != m_gwInfos.end());
        auto finalTimestamp = iter->m_timestamp + iter->m_duration;
        if (finalTimestamp < m_timestamp) {
            COMMS_ASSERT(!"Gateways are not cleaned up properly");
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

        auto pingTimestamp = m_lastPingTimestamp + m_retryPeriod;
        if (m_pingCount == 0) {
            pingTimestamp =
                std::min(m_lastSentMsgTimestamp, m_lastRecvMsgTimestamp) + m_keepAlivePeriod;
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

        COMMS_ASSERT(m_nextTickProgramFn != nullptr);
        m_nextTickProgramFn(m_nextTickProgramData, delay);
        m_nextTimeoutTimestamp = m_timestamp + delay;
        m_tickDelay = delay;
    }

    void checkAvailableGateways()
    {
        auto checkMustRemoveFunc =
            [this](typename GwInfoStorage::const_reference elem) -> bool
            {
                COMMS_ASSERT(elem.m_duration != 0U);
                return ((elem.m_timestamp + elem.m_duration) <= m_timestamp);
            };

        typedef details::GwInfoStorageTypeT<std::uint8_t, TClientOpts> GwIdStorage;
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
            COMMS_ASSERT(m_lastSentMsgTimestamp != 0);
            COMMS_ASSERT(m_lastRecvMsgTimestamp != 0);

            bool needsToSendPing =
                ((m_lastSentMsgTimestamp + m_keepAlivePeriod) <= m_timestamp) ||
                ((m_lastRecvMsgTimestamp + m_keepAlivePeriod) <= m_timestamp);

            if (needsToSendPing) {
                sendPing();
            }

            return;
        }

        COMMS_ASSERT(0U < m_lastPingTimestamp);
        if (m_timestamp < (m_lastPingTimestamp + m_retryPeriod)) {
            return;
        }

        if (m_retryCount <= m_pingCount) {
            reportGwDisconnected();
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

        typedef bool (BasicClient<TClientOpts>::*DoOpFunc)();
        static const DoOpFunc OpTimeoutFuncMap[] =
        {
            &BasicClient::doConnect,
            &BasicClient::doDisconnect,
            &BasicClient::doPublishId,
            &BasicClient::doPublish,
            &BasicClient::doSubscribeId,
            &BasicClient::doSubscribe,
            &BasicClient::doUnsubscribeId,
            &BasicClient::doUnsubscribe,
            &BasicClient::doWillTopicUpdate,
            &BasicClient::doWillMsgUpdate,
            &BasicClient::doSleep,
            &BasicClient::doCheckMessages
        };
        static const std::size_t OpTimeoutFuncMapSize =
                            std::extent<decltype(OpTimeoutFuncMap)>::value;

        static_assert(OpTimeoutFuncMapSize == (static_cast<std::size_t>(Op::NumOfValues) - 1U),
            "Map above is incorrect");

        COMMS_ASSERT((static_cast<unsigned>(m_currOp) - 1) < OpTimeoutFuncMapSize);
        auto fn = OpTimeoutFuncMap[static_cast<unsigned>(m_currOp) - 1];

        COMMS_ASSERT(fn != nullptr);
        if ((this->*(fn))()) {
            op->m_lastMsgTimestamp = m_timestamp;
            return;
        }

        auto finaliseFn = getFinaliseFunc();
        COMMS_ASSERT(finaliseFn != nullptr);
        (this->*finaliseFn)(MqttsnAsyncOpStatus_NoResponse);
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
        msg.field_radius().value() = m_broadcastRadius;
        sendMessage(msg, true);
        m_lastGwSearchTimestamp = m_timestamp;
    }

    void sendMessage(const Message& msg, bool broadcast = false)
    {
        if (m_sendOutputDataFn == nullptr) {
            COMMS_ASSERT(!"Unexpected send");
            return;
        }

        m_writeBuf.resize(std::max(m_writeBuf.size(), m_stack.length(msg)));
        COMMS_ASSERT(!m_writeBuf.empty());
        auto writeIter = comms::writeIteratorFor<Message>(&m_writeBuf[0]);
        auto es = m_stack.write(msg, writeIter, m_writeBuf.size());
        COMMS_ASSERT(es == comms::ErrorStatus::Success);
        if (es != comms::ErrorStatus::Success) {
            // Buffer is too small
            return;
        }

        auto writtenBytes = static_cast<std::size_t>(
            std::distance(comms::writeIteratorFor<Message>(&m_writeBuf[0]), writeIter));

        m_lastSentMsgTimestamp = m_timestamp;
        m_sendOutputDataFn(m_sendOutputDataData, &m_writeBuf[0], writtenBytes, broadcast);
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
        COMMS_ASSERT (m_currOp == Op::Connect);

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
        COMMS_ASSERT (m_currOp == Op::Disconnect);

        auto* op = opPtr<DisconnectOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        DisconnectMsg msg;
        msg.field_duration().setMissing();
        sendMessage(msg);
        return true;
    }

    bool doPublishId()
    {
        COMMS_ASSERT (m_currOp == Op::PublishId);

        auto* op = opPtr<PublishIdOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        bool firstAttempt = (op->m_attempt == 0U);
        ++op->m_attempt;

        if (firstAttempt && (MqttsnQoS_AtLeastOnceDelivery <= op->m_qos)) {
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
            TopicIdTypeVal::PredefinedTopicId,
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
        COMMS_ASSERT (m_currOp == Op::Publish);

        auto* op = opPtr<PublishOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        bool firstAttempt = (op->m_attempt == 0U);

        ++op->m_attempt;

        do {
            if (op->m_registered || op->m_shortName) {
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

        op->m_msgId = 0U;
        if (firstAttempt && (MqttsnQoS_AtLeastOnceDelivery <= op->m_qos)) {
            op->m_msgId = allocMsgId();
        }

        COMMS_ASSERT((op->m_registered) || (op->m_shortName));

        if ((!firstAttempt) &&
            (op->m_ackReceived) &&
            (MqttsnQoS_ExactlyOnceDelivery <= op->m_qos)) {
            sendPubrel(op->m_msgId);
            return true;
        }

        auto topicIdType = TopicIdTypeVal::Normal;
        if (op->m_shortName) {
            topicIdType = TopicIdTypeVal::ShortTopicName;
        }

        sendPublish(
            op->m_topicId,
            op->m_msgId,
            op->m_msg,
            op->m_msgLen,
            topicIdType,
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
        COMMS_ASSERT (m_currOp == Op::SubscribeId);

        auto* op = opPtr<SubscribeIdOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        bool firstAttempt = (op->m_attempt == 0U);
        ++op->m_attempt;

        SubscribeMsg msg;
        msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::PredefinedTopicId;
        msg.field_flags().field_qos().value() = details::translateQosValue(op->m_qos);
        msg.field_flags().field_high().setBitValue_Dup(!firstAttempt);
        msg.field_msgId().value() = op->m_msgId;
        msg.field_topicId().field().value() = op->m_topicId;
        msg.doRefresh();
        COMMS_ASSERT(msg.field_topicId().doesExist());
        sendMessage(msg);
        return true;
    }

    bool doSubscribe()
    {
        COMMS_ASSERT (m_currOp == Op::Subscribe);

        auto* op = opPtr<SubscribeOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        bool firstAttempt = (op->m_attempt == 0U);
        ++op->m_attempt;
\
        COMMS_ASSERT((op->m_topicId == 0) || (op->m_usingShortTopicName));

        SubscribeMsg msg;
        if (op->m_topicId == 0U) {
            msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::Normal;
            msg.field_topicName().field().value() = op->m_topic;
        }
        else {
            msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::ShortTopicName;
            msg.field_topicId().field().value() = op->m_topicId;
        }

        msg.field_flags().field_qos().value() = details::translateQosValue(op->m_qos);
        msg.field_flags().field_high().setBitValue_Dup(!firstAttempt);
        msg.field_msgId().value() = op->m_msgId;
        msg.doRefresh();
        COMMS_ASSERT((op->m_topicId != 0U) || (msg.field_topicName().doesExist()));
        COMMS_ASSERT((op->m_topicId != 0U) || (msg.field_topicId().isMissing()));
        COMMS_ASSERT((op->m_topicId == 0U) || (msg.field_topicName().isMissing()));
        COMMS_ASSERT((op->m_topicId == 0U) || (msg.field_topicId().doesExist()));

        sendMessage(msg);
        return true;
    }

    bool doUnsubscribeId()
    {
        COMMS_ASSERT (m_currOp == Op::UnsubscribeId);

        auto* op = opPtr<UnsubscribeIdOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        UnsubscribeMsg msg;
        msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::PredefinedTopicId;
        msg.field_msgId().value() = op->m_msgId;
        msg.field_topicId().field().value() = op->m_topicId;
        msg.doRefresh();
        COMMS_ASSERT(msg.field_topicId().doesExist());
        sendMessage(msg);
        return true;
    }

    bool doUnsubscribe()
    {
        COMMS_ASSERT (m_currOp == Op::Unsubscribe);

        auto* op = opPtr<UnsubscribeOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;
        COMMS_ASSERT((op->m_topicId == 0) || (op->m_usingShortTopicName));

        UnsubscribeMsg msg;

        if (op->m_topicId == 0U) {
            msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::Normal;
            msg.field_topicName().field().value() = op->m_topic;
        }
        else {
            msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::ShortTopicName;
            msg.field_topicId().field().value() = op->m_topicId;
        }

        msg.field_msgId().value() = op->m_msgId;
        msg.doRefresh();
        COMMS_ASSERT((op->m_topicId != 0U) || (msg.field_topicName().doesExist()));
        COMMS_ASSERT((op->m_topicId != 0U) || (msg.field_topicId().isMissing()));
        COMMS_ASSERT((op->m_topicId == 0U) || (msg.field_topicName().isMissing()));
        COMMS_ASSERT((op->m_topicId == 0U) || (msg.field_topicId().doesExist()));

        sendMessage(msg);
        return true;
    }

    bool doWillTopicUpdate()
    {
        COMMS_ASSERT (m_currOp == Op::WillTopicUpdate);

        auto* op = opPtr<WillTopicUpdateOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;
        WilltopicupdMsg msg;
        bool topicEmpty = ((op->m_topic == nullptr) || (op->m_topic[0] == '\0'));
        if (!topicEmpty) {
            msg.field_flags().field().field_qos().value() = details::translateQosValue(op->m_qos);
            msg.field_flags().field().field_mid().setBitValue_Retain(op->m_retain);
            msg.field_willTopic().value() = op->m_topic;
        }

        msg.doRefresh();
        COMMS_ASSERT(topicEmpty || msg.field_flags().doesExist());
        COMMS_ASSERT((!topicEmpty) || msg.field_flags().isMissing());

        sendMessage(msg);
        return true;
    }

    bool doWillMsgUpdate()
    {
        COMMS_ASSERT (m_currOp == Op::WillMsgUpdate);

        auto* op = opPtr<WillMsgUpdateOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        WillmsgupdMsg msg;
        auto* msgBodyBeg = op->m_msg;
        if (msgBodyBeg != nullptr) {
            auto& dataStorage = msg.field_willMsg().value();
            using DataStorage = typename std::decay<decltype(dataStorage)>::type;
            dataStorage = DataStorage(msgBodyBeg, op->m_msgLen);
        }
        sendMessage(msg);
        return true;
    }

    bool doSleep()
    {
        COMMS_ASSERT (m_currOp == Op::Sleep);

        auto* op = opPtr<SleepOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        DisconnectMsg msg;
        msg.field_duration().setExists();
        msg.field_duration().field().value() = op->m_duration;
        sendMessage(msg);
        return true;
    }

    bool doCheckMessages()
    {
        COMMS_ASSERT (m_currOp == Op::CheckMessages);

        auto* op = opPtr<CheckMessagesOp>();
        if (m_retryCount <= op->m_attempt) {
            return false;
        }

        ++op->m_attempt;

        PingreqMsg msg;
        auto& clientIdStorage = msg.field_clientId().value();
        using ClientIdStorage = typename std::decay<decltype(clientIdStorage)>::type;
        clientIdStorage = ClientIdStorage(m_clientId.c_str(), m_clientId.size());
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
        msg.field_flags().field_mid().setBitValue_CleanSession(cleanSession);
        msg.field_flags().field_mid().setBitValue_Will(hasWill);
        msg.field_duration().value() = keepAlivePeriod;
        if (clientId != nullptr) {
            msg.field_clientId().value() = clientId;
        }
        sendMessage(msg);
    }

    void sendWilltopic(
        const char* topic,
        MqttsnQoS qos,
        bool retain)
    {
        WilltopicMsg msg;
        bool topicEmpty = ((topic == nullptr) || (topic[0] == '\0'));
        if (!topicEmpty) {
            msg.field_flags().field().field_qos().value() = details::translateQosValue(qos);
            msg.field_flags().field().field_mid().setBitValue_Retain(retain);
            msg.field_willTopic().value() = topic;
        }

        msg.doRefresh();
        COMMS_ASSERT(topicEmpty || msg.field_flags().doesExist());
        COMMS_ASSERT((!topicEmpty) || msg.field_flags().isMissing());
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
        TopicIdTypeVal topicIdType,
        mqttsn::field::QosVal qos,
        bool retain,
        bool duplicate)
    {
        PublishMsg pubMsg;
        pubMsg.field_flags().field_topicIdType().value() = topicIdType;
        pubMsg.field_flags().field_mid().setBitValue_Retain(retain);
        pubMsg.field_flags().field_qos().value() = qos;
        pubMsg.field_flags().field_high().setBitValue_Dup(duplicate);
        pubMsg.field_topicId().value() = topicId;
        pubMsg.field_msgId().value() = msgId;

        auto& dataStorage = pubMsg.field_data().value();
        using DataStorage = typename std::decay<decltype(dataStorage)>::type;
        dataStorage = DataStorage(msg, msgLen);

        sendMessage(pubMsg);
    }

    void sendPuback(
        MqttsnTopicId topicId,
        std::uint16_t msgId,
        ReturnCodeVal retCode)
    {
        PubackMsg msg;
        msg.field_topicId().value() = topicId;
        msg.field_msgId().value() = msgId;
        msg.field_returnCode().value() = retCode;
        sendMessage(msg);
    }

    void sendRegister(
        std::uint16_t msgId,
        const char* topic)
    {
        RegisterMsg msg;
        msg.field_msgId().value() = msgId;
        msg.field_topicName().value() = topic;
        sendMessage(msg);
    }

    void sendPubrel(std::uint16_t msgId)
    {
        PubrelMsg msg;
        msg.field_msgId().value() = msgId;
        sendMessage(msg);
    }

    void reportGwDisconnected()
    {
        m_connectionStatus = ConnectionStatus::Disconnected;

        if (m_gwDisconnectReportFn != nullptr) {
            m_gwDisconnectReportFn(m_gwDisconnectReportData);
        }
    }

    std::uint16_t allocMsgId()
    {
        ++m_msgId;
        return m_msgId;
    }

    template <typename TOp, Op TCurr>
    void finaliseAsyncOp(MqttsnAsyncOpStatus status)
    {
        COMMS_ASSERT(m_currOp == TCurr);

        static_assert(std::is_base_of<AsyncOpBase, TOp>::value, "Invalid base");
        auto* op = opPtr<AsyncOpBase>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        finaliseOp<TOp>();
        COMMS_ASSERT(m_currOp == Op::None);
        COMMS_ASSERT(cb != nullptr);
        cb(cbData, status);
    }

    template <typename TOp1, Op TCurr1, typename TOp2, Op TCurr2>
    void finaliseAsyncDoubleOp(MqttsnAsyncOpStatus status)
    {
        COMMS_ASSERT((m_currOp == TCurr1) || (m_currOp == TCurr2));

        static_assert(std::is_base_of<AsyncOpBase, TOp1>::value, "Invalid base");
        static_assert(std::is_base_of<AsyncOpBase, TOp2>::value, "Invalid base");
        auto* op = opPtr<AsyncOpBase>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        if (m_currOp == TCurr1) {
            finaliseOp<TOp1>();
        }
        else {
            finaliseOp<TOp2>();
        }

        COMMS_ASSERT(m_currOp == Op::None);
        COMMS_ASSERT(cb != nullptr);
        cb(cbData, status);
    }

    void finaliseConnectOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncOp<ConnectOp, Op::Connect>(status);
    }

    void finaliseDisconnectOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncOp<DisconnectOp, Op::Disconnect>(status);
    }

    void finalisePublishOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncDoubleOp<PublishOp, Op::Publish, PublishIdOp, Op::PublishId>(status);
    }

    void finaliseSubscribeOp(MqttsnAsyncOpStatus status)
    {
        COMMS_ASSERT((m_currOp == Op::Subscribe) || (m_currOp == Op::SubscribeId));
        auto* op = opPtr<SubscribeOpBase>();
        auto* cb = op->m_cb;
        auto* cbData = op->m_cbData;

        if (m_currOp == Op::Subscribe) {
            finaliseOp<SubscribeOp>();
        }
        else {
            finaliseOp<SubscribeIdOp>();
        }
        COMMS_ASSERT(m_currOp == Op::None);
        COMMS_ASSERT(cb != nullptr);
        cb(cbData, status, op->m_qos);
    }

    void finaliseUnsubscribeOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncDoubleOp<UnsubscribeOp, Op::Unsubscribe, UnsubscribeIdOp, Op::UnsubscribeId>(status);
    }

    void finaliseWillTopicUpdateOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncOp<WillTopicUpdateOp, Op::WillTopicUpdate>(status);
    }

    void finaliseWillMsgUpdateOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncOp<WillMsgUpdateOp, Op::WillMsgUpdate>(status);
    }

    void finaliseSleepOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncOp<SleepOp, Op::Sleep>(status);
    }

    void finaliseCheckMessagesOp(MqttsnAsyncOpStatus status)
    {
        finaliseAsyncOp<CheckMessagesOp, Op::CheckMessages>(status);
    }

    FinaliseFunc getFinaliseFunc() const
    {
        static const FinaliseFunc Map[] =
        {
            &BasicClient::finaliseConnectOp,
            &BasicClient::finaliseDisconnectOp,
            &BasicClient::finalisePublishOp,
            &BasicClient::finalisePublishOp,
            &BasicClient::finaliseSubscribeOp,
            &BasicClient::finaliseSubscribeOp,
            &BasicClient::finaliseUnsubscribeOp,
            &BasicClient::finaliseUnsubscribeOp,
            &BasicClient::finaliseWillTopicUpdateOp,
            &BasicClient::finaliseWillMsgUpdateOp,
            &BasicClient::finaliseSleepOp,
            &BasicClient::finaliseCheckMessagesOp,
        };
        static const std::size_t MapSize =
                            std::extent<decltype(Map)>::value;

        static_assert(MapSize == (static_cast<std::size_t>(Op::NumOfValues) - 1U),
            "Map above is incorrect");

        auto opIdx = static_cast<unsigned>(m_currOp) - 1;
        COMMS_ASSERT(opIdx < MapSize);
        return Map[opIdx];
    }

    MqttsnAsyncOpStatus retCodeToStatus(ReturnCodeVal val)
    {
        static const MqttsnAsyncOpStatus Map[] = {
            /* ReturnCodeVal_Accepted */ MqttsnAsyncOpStatus_Successful,
            /* ReturnCodeVal_Congestion */ MqttsnAsyncOpStatus_Congestion,
            /* ReturnCodeVal_InvalidTopicId */ MqttsnAsyncOpStatus_InvalidId,
            /* ReturnCodeVal_NotSupported */ MqttsnAsyncOpStatus_NotSupported
        };

        static const std::size_t MapSize = std::extent<decltype(Map)>::value;

        static_assert(MapSize == (unsigned)ReturnCodeVal::ValuesLimit,
            "Map is incorrect");

        MqttsnAsyncOpStatus status = MqttsnAsyncOpStatus_NotSupported;
        if (static_cast<unsigned>(val) < MapSize) {
            status = Map[static_cast<unsigned>(val)];
        }

        return status;
    }

    void apiCallExit()
    {
        COMMS_ASSERT(0U < m_callStackCount);
        --m_callStackCount;
        if (m_callStackCount == 0U) {
            programNextTimeout();
        }
    }

#ifdef _MSC_VER
    // VC compiler
    auto apiCall()
#else
    auto apiCall() -> decltype(comms::util::makeScopeGuard(std::bind(&BasicClient<TClientOpts>::apiCallExit, this)))
#endif
    {
        ++m_callStackCount;
        if (m_callStackCount == 1U) {
            updateTimestamp();
        }

        return
            comms::util::makeScopeGuard(
                std::bind(
                    &BasicClient<TClientOpts>::apiCallExit,
                    this));
    }

    bool isTimerActive() const
    {
        return (m_tickDelay != 0U);
    }

    static bool isShortTopicName(const char* topic)
    {
        COMMS_ASSERT(topic != nullptr);
        auto checkCharFunc =
            [](char ch)
            {
                return
                    (ch != '\0') &&
                    (ch != '+') &&
                    (ch != '#');
            };

        if ((!checkCharFunc(topic[0])) ||
            (!checkCharFunc(topic[1]))) {
            return false;
        }

        return topic[2] == '\0';
    }

    static MqttsnTopicId shortTopicToTopicId(const char* topic)
    {
        COMMS_ASSERT(topic[0] != '\0');
        COMMS_ASSERT(topic[1] != '\0');

        return
            static_cast<MqttsnTopicId>(
                (static_cast<MqttsnTopicId>(topic[0]) << 8) | static_cast<std::uint8_t>(topic[1]));
    }

    static void topicIdToShortTopic(MqttsnTopicId topicId, char* topicOut)
    {
        topicOut[0] = static_cast<char>((topicId >> 8) & 0xff);
        topicOut[1] = static_cast<char>(topicId & 0xff);
        topicOut[2] = '\0';
    }


    ProtStack m_stack;
    GwInfoStorage m_gwInfos;
    Timestamp m_timestamp = DefaultStartTimestamp;
    Timestamp m_nextTimeoutTimestamp = 0;
    Timestamp m_lastGwSearchTimestamp = 0;
    Timestamp m_lastRecvMsgTimestamp = 0;
    Timestamp m_lastSentMsgTimestamp = 0;
    Timestamp m_lastPingTimestamp = 0;
    ClientIdType m_clientId;

    unsigned m_callStackCount = 0U;
    unsigned m_pingCount = 0;
    unsigned m_retryPeriod = DefaultRetryPeriod;
    unsigned m_retryCount = DefaultRetryCount;
    unsigned m_keepAlivePeriod = 0;
    ConnectionStatus m_connectionStatus = ConnectionStatus::Disconnected;
    std::uint16_t m_msgId = 0;
    std::uint8_t m_broadcastRadius = DefaultBroadcastRadius;

    unsigned m_tickDelay = 0U;
    bool m_running = false;
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

    MqttsnGwDisconnectReportFn m_gwDisconnectReportFn = nullptr;
    void* m_gwDisconnectReportData = nullptr;

    MqttsnMessageReportFn m_msgReportFn = nullptr;
    void* m_msgReportData = nullptr;

    WriteBufStorage m_writeBuf;

    static const unsigned DefaultAdvertisePeriod = 30 * 60 * 1000;
    static const unsigned DefaultRetryPeriod = 15 * 1000;
    static const unsigned DefaultRetryCount = 3;
    static const std::uint8_t DefaultBroadcastRadius = 0U;

    static const unsigned NoTimeout = std::numeric_limits<unsigned>::max();
    static const Timestamp DefaultStartTimestamp = 100;
};

}  // namespace client

}  // namespace mqttsn


