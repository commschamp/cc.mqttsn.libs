//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ExtConfig.h"
#include "ObjListType.h"
#include "ProtocolDefs.h"

#include "cc_mqttsn_client/common.h"

#include <limits>

namespace cc_mqttsn_client
{

class ClientImpl;

namespace op
{

class Op : public ProtMsgHandler
{
public:
    enum Type
    {
        // Type_Connect,
        // Type_KeepAlive,
        // Type_Disconnect,
        // Type_Subscribe,
        // Type_Unsubscribe,
        // Type_Recv,
        // Type_Send,
        Type_NumOfValues // Must be last
    };

    using Qos = cc_mqttsn::field::QosCommon::ValueType;

    virtual ~Op() noexcept = default;

    Type type() const
    {
        return typeImpl();
    }

    void terminateOp(CC_MqttsnAsyncOpStatus status)
    {
        terminateOpImpl(status);
    }

    unsigned getResponseTimeout() const
    {
        return m_responseTimeoutMs;
    }

    void setResponseTimeout(unsigned ms)
    {
        m_responseTimeoutMs = ms;
    }    

    inline 
    static bool verifyQosValid(Qos qos)
    {
        return (qos <= static_cast<decltype(qos)>(Config::MaxQos));
    }    

protected:
    explicit Op(ClientImpl& client);

    virtual Type typeImpl() const = 0;
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status);

    void sendMessage(const ProtMessage& msg, unsigned broadcastRadius = 0U);
    void opComplete();
    // std::uint16_t allocPacketId();
    // void releasePacketId(std::uint16_t id);

    ClientImpl& client()
    {
        return m_client;
    }

    const ClientImpl& client() const
    {
        return m_client;
    }    

    inline void errorLog(const char* msg)
    {
        if constexpr (Config::HasErrorLog) {
            errorLogInternal(msg);
        }
    }

    // inline bool verifySubFilter(const char* filter)
    // {
    //     if (Config::HasTopicFormatVerification) {
    //         return verifySubFilterInternal(filter);
    //     }
    //     else {
    //         return true;
    //     }
    // }    

    // inline bool verifyPubTopic(const char* topic, bool outgoing)
    // {
    //     if (Config::HasTopicFormatVerification) {
    //         return verifyPubTopicInternal(topic, outgoing);
    //     }
    //     else {
    //         return true;
    //     }
    // }     

    static constexpr std::size_t maxStringLen()
    {
        return std::numeric_limits<std::uint16_t>::max();
    }

private:
    void errorLogInternal(const char* msg);
    // bool verifySubFilterInternal(const char* filter);
    // bool verifyPubTopicInternal(const char* topic, bool outgoing);

    ClientImpl& m_client;    
    unsigned m_responseTimeoutMs = 0U;
    unsigned m_retryCount = 0U;
};

} // namespace op

} // namespace cc_mqttsn_client
