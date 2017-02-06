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

#include "PubRecv.h"

#include <cassert>
#include <algorithm>

namespace mqttsn
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
    auto& pubFlags = msg.field_publishFlags();
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

}  // namespace mqttsn



