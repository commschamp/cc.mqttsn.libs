//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <limits>

#include "cc_mqttsn/Version.h"
#include "cc_mqttsn/field/QosCommon.h"
#include "cc_mqtt311/Version.h"
#include "cc_mqtt311/field/QosCommon.h"

#include "RegMgr.h"

static_assert(COMMS_MAKE_VERSION(2, 7, 1) <= cc_mqttsn::version(),
    "The version of cc.mqttsn.generated library is too old");

static_assert(COMMS_MAKE_VERSION(2, 7, 0) <= cc_mqtt311::version(),
    "The version of cc.mqtt311.generated library is too old");    

namespace cc_mqttsn_gateway
{

typedef std::vector<std::uint8_t> DataBuf;

enum QoS
{
    QoS_AtMostOnceDelivery,
    QoS_AtLeastOnceDelivery,
    QoS_ExactlyOnceDelivery,
    QoS_NumOfValues
};

inline
cc_mqtt311::field::QosVal translateQosForBroker(QoS val)
{
    return static_cast<cc_mqtt311::field::QosVal>(val);
}

inline
cc_mqttsn::field::QosVal translateQosForClient(QoS val)
{
    return static_cast<cc_mqttsn::field::QosVal>(val);
}

inline
QoS translateQos(cc_mqtt311::field::QosVal val)
{
    return static_cast<QoS>(val);
}

inline
QoS translateQos(cc_mqttsn::field::QosVal val)
{
    if (val == cc_mqttsn::field::QosVal::NoGwPublish) {
        return QoS_AtMostOnceDelivery;
    }

    return static_cast<QoS>(val);
}

struct WillInfo
{
    std::string m_topic;
    DataBuf m_msg;
    QoS m_qos = QoS_AtMostOnceDelivery;
    bool m_retain = false;
};

inline
bool operator==(const WillInfo& info1, const WillInfo& info2)
{
    return
        ((info1.m_topic == info2.m_topic) &&
         (info1.m_msg == info2.m_msg) &&
         (info1.m_qos == info2.m_qos) &&
         (info1.m_retain == info2.m_retain));
}

inline
bool operator!=(const WillInfo& info1, const WillInfo& info2)
{
    return !(info1 == info2);
}

enum class ConnectionStatus
{
    Disconnected,
    Connected,
    Asleep
};

typedef unsigned long long Timestamp;

struct PubInfo
{
    std::string m_topic;
    DataBuf m_msg;
    QoS m_qos = QoS_AtMostOnceDelivery;
    bool m_retain = false;
    bool m_dup = false;
};

typedef std::unique_ptr<PubInfo> PubInfoPtr;

struct SessionState
{
    static const unsigned DefaultRetryPeriod = 10 * 1000;
    static const unsigned DefaultRetryCount = 3;
    static const Timestamp InitialTimestamp = 1000U;
    static const std::uint16_t DefaultKeepAlive = 60U;

    unsigned m_retryPeriod = DefaultRetryPeriod;
    unsigned m_retryCount = DefaultRetryCount;
    unsigned m_tickReq = 0U;
    unsigned m_callStackCount = 0U;
    Timestamp m_timestamp = InitialTimestamp;
    Timestamp m_lastMsgTimestamp = InitialTimestamp;
    std::size_t m_sleepPubAccLimit = std::numeric_limits<std::size_t>::max();
    std::string m_clientId;
    std::string m_defaultClientId;
    std::string m_username;
    DataBuf m_password;
    WillInfo m_will;
    std::list<PubInfoPtr> m_brokerPubs;
    RegMgr m_regMgr;
    ConnectionStatus m_connStatus = ConnectionStatus::Disconnected;
    std::uint16_t m_keepAlive = 0U;
    std::uint16_t m_pubOnlyKeepAlive = DefaultKeepAlive;
    std::uint8_t m_gwId = 0U;
    bool m_running = false;
    bool m_brokerConnected = false;
    bool m_reconnectingBroker = false;
    bool m_terminating = false;
    bool m_pendingClientDisconnect = false;
    bool m_clientConnectReported = false;    
    bool m_encapsulatedMsg = false;
};

}  // namespace cc_mqttsn_gateway
