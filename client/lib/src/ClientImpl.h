//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ClientState.h"
#include "ConfigState.h"
#include "ExtConfig.h"
#include "ObjAllocator.h"
#include "ObjListType.h"
#include "ProtocolDefs.h"
#include "ReuseState.h"
#include "SessionState.h"
#include "TimerMgr.h"

#include "op/ConnectOp.h"
#include "op/DisconnectOp.h"
#include "op/KeepAliveOp.h"
#include "op/Op.h"
// #include "op/RecvOp.h"
#include "op/SearchOp.h"
#include "op/SendOp.h"
#include "op/SubscribeOp.h"
#include "op/UnsubscribeOp.h"
#include "op/WillOp.h"

#include "cc_mqttsn_client/common.h"

namespace cc_mqttsn_client
{

class ClientImpl final : public ProtMsgHandler
{
    using Base = ProtMsgHandler;

public:
    class ApiEnterGuard
    {
    public:
        ApiEnterGuard(ClientImpl& client) : m_client(client)
        {
            m_client.doApiEnter();
        }

        ~ApiEnterGuard() noexcept
        {
            m_client.doApiExit();
        }

    private:
        ClientImpl& m_client;
    };

    ClientImpl();
    ~ClientImpl();

    ApiEnterGuard apiEnter()
    {
        return ApiEnterGuard(*this);
    }

    // -------------------- API Calls -----------------------------
    void tick(unsigned ms);
    void processData(const std::uint8_t* iter, unsigned len);

    op::SearchOp* searchPrepare(CC_MqttsnErrorCode* ec);
    op::ConnectOp* connectPrepare(CC_MqttsnErrorCode* ec);
    op::DisconnectOp* disconnectPrepare(CC_MqttsnErrorCode* ec);
    op::SubscribeOp* subscribePrepare(CC_MqttsnErrorCode* ec);
    op::UnsubscribeOp* unsubscribePrepare(CC_MqttsnErrorCode* ec);
    op::SendOp* publishPrepare(CC_MqttsnErrorCode* ec);
    op::WillOp* willPrepare(CC_MqttsnErrorCode* ec);

    CC_MqttsnErrorCode setOutgoingRegTopicsLimit(std::size_t limit);
    std::size_t getOutgoingRegTopicsLimit() const;
    CC_MqttsnErrorCode setIncomingRegTopicsLimit(std::size_t limit);
    std::size_t getIncomingRegTopicsLimit() const;    

    void setNextTickProgramCallback(CC_MqttsnNextTickProgramCb cb, void* data)
    {
        if (cb != nullptr) {
            m_nextTickProgramCb = cb;
            m_nextTickProgramData = data;
        }
    }

    void setCancelNextTickWaitCallback(CC_MqttsnCancelNextTickWaitCb cb, void* data)
    {
        if (cb != nullptr) {
            m_cancelNextTickWaitCb = cb;
            m_cancelNextTickWaitData = data;
        }
    }

    void setSendOutputDataCallback(CC_MqttsnSendOutputDataCb cb, void* data)
    {
        if (cb != nullptr) {
            m_sendOutputDataCb = cb;
            m_sendOutputDataData = data;
        }
    }

    void setGatewayStatusReportCallback(CC_MqttsnGwStatusReportCb cb, void* data)
    {
        if (cb != nullptr) {
            m_gatewayStatusReportCb = cb;
            m_gatewayStatusReportData = data;
        }
    }

    void setGatewayDisconnectedReportCallback(CC_MqttsnGwDisconnectedReportCb cb, void* data)
    {
        if (cb != nullptr) {
            m_gatewayDisconnectedReportCb = cb;
            m_gatewayDisconnectedReportData = data;
        }
    }    

    void setMessageReceivedCallback(CC_MqttsnMessageReportCb cb, void* data)
    {
        if (cb != nullptr) {
            m_messageReceivedReportCb = cb;
            m_messageReceivedReportData = data;            
        }
    }

    void setErrorLogCallback(CC_MqttsnErrorLogCb cb, void* data)
    {
        m_errorLogCb = cb;
        m_errorLogData = data;
    }

    void setGwinfoDelayReqCb(CC_MqttsnGwinfoDelayRequestCb cb, void* data)
    {
        m_gwinfoDelayReqCb = cb;
        m_gwinfoDelayReqData = data;
    }

    // -------------------- Message Handling -----------------------------

    using Base::handle;
#if CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY
    virtual void handle(AdvertiseMsg& msg) override;
    virtual void handle(SearchgwMsg& msg) override;
    virtual void handle(GwinfoMsg& msg) override;
#endif // #if CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY        
//     virtual void handle(PublishMsg& msg) override;

    virtual void handle(RegisterMsg& msg) override;
    virtual void handle(PublishMsg& msg) override;
    virtual void handle(PubackMsg& msg) override;

#if CC_MQTTSN_CLIENT_MAX_QOS >= 2
    virtual void handle(PubrelMsg& msg) override;
#endif // #if CC_MQTTSN_CLIENT_MAX_QOS >= 2

    virtual void handle(PingreqMsg& msg) override;
    virtual void handle(DisconnectMsg& msg) override;
    virtual void handle(ProtMessage& msg) override;

    // -------------------- Ops Access API -----------------------------

    CC_MqttsnErrorCode sendMessage(const ProtMessage& msg, unsigned broadcastRadius = 0);
    void opComplete(const op::Op* op);
    void gatewayConnected();
    void gatewayDisconnected(
        CC_MqttsnGatewayDisconnectReason reason = CC_MqttsnGatewayDisconnectReason_ValuesLimit,  
        CC_MqttsnAsyncOpStatus status = CC_MqttsnAsyncOpStatus_GatewayDisconnected);
    // void reportMsgInfo(const CC_MqttsnMessageInfo& info);
    // bool hasPausedSendsBefore(const op::SendOp* sendOp) const;
    // bool hasHigherQosSendsBefore(const op::SendOp* sendOp, op::Op::Qos qos) const;
    void allowNextPrepare();
    void storeInRegTopic(const char* topic, CC_MqttsnTopicId topicId);
    bool removeInRegTopic(const char* topic, CC_MqttsnTopicId topicId);    

    TimerMgr& timerMgr()
    {
        return m_timerMgr;
    }

    ConfigState& configState()
    {
        return m_configState;
    }

    const ConfigState& configState() const
    {
        return m_configState;
    }

    ClientState& clientState()
    {
        return m_clientState;
    }    

    const ClientState& clientState() const
    {
        return m_clientState;
    }       

    SessionState& sessionState()
    {
        return m_sessionState;
    }

    const SessionState& sessionState() const
    {
        return m_sessionState;
    }    

    ReuseState& reuseState()
    {
        return m_reuseState;
    }    

    inline void errorLog(const char* msg)
    {
        if constexpr (Config::HasErrorLog) {
            errorLogInternal(msg);
        }
    }

    inline bool verifyPubTopic(const char* topic, bool outgoing)
    {
        if (Config::HasTopicFormatVerification) {
            return verifyPubTopicInternal(topic, outgoing);
        }
        else {
            return true;
        }
    }       

    // std::size_t recvsCount() const
    // {
    //     return m_recvOps.size();
    // }    
    
private:
#if CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY
    using SearchOpAlloc = ObjAllocator<op::SearchOp, ExtConfig::SearchOpsLimit>;
    using SearchOpsList = ObjListType<SearchOpAlloc::Ptr, ExtConfig::SearchOpsLimit>;
#endif // #if CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY

    using ConnectOpAlloc = ObjAllocator<op::ConnectOp, ExtConfig::ConnectOpsLimit>;
    using ConnectOpsList = ObjListType<ConnectOpAlloc::Ptr, ExtConfig::ConnectOpsLimit>;

    using KeepAliveOpAlloc = ObjAllocator<op::KeepAliveOp, ExtConfig::KeepAliveOpsLimit>;
    using KeepAliveOpsList = ObjListType<KeepAliveOpAlloc::Ptr, ExtConfig::KeepAliveOpsLimit>;

    using DisconnectOpAlloc = ObjAllocator<op::DisconnectOp, ExtConfig::DisconnectOpsLimit>;
    using DisconnectOpsList = ObjListType<DisconnectOpAlloc::Ptr, ExtConfig::DisconnectOpsLimit>;

    using SubscribeOpAlloc = ObjAllocator<op::SubscribeOp, ExtConfig::SubscribeOpsLimit>;
    using SubscribeOpsList = ObjListType<SubscribeOpAlloc::Ptr, ExtConfig::SubscribeOpsLimit>;

    using UnsubscribeOpAlloc = ObjAllocator<op::UnsubscribeOp, ExtConfig::UnsubscribeOpsLimit>;
    using UnsubscribeOpsList = ObjListType<UnsubscribeOpAlloc::Ptr, ExtConfig::UnsubscribeOpsLimit>;

    // using RecvOpAlloc = ObjAllocator<op::RecvOp, ExtConfig::RecvOpsLimit>;
    // using RecvOpsList = ObjListType<RecvOpAlloc::Ptr, ExtConfig::RecvOpsLimit>;

    using SendOpAlloc = ObjAllocator<op::SendOp, ExtConfig::SendOpsLimit>;
    using SendOpsList = ObjListType<SendOpAlloc::Ptr, ExtConfig::SendOpsLimit>;

#if CC_MQTTSN_CLIENT_HAS_WILL
    using WillOpAlloc = ObjAllocator<op::WillOp, ExtConfig::WillOpsLimit>;
    using WillOpsList = ObjListType<WillOpAlloc::Ptr, ExtConfig::WillOpsLimit>;
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL


    using OpPtrsList = ObjListType<op::Op*, ExtConfig::OpsLimit>;
    // using OpToDeletePtrsList = ObjListType<const op::Op*, ExtConfig::OpsLimit>;
    using OutputBuf = ObjListType<std::uint8_t, ExtConfig::MaxOutputPacketSize>;

    void doApiEnter();
    void doApiExit();
    void createKeepAliveOpIfNeeded();
    void terminateOps(CC_MqttsnAsyncOpStatus status);
    void cleanOps();
    void errorLogInternal(const char* msg);
    CC_MqttsnErrorCode initInternal();
    bool verifyPubTopicInternal(const char* topic, bool outgoing);
    // void resumeSendOpsSince(unsigned idx);
    // op::SendOp* findSendOp(std::uint16_t packetId);
    // bool isLegitSendAck(const op::SendOp* sendOp, bool pubcompAck = false) const;
    // void resendAllUntil(op::SendOp* sendOp);
    // bool processPublishAckMsg(ProtMessage& msg, std::uint16_t packetId, bool pubcompAck = false);

    void opComplete_Search(const op::Op* op);
    void opComplete_Connect(const op::Op* op);
    void opComplete_KeepAlive(const op::Op* op);
    void opComplete_Disconnect(const op::Op* op);
    void opComplete_Subscribe(const op::Op* op);
    void opComplete_Unsubscribe(const op::Op* op);
    // void opComplete_Recv(const op::Op* op);
    void opComplete_Send(const op::Op* op);
    void opComplete_Will(const op::Op* op);

    void finaliseSupUnsubOp();
    void monitorGatewayExpiry();
    void gwExpiryTimeout();
    void reportGwStatus(CC_MqttsnGwStatus status, const ClientState::GwInfo& info);
    void sendGwinfo();

    static void gwExpiryTimeoutCb(void* data);
    static void sendGwinfoCb(void* data);

    friend class ApiEnterGuard;

    CC_MqttsnNextTickProgramCb m_nextTickProgramCb = nullptr;
    void* m_nextTickProgramData = nullptr;

    CC_MqttsnCancelNextTickWaitCb m_cancelNextTickWaitCb = nullptr;
    void* m_cancelNextTickWaitData = nullptr;

    CC_MqttsnSendOutputDataCb m_sendOutputDataCb = nullptr;
    void* m_sendOutputDataData = nullptr;

    CC_MqttsnGwStatusReportCb m_gatewayStatusReportCb = nullptr;
    void* m_gatewayStatusReportData = nullptr;

    CC_MqttsnGwDisconnectedReportCb m_gatewayDisconnectedReportCb = nullptr;
    void* m_gatewayDisconnectedReportData = nullptr;

    CC_MqttsnMessageReportCb m_messageReceivedReportCb = nullptr;
    void* m_messageReceivedReportData = nullptr;      

    CC_MqttsnErrorLogCb m_errorLogCb = nullptr;
    void* m_errorLogData = nullptr;

    CC_MqttsnGwinfoDelayRequestCb m_gwinfoDelayReqCb = nullptr;
    void* m_gwinfoDelayReqData = nullptr;

    ConfigState m_configState;
    ClientState m_clientState;
    SessionState m_sessionState;
    ReuseState m_reuseState;

    TimerMgr m_timerMgr;
    TimerMgr::Timer m_gwDiscoveryTimer;  
    TimerMgr::Timer m_sendGwinfoTimer;  
    unsigned m_apiEnterCount = 0U;

    OutputBuf m_buf;

    ProtFrame m_frame;

#if CC_MQTTSN_CLIENT_HAS_GATEWAY_DISCOVERY
    SearchOpAlloc m_searchOpAlloc;
    SearchOpsList m_searchOps;    
#endif

    ConnectOpAlloc m_connectOpAlloc;
    ConnectOpsList m_connectOps;

    KeepAliveOpAlloc m_keepAliveOpsAlloc;
    KeepAliveOpsList m_keepAliveOps;

    DisconnectOpAlloc m_disconnectOpsAlloc;
    DisconnectOpsList m_disconnectOps;

    SubscribeOpAlloc m_subscribeOpsAlloc;
    SubscribeOpsList m_subscribeOps;

    UnsubscribeOpAlloc m_unsubscribeOpsAlloc;
    UnsubscribeOpsList m_unsubscribeOps;

    // RecvOpAlloc m_recvOpsAlloc;
    // RecvOpsList m_recvOps;

    SendOpAlloc m_sendOpsAlloc;
    SendOpsList m_sendOps;

#if CC_MQTTSN_CLIENT_HAS_WILL
    WillOpAlloc m_willOpAlloc;
    WillOpsList m_willOps;    
#endif // #if CC_MQTTSN_CLIENT_HAS_WILL    

    OpPtrsList m_ops;
    unsigned m_pendingGwinfoBroadcastRadius = 0U;
    bool m_opsDeleted = false;
    bool m_preparationLocked = false;
};

} // namespace cc_mqttsn_client
