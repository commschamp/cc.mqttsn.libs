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
    typedef PublishMsg MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_PublishFlags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& retainFlagField = std::get<mqtt::message::PublishActualFlagIdx_Retain>(flagsMembers);
    auto& qosField = std::get<mqtt::message::PublishActualFlagIdx_QoS>(flagsMembers);
    auto& dupFlagField = std::get<mqtt::message::PublishActualFlagIdx_Dup>(flagsMembers);
    auto& topicField = std::get<MsgType::FieldIdx_Topic>(fields);
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);
    auto& payloadField = std::get<MsgType::FieldIdx_Payload>(fields);

    bool retain = (retainFlagField.value() != 0);
    bool dup = (dupFlagField.value() != 0);

    if (qosField.value() == mqtt::field::QosType::AtLeastOnceDelivery) {
        PubackMsg respMsg;
        auto& respFields = respMsg.fields();
        auto& respPacketIdField = std::get<decltype(respMsg)::FieldIdx_PacketId>(respFields);

        assert(packetIdField.getMode() == comms::field::OptionalMode::Exists);
        respPacketIdField.value() = packetIdField.field().value();
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
        [this, &packetIdField, cleanIncompleteFunc]()
        {
            cleanIncompleteFunc();

            if (packetIdField.getMode() != comms::field::OptionalMode::Exists) {
                return;
            }

            m_recvMsgs.remove_if(
                [packetIdField](BrokPubInfosList::const_reference elem) -> bool
                {
                    return elem.m_packetId == packetIdField.field().value();
                });
        };

    if (qosField.value() <= mqtt::field::QosType::AtLeastOnceDelivery) {
        cleanPubsFunc();
        PubInfoPtr pubInfo(new PubInfo);
        pubInfo->m_topic = topicField.value();
        pubInfo->m_msg = payloadField.value();
        pubInfo->m_qos = translateQos(qosField.value());
        pubInfo->m_retain = retain;
        pubInfo->m_dup = dup;
        addPubInfo(std::move(pubInfo));
        return;
    }

    assert(packetIdField.getMode() == comms::field::OptionalMode::Exists);
    auto sendRecFunc =
        [this, &packetIdField]()
        {
            PubrecMsg respMsg;
            auto& respFields = respMsg.fields();
            auto& respPacketIdField = std::get<decltype(respMsg)::FieldIdx_PacketId>(respFields);
            respPacketIdField.value() = packetIdField.field().value();
            sendToBroker(respMsg);
        };

    do {
        if (!dup) {
            break;
        }

        auto iter = std::find_if(
            m_recvMsgs.begin(), m_recvMsgs.end(),
            [&packetIdField](BrokPubInfosList::const_reference elem) -> bool
            {
                return packetIdField.field().value() == elem.m_packetId;
            });

        if (iter == m_recvMsgs.end()) {
            break;
        }

        iter->m_topic = topicField.value();
        iter->m_msg = payloadField.value();
        iter->m_dup = dup;
        iter->m_retain = retain;
        iter->m_timestamp = state().m_timestamp;
        cleanIncompleteFunc();
        sendRecFunc();
        return;
    } while (false);

    cleanPubsFunc();

    BrokPubInfo info;
    info.m_topic = topicField.value();
    info.m_msg = payloadField.value();
    info.m_dup = dup;
    info.m_retain = retain;
    info.m_packetId = packetIdField.field().value();
    info.m_timestamp = state().m_timestamp;
    m_recvMsgs.push_back(std::move(info));
    sendRecFunc();
}

void PubRecv::handle(PubrelMsg& msg)
{
    typedef PubrelMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    auto iter = std::find_if(
        m_recvMsgs.begin(), m_recvMsgs.end(),
        [&packetIdField](BrokPubInfosList::const_reference elem) -> bool
        {
            return packetIdField.value() == elem.m_packetId;
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
    auto& respFields = respMsg.fields();
    auto& respPacketIdField = std::get<decltype(respMsg)::FieldIdx_PacketId>(respFields);
    respPacketIdField.value() = packetIdField.value();
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



