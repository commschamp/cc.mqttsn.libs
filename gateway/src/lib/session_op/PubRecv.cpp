//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "PubRecv.h"

#include <cassert>
#include <algorithm>

namespace cc_mqttsn
{

namespace gateway
{

namespace session_op
{

PubRecv::PubRecv(SessionState& sessionState)
  : Base(sessionState)
{
}

PubRecv::~PubRecv() = default;

void PubRecv::handle(PublishMsg& msg)
{
    auto& pubFlags = msg.transportField_flags();
    bool retain = (pubFlags.field_retain().value() != 0);
    bool dup = (pubFlags.field_dup().value() != 0);

    typedef typename std::decay<decltype(pubFlags.field_qos())>::type QosFieldType;
    if (pubFlags.field_qos().value() == QosFieldType::ValueType::AtLeastOnceDelivery) {
        PubackMsg respMsg;

        assert(msg.field_packetId().doesExist());
        respMsg.field_packetId().value() = msg.field_packetId().field().value();
        sendToBroker(respMsg);
    }

    auto cleanIncompleteFunc =
        [this]()
        {
            m_recvMsgs.remove_if(
                [this](BrokPubInfosList::const_reference elem) -> bool
                {
                    return (elem.m_timestamp + state().m_retryPeriod) < state().m_timestamp;
                });
        };

    auto cleanPubsFunc =
        [this, &msg, cleanIncompleteFunc]()
        {
            cleanIncompleteFunc();

            if (!msg.field_packetId().doesExist()) {
                return;
            }

            m_recvMsgs.remove_if(
                [&msg](BrokPubInfosList::const_reference elem) -> bool
                {
                    return elem.m_packetId == msg.field_packetId().field().value();
                });
        };

    if (pubFlags.field_qos().value() <= QosFieldType::ValueType::AtLeastOnceDelivery) {
        cleanPubsFunc();
        PubInfoPtr pubInfo(new PubInfo);
        pubInfo->m_topic = msg.field_topic().value();
        pubInfo->m_msg = msg.field_payload().value();
        pubInfo->m_qos = translateQos(pubFlags.field_qos().value());
        pubInfo->m_retain = retain;
        pubInfo->m_dup = dup;
        addPubInfo(std::move(pubInfo));
        return;
    }

    assert(msg.field_packetId().doesExist());
    auto sendRecFunc =
        [this, &msg]()
        {
            PubrecMsg respMsg;
            respMsg.field_packetId().value() = msg.field_packetId().field().value();
            sendToBroker(respMsg);
        };

    do {
        if (!dup) {
            break;
        }

        auto iter = std::find_if(
            m_recvMsgs.begin(), m_recvMsgs.end(),
            [&msg](BrokPubInfosList::const_reference elem) -> bool
            {
                return msg.field_packetId().field().value() == elem.m_packetId;
            });

        if (iter == m_recvMsgs.end()) {
            break;
        }

        iter->m_topic = msg.field_topic().value();
        iter->m_msg = msg.field_payload().value();
        iter->m_dup = dup;
        iter->m_retain = retain;
        iter->m_timestamp = state().m_timestamp;
        cleanIncompleteFunc();
        sendRecFunc();
        return;
    } while (false);

    cleanPubsFunc();

    BrokPubInfo info;
    info.m_topic = msg.field_topic().value();
    info.m_msg = msg.field_payload().value();
    info.m_dup = dup;
    info.m_retain = retain;
    info.m_packetId = msg.field_packetId().field().value();
    info.m_timestamp = state().m_timestamp;
    m_recvMsgs.push_back(std::move(info));
    sendRecFunc();
}

void PubRecv::handle(PubrelMsg& msg)
{
    auto iter = std::find_if(
        m_recvMsgs.begin(), m_recvMsgs.end(),
        [&msg](BrokPubInfosList::const_reference elem) -> bool
        {
            return msg.field_packetId().value() == elem.m_packetId;
        });

    if (iter != m_recvMsgs.end()) {
        PubInfoPtr pubInfo(new PubInfo);
        pubInfo->m_topic = iter->m_topic;
        pubInfo->m_msg = iter->m_msg;
        pubInfo->m_qos = QoS_ExactlyOnceDelivery;
        pubInfo->m_retain = iter->m_retain;
        pubInfo->m_dup = false;
        addPubInfo(std::move(pubInfo));
        m_recvMsgs.erase(iter);
    }

    PubcompMsg respMsg;
    respMsg.field_packetId().value() = msg.field_packetId().value();
    sendToBroker(respMsg);
}

void PubRecv::addPubInfo(PubInfoPtr info)
{
    auto& st = state();
    while (st.m_sleepPubAccLimit <= st.m_brokerPubs.size()) {
        st.m_brokerPubs.pop_front();
    }
    st.m_brokerPubs.push_back(std::move(info));
}

}  // namespace session_op

}  // namespace gateway

}  // namespace cc_mqttsn



