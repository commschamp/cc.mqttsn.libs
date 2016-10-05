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

    auto cleanPubsFunc =
        [this, &packetIdField]()
        {
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
        PubInfo pubInfo;
        pubInfo.m_topic = topicField.value();
        pubInfo.m_msg = payloadField.value();
        pubInfo.m_qos = translateQos(qosField.value());
        pubInfo.m_retain = retain;
        pubInfo.m_dup = dup;
        state().m_brokerPubs.push_back(std::move(pubInfo));
        return;
    }

    if (!dup) {
        cleanPubsFunc();

        // TODO: insert new elem
    }
    else {

    }


//    BrokPubInfo info;
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



