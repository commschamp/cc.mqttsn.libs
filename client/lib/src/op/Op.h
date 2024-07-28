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
#include "ReuseState.h"

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
        Type_Search,
        Type_Connect,
        Type_KeepAlive,
        Type_Disconnect,
        Type_Subscribe,
        Type_Unsubscribe,
        // Type_Recv,
        Type_Send,
        Type_Will,
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

    unsigned getRetryPeriod() const
    {
        return m_retryPeriod;
    }

    void setRetryPeriod(unsigned ms)
    {
        m_retryPeriod = ms;
    }   

    unsigned getRetryCount() const
    {
        return m_retryCount;
    }

    void setRetryCount(unsigned value)
    {
        m_retryCount = value;
    }        

    inline 
    static bool verifyQosValid(Qos qos)
    {
        return (qos <= static_cast<decltype(qos)>(Config::MaxQos));
    }    

    ClientImpl& client()
    {
        return m_client;
    }    

protected:
    explicit Op(ClientImpl& client);

    virtual Type typeImpl() const = 0;
    virtual void terminateOpImpl(CC_MqttsnAsyncOpStatus status);

    static CC_MqttsnAsyncOpStatus translateErrorCodeToAsyncOpStatus(CC_MqttsnErrorCode ec);
    CC_MqttsnErrorCode sendMessage(const ProtMessage& msg, unsigned broadcastRadius = 0U);
    void opComplete();
    std::uint16_t allocPacketId();
    void releasePacketId(std::uint16_t id);
    void decRetryCount();
    void storeInRegTopic(const char* topic, CC_MqttsnTopicId topicId);
    bool removeInRegTopic(const char* topic, CC_MqttsnTopicId topicId);

    static bool isValidTopicId(CC_MqttsnTopicId id);
    static bool isShortTopic(const char* topic);

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

    inline bool verifySubFilter(const char* filter)
    {
        if constexpr (Config::HasTopicFormatVerification) {
            return verifySubFilterInternal(filter);
        }
        else {
            return true;
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

    static constexpr std::size_t maxStringLen()
    {
        return std::numeric_limits<std::uint16_t>::max();
    }

private:
    void errorLogInternal(const char* msg);
    bool verifySubFilterInternal(const char* filter);
    bool verifyPubTopicInternal(const char* topic, bool outgoing);

    ClientImpl& m_client;    
    unsigned m_retryPeriod = 0U;
    unsigned m_retryCount = 0U;
};

} // namespace op

} // namespace cc_mqttsn_client
