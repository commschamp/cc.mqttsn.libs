//
// Copyright 2016 - 2017 (C). Alex Robenko. All rights reserved.
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
    auto& midFlagsField = msg.field_flags().field_midFlags();
    auto& dupFlagsField = msg.field_flags().field_dupFlags();

    typedef typename std::decay<decltype(midFlagsField)>::type MidFlags;
    typedef typename std::decay<decltype(dupFlagsField)>::type DupFlags;

    auto& st = state();

    do {

        if ((msg.field_flags().field_qos().value() != mqttsn::protocol::field::QosType::NoGwPublish) ||
            (st.m_connStatus == ConnectionStatus::Connected)) {
            break;
        }

        if (st.m_connStatus != ConnectionStatus::Disconnected) {
            return;
        }

        NoGwPubInfo info;
        info.m_topicId = msg.field_topicId().value();
        auto& data = msg.field_data().value();
        info.m_data.assign(data.begin(), data.end());
        m_pubs.push_back(std::move(info));
        return;
    } while (false);

    if (st.m_connStatus != ConnectionStatus::Connected) {
        sendPubackToClient(
            msg.field_topicId().value(),
            msg.field_msgId().value(),
            mqttsn::protocol::field::ReturnCodeVal_NotSupported);
        return;
    }

    if (!st.m_brokerConnected) {
        sendPubackToClient(
            msg.field_topicId().value(),
            msg.field_msgId().value(),
            mqttsn::protocol::field::ReturnCodeVal_Congestion);
        return;
    }

    auto& topic = st.m_regMgr.mapTopicId(msg.field_topicId().value());
    if (topic.empty()) {
        sendPubackToClient(
            msg.field_topicId().value(),
            msg.field_msgId().value(),
            mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId);
        sendToBroker(PingreqMsg());
        return;
    }

    bool retain = midFlagsField.getBitValue(MidFlags::BitIdx_retain);
    bool dup = dupFlagsField.getBitValue(DupFlags::BitIdx_bit);
    m_lastPubTopicId = msg.field_topicId().value();

    PublishMsg fwdMsg;
    auto& fwdFlags = fwdMsg.field_publishFlags();

    fwdFlags.field_retain().setBitValue(0, retain);
    fwdFlags.field_qos().value() = translateQosForBroker(translateQos(msg.field_flags().field_qos().value()));
    fwdFlags.field_dup().setBitValue(0, dup);
    fwdMsg.field_topic().value() = topic;
    fwdMsg.field_packetId().field().value() = msg.field_msgId().value();
    auto& data = msg.field_data().value();
    fwdMsg.field_payload().value().assign(data.begin(), data.end());
    fwdMsg.doRefresh();
    sendToBroker(fwdMsg);
}

void Forward::handle(PubrelMsg_SN& msg)
{
    PubrelMsg fwdMsg;
    fwdMsg.field_packetId().value() = msg.field_msgId().value();
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
    auto sendSubackFunc =
        [this, &msg](mqttsn::protocol::field::ReturnCodeVal rc)
        {
            SubackMsg_SN respMsg;
            respMsg.field_flags().field_qos().value() = msg.field_flags().field_qos().value();
            respMsg.field_topicId().value() = msg.field_topicId().field().value();
            respMsg.field_msgId().value() = msg.field_msgId().value();
            respMsg.field_returnCode().value() = rc;
            sendToClient(respMsg);
        };

    if (state().m_connStatus != ConnectionStatus::Connected) {
        sendSubackFunc(mqttsn::protocol::field::ReturnCodeVal_NotSupported);
        return;
    }

    std::string topic;
    std::uint16_t topicId = 0U;
    do {
        if (msg.field_topicName().doesExist()) {
            assert(msg.field_topicId().isMissing());
            auto& topicStorage = msg.field_topicName().field().value();
            topic.assign(topicStorage.begin(), topicStorage.end());

            if (topic.empty()) {
                sendSubackFunc(mqttsn::protocol::field::ReturnCodeVal_NotSupported);
                sendToBroker(PingreqMsg());
                return;
            }

            bool hasWildcards =
                std::any_of(topic.begin(), topic.end(),
                    [](char ch) -> bool
                    {
                        return (ch == '#') || (ch == '+');
                    });

            if (hasWildcards) {
                break;
            }

            topicId = state().m_regMgr.mapTopicNoInfo(topic);
            break;
        }

        assert(msg.field_topicId().doesExist());
        assert(msg.field_topicName().isMissing());

        auto& topicStr = state().m_regMgr.mapTopicId(msg.field_topicId().field().value());
        if (!topicStr.empty()) {
            topic = topicStr;
            topicId = msg.field_topicId().field().value();
            break;
        }

        sendSubackFunc(mqttsn::protocol::field::ReturnCodeVal_InvalidTopicId);
        sendToBroker(PingreqMsg());
        return;
    } while (false);

    SubInfo info;
    info.m_timestamp = state().m_timestamp;
    info.m_msgId = msg.field_msgId().value();
    info.m_topicId = topicId;
    m_subs.push_back(info);

    SubscribeMsg fwdMsg;
    fwdMsg.field_packetId().value() = msg.field_msgId().value();
    auto& payloadContainer = fwdMsg.field_payload().value();
    typedef std::decay<decltype(payloadContainer)>::type ContainerType;
    typedef ContainerType::value_type SubElemBundle;

    SubElemBundle subElem;
    auto& subMembers = subElem.value();
    auto& subTopicField = std::get<0>(subMembers);
    auto& subQosField = std::get<1>(subMembers);

    assert(!topic.empty());
    subTopicField.value() = std::move(topic);
    subQosField.value() = translateQosForBroker(translateQos(msg.field_flags().field_qos().value()));

    payloadContainer.push_back(std::move(subElem));
    sendToBroker(fwdMsg);
}

void Forward::handle(UnsubscribeMsg_SN& msg)
{
    if (state().m_connStatus != ConnectionStatus::Connected) {
        return;
    }

    std::string topic;
    do {
        if (msg.field_topicName().doesExist()) {
            assert(msg.field_topicId().isMissing());
            auto& topicStorage = msg.field_topicName().field().value();
            topic.assign(topicStorage.begin(), topicStorage.end());

            if (topic.empty()) {
                return;
            }

            break;
        }

        assert(msg.field_topicId().doesExist());
        assert(msg.field_topicName().isMissing());

        auto& topicStr = state().m_regMgr.mapTopicId(msg.field_topicId().field().value());
        if (topicStr.empty()) {
            return;
        }
        topic = topicStr;
    } while (false);

    UnsubscribeMsg fwdMsg;
    fwdMsg.field_packetId().value() = msg.field_msgId().value();
    auto& payloadContainer = fwdMsg.field_payload().value();
    typedef std::decay<decltype(payloadContainer)>::type ContainerType;
    typedef ContainerType::value_type UnsubString;

    UnsubString unsubStr;
    unsubStr.value() = std::move(topic);
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
        auto& flags = msg.field_publishFlags();

        flags.field_qos().value() = mqtt::protocol::common::field::QosVal::AtMostOnceDelivery;
        msg.field_topic().value() = topic;
        msg.field_payload().value() = std::move(pub.m_data);
        msg.doRefresh();
        sendToBroker(msg);
    }
}

void Forward::handle(PubackMsg& msg)
{
    sendPubackToClient(
        m_lastPubTopicId,
        msg.field_packetId().value(),
        mqttsn::protocol::field::ReturnCodeVal_Accepted);
}

void Forward::handle(PubrecMsg& msg)
{
    PubrecMsg_SN respMsg;
    respMsg.field_msgId().value() = msg.field_packetId().value();
    sendToClient(respMsg);
}

void Forward::handle(PubcompMsg& msg)
{
    PubcompMsg_SN respMsg;
    respMsg.field_msgId().value() = msg.field_packetId().value();
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
    std::uint16_t msgId = msg.field_packetId().value();
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
        auto& retCodesList = msg.field_payload().value();
        if (retCodesList.empty()) {
            break;
        }

        auto& ackRetCode = retCodesList.front();
        if (ackRetCode.value() == mqtt::protocol::v311::field::SubackReturnCodeVal::Failure) {
            break;
        }

        auto adjustedRetCode = std::min(ackRetCode.value(), mqtt::protocol::v311::field::SubackReturnCodeVal::SuccessQos2);
        auto reportedQos = static_cast<mqtt::protocol::common::field::QosVal>(adjustedRetCode);
        qos = translateQosForClient(translateQos(reportedQos));
        rc = mqttsn::protocol::field::ReturnCodeVal_Accepted;
    } while (false);

    SubackMsg_SN respMsg;
    respMsg.field_flags().field_qos().value() = qos;
    respMsg.field_topicId().value() = topicId;
    respMsg.field_msgId().value() = msgId;
    respMsg.field_returnCode().value() = rc;
    sendToClient(respMsg);
}

void Forward::handle(UnsubackMsg& msg)
{
    UnsubackMsg_SN fwdMsg;
    fwdMsg.field_msgId().value() = msg.field_packetId().value();
    sendToClient(fwdMsg);
}

void Forward::sendPubackToClient(
    std::uint16_t topicId,
    std::uint16_t msgId,
    mqttsn::protocol::field::ReturnCodeVal rc)
{
    PubackMsg_SN msg;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_returnCode().value() = rc;
    sendToClient(msg);
}

}  // namespace session_op

}  // namespace gateway

}  // namespace mqttsn



