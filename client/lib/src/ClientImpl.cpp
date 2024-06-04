//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ClientImpl.h"

#include "comms/cast.h"
#include "comms/Assert.h"
#include "comms/process.h"
#include "comms/units.h"
#include "comms/util/ScopeGuard.h"
#include "comms/util/assign.h"

#include <algorithm>
#include <type_traits>

namespace cc_mqttsn_client
{

namespace 
{

template <typename TList>
unsigned eraseFromList(const op::Op* op, TList& list)
{
    auto iter = 
        std::find_if(
            list.begin(), list.end(),
            [op](auto& opPtr)
            {
                return op == opPtr.get();
            });

    auto result = static_cast<unsigned>(std::distance(list.begin(), iter));

    COMMS_ASSERT(iter != list.end());
    if (iter != list.end()) {
        list.erase(iter);
    }

    return result;
}

void updateEc(CC_MqttsnErrorCode* ec, CC_MqttsnErrorCode val)
{
    if (ec != nullptr) {
        *ec = val;
    }
}

} // namespace 

ClientImpl::ClientImpl() : 
    m_gwDiscoveryTimer(m_timerMgr.allocTimer()),
    m_sendGwinfoTimer(m_timerMgr.allocTimer())
{
    // TODO: check validity of timer in during intialization
    static_cast<void>(m_searchOpAlloc);
}

ClientImpl::~ClientImpl()
{
    COMMS_ASSERT(m_apiEnterCount == 0U);
    // terminateOps(CC_MqttsnAsyncOpStatus_Aborted, TerminateMode_AbortSendRecvOps);
}


void ClientImpl::tick(unsigned ms)
{
    COMMS_ASSERT(m_apiEnterCount == 0U);
    ++m_apiEnterCount;
    m_clientState.m_timestamp += ms;
    m_timerMgr.tick(ms);
    doApiExit();
}

void ClientImpl::processData(const std::uint8_t* iter, unsigned len)
{
    auto guard = apiEnter();

    ProtFrame::MsgPtr msgPtr;
    auto es = comms::processSingleWithDispatch(iter, len, m_frame, msgPtr, *this);
    if (es != comms::ErrorStatus::Success) {
        errorLog("Failed to decode the received message");
        return;
    }
}

op::SearchOp* ClientImpl::searchPrepare(CC_MqttsnErrorCode* ec)
{
    if constexpr (Config::HasGatewayDiscovery) {
        op::SearchOp* op = nullptr;
        do {
            if (!m_clientState.m_initialized) {
                if (m_apiEnterCount > 0U) {
                    errorLog("Cannot prepare search from within callback");
                    updateEc(ec, CC_MqttsnErrorCode_RetryLater);
                    break;
                }

                auto initEc = initInternal();
                if (initEc != CC_MqttsnErrorCode_Success) {
                    updateEc(ec, initEc);
                    break;
                }
            }
                    
            if (!m_searchOps.empty()) {
                // Already allocated
                errorLog("Another search operation is in progress.");
                updateEc(ec, CC_MqttsnErrorCode_Busy);
                break;
            }

            if (m_ops.max_size() <= m_ops.size()) {
                errorLog("Cannot start search operation, retry in next event loop iteration.");
                updateEc(ec, CC_MqttsnErrorCode_RetryLater);
                break;
            }

            if (m_preparationLocked) {
                errorLog("Another operation is being prepared, cannot prepare \"search\" without \"send\" or \"cancel\" of the previous.");
                updateEc(ec, CC_MqttsnErrorCode_PreparationLocked);            
                break;
            }

            auto ptr = m_searchOpAlloc.alloc(*this);
            if (!ptr) {
                errorLog("Cannot allocate new search operation.");
                updateEc(ec, CC_MqttsnErrorCode_OutOfMemory);
                break;
            }

            m_preparationLocked = true;
            m_ops.push_back(ptr.get());
            m_searchOps.push_back(std::move(ptr));
            op = m_searchOps.back().get();
            updateEc(ec, CC_MqttsnErrorCode_Success);
        } while (false);

        return op;
    }
    else {
        errorLog("Gateway discovery support for excluded from compilation");
        updateEc(ec, CC_MqttsnErrorCode_NotSupported);
        return nullptr;
    }
}

op::ConnectOp* ClientImpl::connectPrepare(CC_MqttsnErrorCode* ec)
{
    op::ConnectOp* op = nullptr;
    do {
        if (!m_clientState.m_initialized) {
            if (m_apiEnterCount > 0U) {
                errorLog("Cannot prepare connect from within callback");
                updateEc(ec, CC_MqttsnErrorCode_RetryLater);
                break;
            }

            auto initEc = initInternal();
            if (initEc != CC_MqttsnErrorCode_Success) {
                updateEc(ec, initEc);
                break;
            }
        }
                
        if (!m_connectOps.empty()) {
            // Already allocated
            errorLog("Another connect operation is in progress.");
            updateEc(ec, CC_MqttsnErrorCode_Busy);
            break;
        }

        if (m_ops.max_size() <= m_ops.size()) {
            errorLog("Cannot start connect operation, retry in next event loop iteration.");
            updateEc(ec, CC_MqttsnErrorCode_RetryLater);
            break;
        }

        if (m_preparationLocked) {
            errorLog("Another operation is being prepared, cannot prepare \"connect\" without \"send\" or \"cancel\" of the previous.");
            updateEc(ec, CC_MqttsnErrorCode_PreparationLocked);            
            break;
        }

        auto ptr = m_connectOpAlloc.alloc(*this);
        if (!ptr) {
            errorLog("Cannot allocate new connect operation.");
            updateEc(ec, CC_MqttsnErrorCode_OutOfMemory);
            break;
        }

        m_preparationLocked = true;
        m_ops.push_back(ptr.get());
        m_connectOps.push_back(std::move(ptr));
        op = m_connectOps.back().get();
        updateEc(ec, CC_MqttsnErrorCode_Success);
    } while (false);

    return op;
}

// op::ConnectOp* ClientImpl::connectPrepare(CC_MqttsnErrorCode* ec)
// {
//     op::ConnectOp* connectOp = nullptr;
//     do {
//         m_clientState.m_networkDisconnected = false;

//         if (!m_clientState.m_initialized) {
//             if (m_apiEnterCount > 0U) {
//                 errorLog("Cannot prepare connect from within callback");
//                 updateEc(ec, CC_MqttsnErrorCode_RetryLater);
//                 break;
//             }

//             auto initEc = initInternal();
//             if (initEc != CC_MqttsnErrorCode_Success) {
//                 updateEc(ec, initEc);
//                 break;
//             }
//         }
                
//         if (!m_connectOps.empty()) {
//             // Already allocated
//             errorLog("Another connect operation is in progress.");
//             updateEc(ec, CC_MqttsnErrorCode_Busy);
//             break;
//         }

//         if (m_sessionState.m_disconnecting) {
//             errorLog("Session disconnection is in progress, cannot initiate connection.");
//             updateEc(ec, CC_MqttsnErrorCode_Disconnecting);
//             break;
//         }

//         if (m_sessionState.m_connected) {
//             errorLog("Client is already connected.");
//             updateEc(ec, CC_MqttsnErrorCode_AlreadyConnected);
//             break;
//         }        

//         if (m_ops.max_size() <= m_ops.size()) {
//             errorLog("Cannot start connect operation, retry in next event loop iteration.");
//             updateEc(ec, CC_MqttsnErrorCode_RetryLater);
//             break;
//         }

//         if (m_preparationLocked) {
//             errorLog("Another operation is being prepared, cannot prepare \"connect\" without \"send\" or \"cancel\" of the previous.");
//             updateEc(ec, CC_MqttsnErrorCode_PreparationLocked);            
//             break;
//         }

//         auto ptr = m_connectOpAlloc.alloc(*this);
//         if (!ptr) {
//             errorLog("Cannot allocate new connect operation.");
//             updateEc(ec, CC_MqttsnErrorCode_OutOfMemory);
//             break;
//         }

//         m_preparationLocked = true;
//         m_ops.push_back(ptr.get());
//         m_connectOps.push_back(std::move(ptr));
//         connectOp = m_connectOps.back().get();
//         updateEc(ec, CC_MqttsnErrorCode_Success);
//     } while (false);

//     return connectOp;
// }

// op::DisconnectOp* ClientImpl::disconnectPrepare(CC_MqttsnErrorCode* ec)
// {
//     op::DisconnectOp* disconnectOp = nullptr;
//     do {
//         if (!m_sessionState.m_connected) {
//             errorLog("Client must be connected to allow disconnect.");
//             updateEc(ec, CC_MqttsnErrorCode_NotConnected);
//             break;
//         }

//         if (!m_disconnectOps.empty()) {
//             errorLog("Another disconnect operation is in progress.");
//             updateEc(ec, CC_MqttsnErrorCode_Busy);
//             break;
//         }        

//         if (m_sessionState.m_disconnecting) {
//             errorLog("Session disconnection is in progress, cannot initiate disconnection.");
//             updateEc(ec, CC_MqttsnErrorCode_Disconnecting);
//             break;
//         }

//         if (m_clientState.m_networkDisconnected) {
//             errorLog("Network is disconnected.");
//             updateEc(ec, CC_MqttsnErrorCode_NetworkDisconnected);
//             break;            
//         }        

//         if (m_ops.max_size() <= m_ops.size()) {
//             errorLog("Cannot start disconnect operation, retry in next event loop iteration.");
//             updateEc(ec, CC_MqttsnErrorCode_RetryLater);
//             break;
//         }   

//         if (m_preparationLocked) {
//             errorLog("Another operation is being prepared, cannot prepare \"disconnect\" without \"send\" or \"cancel\" of the previous.");
//             updateEc(ec, CC_MqttsnErrorCode_PreparationLocked);            
//             break;
//         }            

//         auto ptr = m_disconnectOpsAlloc.alloc(*this);
//         if (!ptr) {
//             errorLog("Cannot allocate new disconnect operation.");
//             updateEc(ec, CC_MqttsnErrorCode_OutOfMemory);
//             break;
//         }

//         m_preparationLocked = true;
//         m_ops.push_back(ptr.get());
//         m_disconnectOps.push_back(std::move(ptr));
//         disconnectOp = m_disconnectOps.back().get();
//         updateEc(ec, CC_MqttsnErrorCode_Success);
//     } while (false);

//     return disconnectOp;
// }

// op::SubscribeOp* ClientImpl::subscribePrepare(CC_MqttsnErrorCode* ec)
// {
//     op::SubscribeOp* subOp = nullptr;
//     do {
//         if (!m_sessionState.m_connected) {
//             errorLog("Client must be connected to allow subscription.");
//             updateEc(ec, CC_MqttsnErrorCode_NotConnected);
//             break;
//         }

//         if (m_sessionState.m_disconnecting) {
//             errorLog("Session disconnection is in progress, cannot initiate subscription.");
//             updateEc(ec, CC_MqttsnErrorCode_Disconnecting);
//             break;
//         }

//         if (m_clientState.m_networkDisconnected) {
//             errorLog("Network is disconnected.");
//             updateEc(ec, CC_MqttsnErrorCode_NetworkDisconnected);
//             break;            
//         }        

//         if (m_ops.max_size() <= m_ops.size()) {
//             errorLog("Cannot start subscribe operation, retry in next event loop iteration.");
//             updateEc(ec, CC_MqttsnErrorCode_RetryLater);
//             break;
//         }  

//         if (m_preparationLocked) {
//             errorLog("Another operation is being prepared, cannot prepare \"subscribe\" without \"send\" or \"cancel\" of the previous.");
//             updateEc(ec, CC_MqttsnErrorCode_PreparationLocked);            
//             break;
//         }            

//         auto ptr = m_subscribeOpsAlloc.alloc(*this);
//         if (!ptr) {
//             errorLog("Cannot allocate new subscribe operation.");
//             updateEc(ec, CC_MqttsnErrorCode_OutOfMemory);
//             break;
//         }

//         m_preparationLocked = true;
//         m_ops.push_back(ptr.get());
//         m_subscribeOps.push_back(std::move(ptr));
//         subOp = m_subscribeOps.back().get();
//         updateEc(ec, CC_MqttsnErrorCode_Success);
//     } while (false);

//     return subOp;
// }

// op::UnsubscribeOp* ClientImpl::unsubscribePrepare(CC_MqttsnErrorCode* ec)
// {
//     op::UnsubscribeOp* unsubOp = nullptr;
//     do {
//         if (!m_sessionState.m_connected) {
//             errorLog("Client must be connected to allow unsubscription.");
//             updateEc(ec, CC_MqttsnErrorCode_NotConnected);
//             break;
//         }

//         if (m_sessionState.m_disconnecting) {
//             errorLog("Session disconnection is in progress, cannot initiate unsubscription.");
//             updateEc(ec, CC_MqttsnErrorCode_Disconnecting);
//             break;
//         }

//         if (m_clientState.m_networkDisconnected) {
//             errorLog("Network is disconnected.");
//             updateEc(ec, CC_MqttsnErrorCode_NetworkDisconnected);
//             break;            
//         }        

//         if (m_ops.max_size() <= m_ops.size()) {
//             errorLog("Cannot start subscribe operation, retry in next event loop iteration.");
//             updateEc(ec, CC_MqttsnErrorCode_RetryLater);
//             break;
//         }    

//         if (m_preparationLocked) {
//             errorLog("Another operation is being prepared, cannot prepare \"unsubscribe\" without \"send\" or \"cancel\" of the previous.");
//             updateEc(ec, CC_MqttsnErrorCode_PreparationLocked);            
//             break;
//         }             

//         auto ptr = m_unsubscribeOpsAlloc.alloc(*this);
//         if (!ptr) {
//             errorLog("Cannot allocate new unsubscribe operation.");
//             updateEc(ec, CC_MqttsnErrorCode_OutOfMemory);
//             break;
//         }

//         m_preparationLocked = true;
//         m_ops.push_back(ptr.get());
//         m_unsubscribeOps.push_back(std::move(ptr));
//         unsubOp = m_unsubscribeOps.back().get();
//         updateEc(ec, CC_MqttsnErrorCode_Success);
//     } while (false);

//     return unsubOp;
// }

// op::SendOp* ClientImpl::publishPrepare(CC_MqttsnErrorCode* ec)
// {
//     op::SendOp* sendOp = nullptr;
//     do {
//         if (!m_sessionState.m_connected) {
//             errorLog("Client must be connected to allow publish.");
//             updateEc(ec, CC_MqttsnErrorCode_NotConnected);
//             break;
//         }

//         if (m_sessionState.m_disconnecting) {
//             errorLog("Session disconnection is in progress, cannot initiate publish.");
//             updateEc(ec, CC_MqttsnErrorCode_Disconnecting);
//             break;
//         }

//         if (m_clientState.m_networkDisconnected) {
//             errorLog("Network is disconnected.");
//             updateEc(ec, CC_MqttsnErrorCode_NetworkDisconnected);
//             break;            
//         }        

//         if (m_ops.max_size() <= m_ops.size()) {
//             errorLog("Cannot start publish operation, retry in next event loop iteration.");
//             updateEc(ec, CC_MqttsnErrorCode_RetryLater);
//             break;
//         }        

//         auto ptr = m_sendOpsAlloc.alloc(*this);
//         if (!ptr) {
//             errorLog("Cannot allocate new publish operation.");
//             updateEc(ec, CC_MqttsnErrorCode_OutOfMemory);
//             break;
//         }

//         if (m_preparationLocked) {
//             errorLog("Another operation is being prepared, cannot prepare \"unsubscribe\" without \"send\" or \"cancel\" of the previous.");
//             updateEc(ec, CC_MqttsnErrorCode_PreparationLocked);            
//             break;
//         }          

//         m_preparationLocked = true;
//         m_ops.push_back(ptr.get());
//         m_sendOps.push_back(std::move(ptr));
//         sendOp = m_sendOps.back().get();
//         updateEc(ec, CC_MqttsnErrorCode_Success);
//     } while (false);

//     return sendOp;
// }

// CC_MqttsnErrorCode ClientImpl::setPublishOrdering(CC_MqttsnPublishOrdering ordering)
// {
//     if (CC_MqttsnPublishOrdering_ValuesLimit <= ordering) {
//         errorLog("Bad publish ordering value");
//         return CC_MqttsnErrorCode_BadParam;
//     }

//     m_configState.m_publishOrdering = ordering;
//     return CC_MqttsnErrorCode_Success;
// }

#if CC_MQTTSN_HAS_GATEWAY_DISCOVERY
void ClientImpl::handle(AdvertiseMsg& msg)
{
    static_assert(Config::HasGatewayDiscovery);
    
    m_gwDiscoveryTimer.cancel();

    CC_MqttsnGwStatus gwStatus = CC_MqttsnGwStatus_ValuesLimit; 
    const ClientState::GwInfo* gwInfo = nullptr;
    auto onExit =
        comms::util::makeScopeGuard( 
            [this, &gwStatus, &gwInfo, &msg]()
            {
                // When advertise arrives before GWINFO and the search is present, 
                // report search completion     
                for (auto& searchOp : m_searchOps) {
                    COMMS_ASSERT(searchOp);
                    searchOp->handle(msg);
                }
                                
                // Reporting the gateway status after 
                // dispatching to the search operation.
                                
                if ((gwStatus < CC_MqttsnGwStatus_ValuesLimit) &&
                    (gwInfo != nullptr)) {
                    reportGwStatus(gwStatus, *gwInfo);
                }

                monitorGatewayExpiry();
            });

    auto iter = 
        std::find_if(
            m_clientState.m_gwInfos.begin(), m_clientState.m_gwInfos.end(),
            [&msg](auto& info)
            {
                return msg.field_gwId().value() == info.m_gwId;
            });

    auto duration = comms::units::getMilliseconds<unsigned>(msg.field_duration());
    auto nextExpiryTimestamp = m_clientState.m_timestamp + duration + m_configState.m_retryPeriod;

    if (iter != m_clientState.m_gwInfos.end()) {
        iter->m_expiryTimestamp = nextExpiryTimestamp;
        iter->m_duration = duration;
        iter->m_allowedAdvLosses = m_configState.m_allowedAdvLosses;
        gwStatus = CC_MqttsnGwStatus_Alive;
        gwInfo = &(*iter);
        return; // Geport gateway status on exit
    }    

    if (m_clientState.m_gwInfos.max_size() <= m_clientState.m_gwInfos.size()) {
        // Ignore new gateways if they cannot be stored
        errorLog("Failed to store the new gateway information, due to insufficient storage");
        return;
    }

    m_clientState.m_gwInfos.resize(m_clientState.m_gwInfos.size() + 1U);
    auto& info = m_clientState.m_gwInfos.back();
    info.m_gwId = msg.field_gwId().value();
    info.m_expiryTimestamp = nextExpiryTimestamp;
    info.m_duration = duration;
    info.m_allowedAdvLosses = m_configState.m_allowedAdvLosses;

    gwStatus = CC_MqttsnGwStatus_AddedByGateway;
    gwInfo = &info;    
    // Geport gateway status on exit
}

void ClientImpl::handle(SearchgwMsg& msg)
{
    static_assert(Config::HasGatewayDiscovery);
    if (m_gwinfoDelayReqCb == nullptr) {
        // The application didn't provide a callback to inquire about the delay for resonditing to SEARCHGW
        return;
    }

    if (m_clientState.m_gwInfos.empty()) {
        // No known gateways
        return;
    }

    auto delay = m_gwinfoDelayReqCb(m_gwinfoDelayReqData);
    if (delay == 0U) {
        // The application rejected sending GWINFO on behalf of gateway
        return;
    }

    m_pendingGwinfoBroadcastRadius = msg.field_radius().value();
    m_sendGwinfoTimer.wait(delay, &ClientImpl::sendGwinfoCb, this);
}

void ClientImpl::handle(GwinfoMsg& msg)
{
    static_assert(Config::HasGatewayDiscovery);
    m_sendGwinfoTimer.cancel(); // Do not send GWINFO if pending

    CC_MqttsnGwStatus gwStatus = CC_MqttsnGwStatus_ValuesLimit; 
    const ClientState::GwInfo* gwInfo = nullptr;

    // Reporting the gateway status after 
    // dispatching to the search operation.
    auto reportGwStatusOnExit = 
        comms::util::makeScopeGuard(
            [this, &gwStatus, &gwInfo]()
            {
                if ((gwStatus < CC_MqttsnGwStatus_ValuesLimit) &&
                    (gwInfo != nullptr)) {
                    reportGwStatus(gwStatus, *gwInfo);
                }
            });

    // Dispatching to the SearchOp after processing and storing
    // the gateway information in the internal data structures.
    // It will allow updating of the gateway address by the application
    // from within the search op callback
    auto handleSearchOpOnExit = 
        comms::util::makeScopeGuard(
            [this, &msg]()
            {
                for (auto& op : m_searchOps) {
                    COMMS_ASSERT(op);
                    op->handle(msg);
                }
            });

    auto iter = 
        std::find_if(
            m_clientState.m_gwInfos.begin(), m_clientState.m_gwInfos.end(),
            [&msg](auto& info)
            {
                return msg.field_gwId().value() == info.m_gwId;
            });

    if (iter != m_clientState.m_gwInfos.end()) {
        auto& addr = msg.field_gwAdd().value();
        if (addr.empty()) {
            // GWINFO by the gateway itself
            iter->m_allowedAdvLosses = m_configState.m_allowedAdvLosses;
            gwStatus = CC_MqttsnGwStatus_Alive;
            gwInfo = &(*iter);
            return;
        }

        // GWINFO by another client

        if (iter->m_addr.max_size() < addr.size()) {
            errorLog("The gateway address reported by the client doesn't fit into the dedicated address storage, ignoring");
            return;
        }

        if ((addr.size() == iter->m_addr.size()) && 
            (std::equal(addr.begin(), addr.end(), iter->m_addr.begin()))) {
            // The address is already recorded.
            return;
        }

        iter->m_addr.assign(addr.begin(), addr.end());
        gwStatus = CC_MqttsnGwStatus_UpdatedByClient;
        gwInfo = &(*iter);
        return; // Report gateway status on exit
    }

    m_clientState.m_gwInfos.resize(m_clientState.m_gwInfos.size() + 1U);
    auto& info = m_clientState.m_gwInfos.back();
    info.m_expiryTimestamp = m_clientState.m_timestamp + m_configState.m_gwAdvTimeoutMs + m_configState.m_retryPeriod;
    info.m_duration = m_configState.m_gwAdvTimeoutMs;

    info.m_gwId = msg.field_gwId().value();
    info.m_allowedAdvLosses = m_configState.m_allowedAdvLosses;
    gwInfo = &info;

    auto& addr = msg.field_gwAdd().value();
    if (addr.empty()) {
        gwStatus = CC_MqttsnGwStatus_AddedByGateway;
        return; // Report gateway status on exit
    }    

    if (m_clientState.m_gwInfos.max_size() <= m_clientState.m_gwInfos.size()) {
        // Not enough space
        errorLog("Failed to store the new gateway information, due to insufficient storage");
        return;
    }

    info.m_addr.assign(addr.begin(), addr.end());
    monitorGatewayExpiry();

    gwStatus = CC_MqttsnGwStatus_AddedByClient;
    // Report geteway status on exit
}
#endif // #if CC_MQTTSN_HAS_GATEWAY_DISCOVERY        

// void ClientImpl::handle(PublishMsg& msg)
// {
//     if (m_sessionState.m_disconnecting) {
//         return;
//     }

//     for (auto& opPtr : m_keepAliveOps) {
//         msg.dispatch(*opPtr);
//     }       

//     do {
//         auto createRecvOp = 
//             [this, &msg]()
//             {
//                 auto ptr = m_recvOpsAlloc.alloc(*this);
//                 if (!ptr) {
//                     errorLog("Failed to allocate handling op for the incoming PUBLISH message, ignoring.");
//                     return; 
//                 }

//                 m_ops.push_back(ptr.get());
//                 m_recvOps.push_back(std::move(ptr));
//                 msg.dispatch(*m_recvOps.back());
//             };

//         using Qos = op::Op::Qos;
//         auto qos = msg.transportField_flags().field_qos().value();
//         if ((qos == Qos::AtMostOnceDelivery) || 
//             (qos == Qos::AtLeastOnceDelivery)) {
//             createRecvOp();
//             break;
//         }

//         if constexpr (Config::MaxQos >= 2) {
//             auto iter = 
//                 std::find_if(
//                     m_recvOps.begin(), m_recvOps.end(),
//                     [&msg](auto& opPtr)
//                     {
//                         return opPtr->packetId() == msg.field_packetId().field().value();
//                     });

//             if (iter == m_recvOps.end()) {
//                 createRecvOp();
//                 break;            
//             }

//             PubrecMsg pubrecMsg;
//             pubrecMsg.field_packetId().setValue(msg.field_packetId().field().value());

//             if (!msg.transportField_flags().field_dup().getBitValue_bit()) {
//                 errorLog("Non duplicate PUBLISH with packet ID in use");
//                 brokerDisconnected(CC_MqttsnBrokerDisconnectReason_ProtocolError);
//                 return;
//             }
//             else {
//                 // Duplicate detected, just re-confirming
//                 (*iter)->resetTimer();
//             }

//             sendMessage(pubrecMsg);
//             return;
//         }
//         else {
//             createRecvOp();
//             break;
//         }
//     } while (false);
// }

// #if CC_MQTTSN_CLIENT_MAX_QOS >= 1
// void ClientImpl::handle(PubackMsg& msg)
// {
//     static_assert(Config::MaxQos >= 1);
//     if (!processPublishAckMsg(msg, msg.field_packetId().value(), false)) {
//         errorLog("PUBACK with unknown packet id");
//     }    
// }
// #endif // #if CC_MQTTSN_CLIENT_MAX_QOS >= 1

// #if CC_MQTTSN_CLIENT_MAX_QOS >= 2
// void ClientImpl::handle(PubrecMsg& msg)
// {
//     static_assert(Config::MaxQos >= 2);
//     if (!processPublishAckMsg(msg, msg.field_packetId().value(), false)) {
//         errorLog("PUBREC with unknown packet id");
//     }       
// }

// void ClientImpl::handle(PubrelMsg& msg)
// {
//     static_assert(Config::MaxQos >= 2);
//     for (auto& opPtr : m_keepAliveOps) {
//         msg.dispatch(*opPtr);
//     }

//     auto iter = 
//         std::find_if(
//             m_recvOps.begin(), m_recvOps.end(),
//             [&msg](auto& opPtr)
//             {
//                 COMMS_ASSERT(opPtr);
//                 return opPtr->packetId() == msg.field_packetId().value();
//             });

//     if (iter == m_recvOps.end()) {
//         errorLog("PUBREL with unknown packet id");
//         return;
//     }

//     msg.dispatch(**iter);
// }

// void ClientImpl::handle(PubcompMsg& msg)
// {
//     static_assert(Config::MaxQos >= 2);
//     if (!processPublishAckMsg(msg, msg.field_packetId().value(), true)) {
//         errorLog("PUBCOMP with unknown packet id");
//     }    
// }

// #endif // #if CC_MQTTSN_CLIENT_MAX_QOS >= 2

void ClientImpl::handle(ProtMessage& msg)
{
    static_cast<void>(msg);
    // if (m_sessionState.m_disconnecting) {
    //     return;
    // }

    // During the dispatch to callbacks can be called and new ops issues,
    // the m_ops vector can be resized and iterators invalidated.
    // As the result, the iteration needs to be performed using indices 
    // instead of iterators.
    // Also do not dispatch the message to new ops.
    auto count = m_ops.size();
    for (auto idx = 0U; idx < count; ++idx) {
        auto* op = m_ops[idx];
        if (op == nullptr) {
            // ops can be deleted, but the pointer will be nullified
            // until last api guard.
            continue;
        }

        msg.dispatch(*op);

        // After message dispatching the whole session may be in terminating state
        // Don't continue iteration
        
        // if (m_sessionState.m_disconnecting) {
        //     break;
        // }    
    }
}

CC_MqttsnErrorCode ClientImpl::sendMessage(const ProtMessage& msg, unsigned broadcastRadius)
{
    auto len = m_frame.length(msg);

    if (m_buf.max_size() < len) {
        errorLog("Output buffer overflow.");
        return CC_MqttsnErrorCode_BufferOverflow;
    }

    m_buf.resize(len);
    auto writeIter = comms::writeIteratorFor<ProtMessage>(&m_buf[0]);
    auto es = m_frame.write(msg, writeIter, len);
    COMMS_ASSERT(es == comms::ErrorStatus::Success);
    if (es != comms::ErrorStatus::Success) {
        errorLog("Failed to serialize output message.");
        return CC_MqttsnErrorCode_InternalError;
    }

    COMMS_ASSERT(m_sendOutputDataCb != nullptr);
    m_sendOutputDataCb(m_sendOutputDataData, &m_buf[0], static_cast<unsigned>(len), broadcastRadius);

    // for (auto& opPtr : m_keepAliveOps) {
    //     opPtr->messageSent();
    // }

    return CC_MqttsnErrorCode_Success;
}

void ClientImpl::opComplete(const op::Op* op)
{
    auto iter = std::find(m_ops.begin(), m_ops.end(), op);
    COMMS_ASSERT(iter != m_ops.end());
    if (iter == m_ops.end()) {
        return;
    }

    *iter = nullptr;
    m_opsDeleted = true;

    using ExtraCompleteFunc = void (ClientImpl::*)(const op::Op*);
    static const ExtraCompleteFunc Map[] = {
        /* Type_Search */ &ClientImpl::opComplete_Search,
        /* Type_Connect */ &ClientImpl::opComplete_Connect,
        // /* Type_KeepAlive */ &ClientImpl::opComplete_KeepAlive,
        // /* Type_Disconnect */ &ClientImpl::opComplete_Disconnect,
        // /* Type_Subscribe */ &ClientImpl::opComplete_Subscribe,
        // /* Type_Unsubscribe */ &ClientImpl::opComplete_Unsubscribe,
        // /* Type_Recv */ &ClientImpl::opComplete_Recv,
        // /* Type_Send */ &ClientImpl::opComplete_Send,
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == op::Op::Type_NumOfValues);

    auto idx = static_cast<unsigned>(op->type());
    COMMS_ASSERT(idx < MapSize);
    if (MapSize <= idx) {
        return;
    }

    auto func = Map[idx];
    (this->*func)(op);
}

// void ClientImpl::brokerConnected(bool sessionPresent)
// {
//     static_cast<void>(sessionPresent);
//     m_clientState.m_firstConnect = false;
//     m_sessionState.m_connected = true;

//     do {
//         if (sessionPresent) {
//             for (auto& sendOpPtr : m_sendOps) {
//                 sendOpPtr->postReconnectionResend();
//             }  

//             for (auto& recvOpPtr : m_recvOps) {
//                 recvOpPtr->postReconnectionResume();
//             }    

//             auto resumeUntilIdx = m_sendOps.size(); 
//             auto resumeFromIdx = resumeUntilIdx; 
//             for (auto count = resumeUntilIdx; count > 0U; --count) {
//                 auto idx = count - 1U;
//                 auto& sendOpPtr = m_sendOps[idx];
//                 if (!sendOpPtr->isPaused()) {
//                     break;
//                 }

//                 resumeFromIdx = idx;
//             }

//             if (resumeFromIdx < resumeUntilIdx) {
//                 resumeSendOpsSince(static_cast<unsigned>(resumeFromIdx));
//             }            

//             break;
//         }

//         // Old stored session, terminate pending ops
//         for (auto* op : m_ops) {
//             auto opType = op->type();
//             if ((opType != op::Op::Type::Type_Send) && 
//                 (opType != op::Op::Type::Type_Recv)) {
//                 continue;
//             }

//             op->terminateOp(CC_MqttsnAsyncOpStatus_Aborted);
//         }
//     } while (false);

//     createKeepAliveOpIfNeeded();    
// }

// void ClientImpl::brokerDisconnected(
//     CC_MqttsnBrokerDisconnectReason reason, 
//     CC_MqttsnAsyncOpStatus status)
// {
//     m_clientState.m_initialized = false; // Require re-initialization
//     m_sessionState.m_connected = false;

//     m_sessionState.m_disconnecting = true;
//     terminateOps(status, TerminateMode_KeepSendRecvOps);    

//     for (auto* op : m_ops) {
//         if (op != nullptr) {
//             op->connectivityChanged();
//         }
//     } 

//     if (reason < CC_MqttsnBrokerDisconnectReason_ValuesLimit) {
//         COMMS_ASSERT(m_brokerDisconnectReportCb != nullptr);
//         m_brokerDisconnectReportCb(m_brokerDisconnectReportData, reason);
//     }
// }

// void ClientImpl::reportMsgInfo(const CC_MqttsnMessageInfo& info)
// {
//     COMMS_ASSERT(m_messageReceivedReportCb != nullptr);
//     m_messageReceivedReportCb(m_messageReceivedReportData, &info);
// }

// bool ClientImpl::hasPausedSendsBefore(const op::SendOp* sendOp) const
// {
//     auto riter = 
//         std::find_if(
//             m_sendOps.rbegin(), m_sendOps.rend(),
//             [sendOp](auto& opPtr)
//             {
//                 return opPtr.get() == sendOp;
//             });

//     COMMS_ASSERT(riter != m_sendOps.rend());
//     if (riter == m_sendOps.rend()) {
//         return false;
//     }

//     auto iter = riter.base() - 1;
//     auto idx = static_cast<unsigned>(std::distance(m_sendOps.begin(), iter));
//     COMMS_ASSERT(idx < m_sendOps.size());
//     if (idx == 0U) {
//         return false;
//     }

//     auto& prevSendOpPtr = m_sendOps[idx - 1U];
//     return prevSendOpPtr->isPaused();
// }

// bool ClientImpl::hasHigherQosSendsBefore(const op::SendOp* sendOp, op::Op::Qos qos) const
// {
//     for (auto& sendOpPtr : m_sendOps) {
//         if (sendOpPtr.get() == sendOp) {
//             return false;
//         }

//         if (sendOpPtr->qos() > qos) {
//             return true;
//         }
//     }

//     COMMS_ASSERT(false); // Mustn't reach here
//     return false;
// }

void ClientImpl::allowNextPrepare()
{
    COMMS_ASSERT(m_preparationLocked);
    m_preparationLocked = false;
}

void ClientImpl::doApiEnter()
{
    ++m_apiEnterCount;
    if ((m_apiEnterCount > 1U) || (m_cancelNextTickWaitCb == nullptr)) {
        return;
    }

    auto prevWait = m_timerMgr.getMinWait();
    if (prevWait == 0U) {
        return;
    }

    auto elapsed = m_cancelNextTickWaitCb(m_cancelNextTickWaitData);
    m_clientState.m_timestamp += elapsed;
    m_timerMgr.tick(elapsed);
}

void ClientImpl::doApiExit()
{
    COMMS_ASSERT(m_apiEnterCount > 0U);
    --m_apiEnterCount;
    if (m_apiEnterCount > 0U) {
        return;
    }

    cleanOps();

    if (m_nextTickProgramCb == nullptr) {
        return;
    }

    auto nextWait = m_timerMgr.getMinWait();
    if (nextWait == 0U) {
        return;
    }

    m_nextTickProgramCb(m_nextTickProgramData, nextWait);
}

// void ClientImpl::createKeepAliveOpIfNeeded()
// {
//     if (!m_keepAliveOps.empty()) {
//         return;
//     }

//     auto ptr = m_keepAliveOpsAlloc.alloc(*this);
//     if (!ptr) {
//         COMMS_ASSERT(false); // Should not happen
//         return;
//     }    

//     m_ops.push_back(ptr.get());
//     m_keepAliveOps.push_back(std::move(ptr));
// }

// void ClientImpl::terminateOps(CC_MqttsnAsyncOpStatus status, TerminateMode mode)
// {
//     for (auto* op : m_ops) {
//         if (op == nullptr) {
//             continue;
//         }

//         if (mode == TerminateMode_KeepSendRecvOps) {
//             auto opType = op->type();

//             if ((opType == op::Op::Type_Recv) || (opType == op::Op::Type_Send)) {
//                 continue;
//             }
//         }

//         op->terminateOp(status);
//     }
// }

void ClientImpl::cleanOps()
{
//     if (!m_opsDeleted) {
//         return;
//     }

//     m_ops.erase(
//         std::remove_if(
//             m_ops.begin(), m_ops.end(),
//             [](auto* op)
//             {
//                 return op == nullptr;
//             }),
//         m_ops.end());

//     m_opsDeleted = false;
}

void ClientImpl::errorLogInternal(const char* msg)
{
    if constexpr (Config::HasErrorLog) {
        if (m_errorLogCb == nullptr) {
            return;
        }

        m_errorLogCb(m_errorLogData, msg);
    }
}

CC_MqttsnErrorCode ClientImpl::initInternal()
{
    auto guard = apiEnter();

    if ((m_sendOutputDataCb == nullptr) ||
        (m_messageReceivedReportCb == nullptr)) {
        errorLog("Hasn't set all must have callbacks");
        return CC_MqttsnErrorCode_NotIntitialized;
    }

    bool hasTimerCallbacks = 
        (m_nextTickProgramCb != nullptr) ||
        (m_cancelNextTickWaitCb != nullptr);

    if (hasTimerCallbacks) {
        bool hasAllTimerCallbacks = 
            (m_nextTickProgramCb != nullptr) &&
            (m_cancelNextTickWaitCb != nullptr);

        if (!hasAllTimerCallbacks) {
            errorLog("Hasn't set all timer management callbacks callbacks");
            return CC_MqttsnErrorCode_NotIntitialized;
        }
    }

    // TODO:
    // terminateOps(CC_MqttsnAsyncOpStatus_Aborted, TerminateMode_KeepSendRecvOps);
    // m_sessionState = SessionState();
    m_clientState.m_initialized = true;
    return CC_MqttsnErrorCode_Success;
}

// void ClientImpl::resumeSendOpsSince(unsigned idx)
// {
//     while (idx < m_sendOps.size()) {
//         auto& opToResumePtr = m_sendOps[idx];
//         if (!opToResumePtr->isPaused()) {
//             ++idx;
//             continue;
//         }         
        
//         if (!opToResumePtr->resume()) {
//             break;
//         }

//         // After resuming some (QoS0) ops can complete right away, increment idx next iteration
//     }
// }

// op::SendOp* ClientImpl::findSendOp(std::uint16_t packetId)
// {
//     auto iter = 
//         std::find_if(
//             m_sendOps.begin(), m_sendOps.end(),
//             [packetId](auto& opPtr)
//             {
//                 COMMS_ASSERT(opPtr);
//                 return opPtr->packetId() == packetId;
//             });

//     if (iter == m_sendOps.end()) {
//         return nullptr;
//     }

//     return iter->get();
// }

// bool ClientImpl::isLegitSendAck(const op::SendOp* sendOp, bool pubcompAck) const
// {
//     if (!sendOp->isPublished()) {
//         return false;
//     }

//     for (auto& sendOpPtr : m_sendOps) {
//         if (sendOpPtr.get() == sendOp) {
//             return true;
//         }

//         if (!sendOpPtr->isAcked()) {
//             return false;
//         }

//         if (pubcompAck && (sendOp != m_sendOps.front().get())) {
//             return false;
//         }
//     }

//     COMMS_ASSERT(false); // Should not be reached;
//     return false;
// }

// void ClientImpl::resendAllUntil(op::SendOp* sendOp)
// {
//     // Do index controlled iteration because forcing dup resend can
//     // cause early message destruction.
//     for (auto idx = 0U; idx < m_sendOps.size();) {
//         auto& sendOpPtr = m_sendOps[idx];
//         COMMS_ASSERT(sendOpPtr);
//         auto* opBeforeResend = sendOpPtr.get();
//         sendOpPtr->forceDupResend(); // can destruct object
//         if (opBeforeResend == sendOp) {
//             break;
//         }

//         auto* opAfterResend = sendOpPtr.get();
//         if (opBeforeResend != opAfterResend) {
//             // The op object was destructed and erased, 
//             // do not increment index;
//             continue;
//         }

//         ++idx;
//     }
// }

// bool ClientImpl::processPublishAckMsg(ProtMessage& msg, std::uint16_t packetId, bool pubcompAck)
// {
//     for (auto& opPtr : m_keepAliveOps) {
//         msg.dispatch(*opPtr);
//     }     

//     auto* sendOp = findSendOp(packetId);
//     if (sendOp == nullptr) {
//         return false;
//     }

//     if (isLegitSendAck(sendOp, pubcompAck)) {
//         msg.dispatch(*sendOp);
//         return true;        
//     }

//     resendAllUntil(sendOp);
//     return true;
// }

void ClientImpl::opComplete_Search(const op::Op* op)
{
    eraseFromList(op, m_searchOps);
}

void ClientImpl::opComplete_Connect(const op::Op* op)
{
    eraseFromList(op, m_connectOps);
}

// void ClientImpl::opComplete_KeepAlive(const op::Op* op)
// {
//     eraseFromList(op, m_keepAliveOps);
// }

// void ClientImpl::opComplete_Disconnect(const op::Op* op)
// {
//     eraseFromList(op, m_disconnectOps);
// }

// void ClientImpl::opComplete_Subscribe(const op::Op* op)
// {
//     eraseFromList(op, m_subscribeOps);
// }

// void ClientImpl::opComplete_Unsubscribe(const op::Op* op)
// {
//     eraseFromList(op, m_unsubscribeOps);
// }

// void ClientImpl::opComplete_Recv(const op::Op* op)
// {
//     eraseFromList(op, m_recvOps);
// }

// void ClientImpl::opComplete_Send(const op::Op* op)
// {
//     auto idx = eraseFromList(op, m_sendOps);
//     if (m_sessionState.m_disconnecting) {
//         return;
//     }

//     resumeSendOpsSince(idx);
// }

void ClientImpl::monitorGatewayExpiry()
{
    if constexpr (Config::HasGatewayDiscovery) {
        auto iter = 
            std::min_element(
                m_clientState.m_gwInfos.begin(), m_clientState.m_gwInfos.end(),
                [](const auto& first, const auto& second)
                {
                    COMMS_ASSERT(first.m_expiryTimestamp != 0);
                    COMMS_ASSERT(second.m_expiryTimestamp != 0);

                    return first.m_expiryTimestamp < second.m_expiryTimestamp;
                });

        if (iter == m_clientState.m_gwInfos.end()) {
            return;
        }

        COMMS_ASSERT(m_clientState.m_timestamp < iter->m_expiryTimestamp);
        m_gwDiscoveryTimer.wait(iter->m_expiryTimestamp - m_clientState.m_timestamp, &ClientImpl::gwExpiryTimeoutCb, this);
    }
}

void ClientImpl::gwExpiryTimeout()
{
    if constexpr (Config::HasGatewayDiscovery) {
        for (auto& info : m_clientState.m_gwInfos) {
            if (m_clientState.m_timestamp < info.m_expiryTimestamp) {
                continue;
            }

            if (info.m_allowedAdvLosses == 0U) {
                reportGwStatus(CC_MqttsnGwStatus_Removed, info);
                continue;
            }

            --info.m_allowedAdvLosses;
            info.m_expiryTimestamp += info.m_duration;
            reportGwStatus(CC_MqttsnGwStatus_Tentative, info);
        }

        m_clientState.m_gwInfos.erase(
            std::remove_if(
                m_clientState.m_gwInfos.begin(), m_clientState.m_gwInfos.end(),
                [this](auto& info)
                {
                    return (info.m_expiryTimestamp <= m_clientState.m_timestamp);
                }),
            m_clientState.m_gwInfos.end());

        monitorGatewayExpiry();
    }
}

void ClientImpl::reportGwStatus(CC_MqttsnGwStatus status, const ClientState::GwInfo& info)
{
    if constexpr (Config::HasGatewayDiscovery) {
        if (m_gatewayStatusReportCb == nullptr) {
            return;
        }

        auto gwInfo = CC_MqttsnGatewayInfo();
        gwInfo.m_gwId = info.m_gwId;
        gwInfo.m_addr = info.m_addr.data();
        comms::cast_assign(gwInfo.m_addrLen) = info.m_addr.size();

        m_gatewayStatusReportCb(m_gatewayStatusReportData, status, &gwInfo);
    }
}

void ClientImpl::sendGwinfo()
{
    if constexpr (Config::HasGatewayDiscovery) {
        auto iter = 
            std::max_element(
                m_clientState.m_gwInfos.begin(), m_clientState.m_gwInfos.end(),
                [](auto& first, auto& second)
                {
                    if (first.m_addr.empty()) {
                        // Prefer one with the address
                        return !second.m_addr.empty();
                    }

                    if (second.m_addr.empty()) {
                        return false;
                    }

                    return first.m_expiryTimestamp < second.m_expiryTimestamp;
                });

        if ((iter == m_clientState.m_gwInfos.end()) || 
            (iter->m_addr.empty())) {
            // None of the gateways have known address
            return;
        }

        GwinfoMsg msg;
        if (msg.field_gwAdd().value().max_size() < iter->m_addr.size()) {
            errorLog("Cannot fit the known gateway address into the GWINFO message address storage");
            return;
        }

        msg.field_gwId().setValue(iter->m_gwId);
        comms::util::assign(msg.field_gwAdd().value(), iter->m_addr.begin(), iter->m_addr.end());
        sendMessage(msg, std::max(m_pendingGwinfoBroadcastRadius, 1U));
    }
}

void ClientImpl::gwExpiryTimeoutCb(void* data)
{
    if constexpr (Config::HasGatewayDiscovery) {
        reinterpret_cast<ClientImpl*>(data)->gwExpiryTimeout();
    }
}

void ClientImpl::sendGwinfoCb(void* data)
{
    if constexpr (Config::HasGatewayDiscovery) {
        reinterpret_cast<ClientImpl*>(data)->sendGwinfo();
    }
}

} // namespace cc_mqttsn_client
