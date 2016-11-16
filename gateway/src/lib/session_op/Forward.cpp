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

#include "Forward.h"

#include <cassert>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace session_op
{

Forward::Forward(SessionState& sessionState)
  : Base(sessionState)
{
}

Forward::~Forward() = default;

void Forward::handle(PublishMsg_SN& msg)
{
    typedef PublishMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
    auto& topicIdField = std::get<MsgType::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    auto& dataField = std::get<MsgType::FieldIdx_data>(fields);

    auto& st = state();

    do {

        if ((qosField.value() != mqttsn::protocol::field::QosType::NoGwPublish) ||
            (st.m_connStatus == ConnectionStatus::Connected)) {
            break;
        }

        if (st.m_connStatus != ConnectionStatus::Disconnected) {
            return;
        }

        NoGwPubInfo info;
        info.m_topicId = topicIdField.value();
        info.m_data = dataField.value();
        m_pubs.push_back(std::move(info));
        return;
    } while (false);

    if (st.m_connStatus != ConnectionStatus::Connected) {
        sendPubackToClient(
            topicIdField.value(),
            msgIdField.value(),
            mqttsn::protocol::field::ReturnCodeVal_NotSupported);
        return;
    }

    if (!st.m_brokerConnected) {
        sendPubackToClient(
            topicIdField.value(),
            msgIdField.value(),
            mqttsn::protocol::field::ReturnCodeVal_Congestion);
        return;
    }

    auto& topic = st.m_regMgr.mapTopicId(topicIdField.value());
    if (topic.empty()) {
        sendPubackToClient(
            topicIdField.value(),
            msgIdField.value(),
            mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId);
        sendToBroker(PingreqMsg());
        return;
    }

    bool retain = midFlagsField.getBitValue(mqttsn::protocol::field::MidFlagsBits_retain);
    bool dup = dupFlagsField.getBitValue(mqttsn::protocol::field::DupFlagsBits_dup);
    m_lastPubTopicId = topicIdField.value();

    PublishMsg fwdMsg;
    auto& fwdFields =fwdMsg.fields();
    auto& fwdFlagsField = std::get<decltype(fwdMsg)::FieldIdx_PublishFlags>(fwdFields);
    auto& fwdFlagsMembers = fwdFlagsField.value();
    auto& fwdRetainFlagsField = std::get<mqtt::message::PublishActualFlagIdx_Retain>(fwdFlagsMembers);
    auto& fwdQosField = std::get<mqtt::message::PublishActualFlagIdx_QoS>(fwdFlagsMembers);
    auto& fwdDupFlagsField = std::get<mqtt::message::PublishActualFlagIdx_Dup>(fwdFlagsMembers);
    auto& fwdTopicField = std::get<decltype(fwdMsg)::FieldIdx_Topic>(fwdFields);
    auto& fwdPacketIdField = std::get<decltype(fwdMsg)::FieldIdx_PacketId>(fwdFields);
    auto& fwdPayloadField = std::get<decltype(fwdMsg)::FieldIdx_Payload>(fwdFields);

    fwdRetainFlagsField.setBitValue(0, retain);
    fwdQosField.value() = translateQosForBroker(translateQos(qosField.value()));
    fwdDupFlagsField.setBitValue(0, dup);
    fwdTopicField.value() = topic;
    fwdPacketIdField.field().value() = msgIdField.value();
    fwdPayloadField.value() = dataField.value();
    fwdMsg.refresh();
    sendToBroker(fwdMsg);
}

void Forward::handle(PubrelMsg_SN& msg)
{
    typedef PubrelMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);

    PubrelMsg fwdMsg;
    auto& fwdFields = fwdMsg.fields();
    auto& packetIdField = std::get<decltype(fwdMsg)::FieldIdx_PacketId>(fwdFields);

    packetIdField.value() = msgIdField.value();
    sendToBroker(fwdMsg);
}

void Forward::handle(PingreqMsg_SN& msg)
{
    static_cast<void>(msg);
    if (state().m_connStatus != ConnectionStatus::Connected) {
        return;
    }

    m_pingInProgress = true;
    sendToBroker(PingreqMsg());
}

void Forward::handle(PingrespMsg_SN& msg)
{
    static_cast<void>(msg);
    sendToBroker(PingrespMsg());
}

void Forward::handle(SubscribeMsg_SN& msg)
{
    typedef SubscribeMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& flagsField = std::get<MsgType::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    auto& topicIdField = std::get<MsgType::FieldIdx_topicId>(fields);
    auto& topicNameField = std::get<MsgType::FieldIdx_topicName>(fields);

    auto sendSubackFunc =
        [this, &qosField, &msgIdField, &topicIdField](mqttsn::protocol::field::ReturnCodeVal rc)
        {
            SubackMsg_SN respMsg;
            auto& respFields = respMsg.fields();
            auto& respFlagsField = std::get<decltype(respMsg)::FieldIdx_flags>(respFields);
            auto& respFlagsMembers = respFlagsField.value();
            auto& respQosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(respFlagsMembers);
            auto& respTopicIdField = std::get<decltype(respMsg)::FieldIdx_topicId>(respFields);
            auto& respMsgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);
            auto& respRetCodeField = std::get<decltype(respMsg)::FieldIdx_returnCode>(respFields);

            respQosField.value() = qosField.value();
            respTopicIdField.value() = topicIdField.field().value();
            respMsgIdField.value() = msgIdField.value();
            respRetCodeField.value() = rc;
            sendToClient(respMsg);
        };

    if (state().m_connStatus != ConnectionStatus::Connected) {
        sendSubackFunc(mqttsn::protocol::field::ReturnCodeVal_NotSupported);
        return;
    }


    const std::string* topic = nullptr;
    std::uint16_t topicId = 0U;
    do {
        if (topicNameField.getMode() == comms::field::OptionalMode::Exists) {
            assert(topicIdField.getMode() == comms::field::OptionalMode::Missing);
            topic = &topicNameField.field().value();

            if (topic->empty()) {
                sendSubackFunc(mqttsn::protocol::field::ReturnCodeVal_NotSupported);
                sendToBroker(PingreqMsg());
                return;
            }

            bool hasWildcards =
                std::any_of(topic->begin(), topic->end(),
                    [](char ch) -> bool
                    {
                        return (ch == '#') || (ch == '+');
                    });

            if (hasWildcards) {
                break;
            }

            topicId = state().m_regMgr.mapTopicNoInfo(*topic);
            break;
        }

        assert(topicIdField.getMode() == comms::field::OptionalMode::Exists);
        assert(topicNameField.getMode() == comms::field::OptionalMode::Missing);

        auto& topicStr = state().m_regMgr.mapTopicId(topicIdField.field().value());
        if (!topicStr.empty()) {
            topic = &topicStr;
            topicId = topicIdField.field().value();
            break;
        }

        sendSubackFunc(mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId);
        sendToBroker(PingreqMsg());
        return;
    } while (false);

    SubInfo info;
    info.m_timestamp = state().m_timestamp;
    info.m_msgId = msgIdField.value();
    info.m_topicId = topicId;
    m_subs.push_back(info);

    SubscribeMsg fwdMsg;
    auto& fwdFields = fwdMsg.fields();
    auto& packetIdField = std::get<decltype(fwdMsg)::FieldIdx_PacketId>(fwdFields);
    auto& payloadField = std::get<decltype(fwdMsg)::FieldIdx_Payload>(fwdFields);

    packetIdField.value() = msgIdField.value();
    auto& payloadContainer = payloadField.value();
    typedef std::decay<decltype(payloadContainer)>::type ContainerType;
    typedef ContainerType::value_type SubElemBundle;

    SubElemBundle subElem;
    auto& subMembers = subElem.value();
    auto& subTopicField = std::get<0>(subMembers);
    auto& subQosField = std::get<1>(subMembers);

    assert(topic != nullptr);
    subTopicField.value() = *topic;
    subQosField.value() = translateQosForBroker(translateQos(qosField.value()));

    payloadContainer.push_back(std::move(subElem));
    sendToBroker(fwdMsg);
}

void Forward::handle(UnsubscribeMsg_SN& msg)
{
    if (state().m_connStatus != ConnectionStatus::Connected) {
        return;
    }

    typedef UnsubscribeMsg_SN MsgType;
    auto& fields = msg.fields();
    auto& msgIdField = std::get<MsgType::FieldIdx_msgId>(fields);
    auto& topicIdField = std::get<MsgType::FieldIdx_topicId>(fields);
    auto& topicNameField = std::get<MsgType::FieldIdx_topicName>(fields);

    const std::string* topic = nullptr;
    do {
        if (topicNameField.getMode() == comms::field::OptionalMode::Exists) {
            assert(topicIdField.getMode() == comms::field::OptionalMode::Missing);
            topic = &topicNameField.field().value();

            if (topic->empty()) {
                return;
            }

            break;
        }

        assert(topicIdField.getMode() == comms::field::OptionalMode::Exists);
        assert(topicNameField.getMode() == comms::field::OptionalMode::Missing);

        auto& topicStr = state().m_regMgr.mapTopicId(topicIdField.field().value());
        if (topicStr.empty()) {
            return;
        }
        topic = &topicStr;
    } while (false);

    UnsubscribeMsg fwdMsg;
    auto& fwdFields = fwdMsg.fields();
    auto& packetIdField = std::get<decltype(fwdMsg)::FieldIdx_PacketId>(fwdFields);
    auto& payloadField = std::get<decltype(fwdMsg)::FieldIdx_Payload>(fwdFields);

    packetIdField.value() = msgIdField.value();
    auto& payloadContainer = payloadField.value();
    typedef std::decay<decltype(payloadContainer)>::type ContainerType;
    typedef ContainerType::value_type UnsubString;

    UnsubString unsubStr;
    unsubStr.value() = *topic;
    payloadContainer.push_back(std::move(unsubStr));
    sendToBroker(fwdMsg);
}

void Forward::handle(ConnackMsg&)
{
    auto& st = state();
    if ((m_pubs.empty()) ||
        (st.m_connStatus != ConnectionStatus::Connected) ||
        (!st.m_brokerConnected)) {
        return;
    }

    while (!m_pubs.empty()) {
        auto guard =
            comms::util::makeScopeGuard(
                [this]()
                {
                    m_pubs.pop_front();
                });

        auto& pub = m_pubs.front();
        auto& topic = st.m_regMgr.mapTopicId(pub.m_topicId);
        if (topic.empty()) {
            continue;
        }

        PublishMsg msg;
        auto& fields = msg.fields();
        auto& flagsField = std::get<decltype(msg)::FieldIdx_PublishFlags>(fields);
        auto& flagsMembers = flagsField.value();
        auto& qosField = std::get<mqtt::message::PublishActualFlagIdx_QoS>(flagsMembers);
        auto& topicField = std::get<decltype(msg)::FieldIdx_Topic>(fields);
        auto& payloadField = std::get<decltype(msg)::FieldIdx_Payload>(fields);

        qosField.value() = mqtt::field::QosType::AtMostOnceDelivery;
        topicField.value() = topic;
        payloadField.value() = std::move(pub.m_data);
        msg.refresh();
        sendToBroker(msg);
    }
}

void Forward::handle(PubackMsg& msg)
{
    typedef PubackMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    sendPubackToClient(
        m_lastPubTopicId,
        packetIdField.value(),
        mqttsn::protocol::field::ReturnCodeVal_Accepted);
}

void Forward::handle(PubrecMsg& msg)
{
    typedef PubrecMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    PubrecMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& msgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);

    msgIdField.value() = packetIdField.value();
    sendToClient(respMsg);
}

void Forward::handle(PubcompMsg& msg)
{
    typedef PubcompMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    PubcompMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& msgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);

    msgIdField.value() = packetIdField.value();
    sendToClient(respMsg);
}

void Forward::handle(PingreqMsg& msg)
{
    static_cast<void>(msg);
    if (state().m_connStatus != ConnectionStatus::Connected) {
        return;
    }

    sendToClient(PingreqMsg_SN());
}

void Forward::handle(PingrespMsg& msg)
{
    static_cast<void>(msg);
    if (!m_pingInProgress) {
        return;
    }

    m_pingInProgress = false;
    sendToClient(PingrespMsg_SN());
}

void Forward::handle(SubackMsg& msg)
{
    typedef SubackMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    std::uint16_t msgId = packetIdField.value();
    std::uint16_t topicId = 0U;

    auto iter =
        std::find_if(
            m_subs.begin(), m_subs.end(),
            [msgId](SubsInProgressList::const_reference elem) -> bool
            {
                return elem.m_msgId == msgId;
            });

    if (iter != m_subs.end()) {
        topicId = iter->m_topicId;
        m_subs.erase(iter);
    }

    m_subs.remove_if(
        [this](SubsInProgressList::const_reference elem) -> bool
        {
            return (elem.m_timestamp + state().m_retryPeriod) < state().m_timestamp;
        });

    auto qos = mqttsn::protocol::field::QosType::AtMostOnceDelivery;
    auto rc = mqttsn::protocol::field::ReturnCodeVal_NotSupported;
    do {
        auto& payloadField = std::get<MsgType::FieldIdx_Payload>(fields);
        auto& retCodesList = payloadField.value();
        if (retCodesList.empty()) {
            break;
        }

        auto& ackRetCode = retCodesList.front();
        if (ackRetCode.value() == mqtt::message::SubackReturnCode::Failure) {
            break;
        }

        auto adjustedRetCode = std::min(ackRetCode.value(), mqtt::message::SubackReturnCode::SuccessQos2);
        auto reportedQos = static_cast<mqtt::field::QosType>(adjustedRetCode);
        qos = translateQosForClient(translateQos(reportedQos));
        rc = mqttsn::protocol::field::ReturnCodeVal_Accepted;
    } while (false);

    SubackMsg_SN respMsg;
    auto& respFields = respMsg.fields();
    auto& respFlagsField = std::get<decltype(respMsg)::FieldIdx_flags>(respFields);
    auto& respFlagsMembers = respFlagsField.value();
    auto& respQosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(respFlagsMembers);
    auto& respTopicIdField = std::get<decltype(respMsg)::FieldIdx_topicId>(respFields);
    auto& respMsgIdField = std::get<decltype(respMsg)::FieldIdx_msgId>(respFields);
    auto& respRetCodeField = std::get<decltype(respMsg)::FieldIdx_returnCode>(respFields);

    respQosField.value() = qos;
    respTopicIdField.value() = topicId;
    respMsgIdField.value() = msgId;
    respRetCodeField.value() = rc;
    sendToClient(respMsg);
}

void Forward::handle(UnsubackMsg& msg)
{
    typedef UnsubackMsg MsgType;
    auto& fields = msg.fields();
    auto& packetIdField = std::get<MsgType::FieldIdx_PacketId>(fields);

    UnsubackMsg_SN fwdMsg;
    auto& fwdFields = fwdMsg.fields();
    auto& fwdMsgIdField = std::get<decltype(fwdMsg)::FieldIdx_msgId>(fwdFields);

    fwdMsgIdField.value() = packetIdField.value();
    sendToClient(fwdMsg);
}

void Forward::sendPubackToClient(
    std::uint16_t topicId,
    std::uint16_t msgId,
    mqttsn::protocol::field::ReturnCodeVal rc)
{
    PubackMsg_SN msg;
    auto& fields = msg.fields();
    auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
    auto& retCodeField = std::get<decltype(msg)::FieldIdx_returnCode>(fields);

    topicIdField.value() = topicId;
    msgIdField.value() = msgId;
    retCodeField.value() = rc;
    sendToClient(msg);
}



}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



