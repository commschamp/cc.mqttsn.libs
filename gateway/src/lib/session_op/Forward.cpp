//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Forward.h"

#include <cassert>
#include <algorithm>

#include "comms/util/ScopeGuard.h"

namespace cc_mqttsn_gateway
{

namespace session_op
{

Forward::Forward(SessionImpl& session) :
    Base(session)
{
}

Forward::~Forward() = default;

void Forward::handle(PublishMsg_SN& msg)
{
    auto& st = state();

    do {

        if ((msg.field_flags().field_qos().value() != cc_mqttsn::field::QosVal::NoGwPublish) ||
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
            ReturnCodeVal::NotSupported);
        return;
    }

    if (!st.m_brokerConnected) {
        sendPubackToClient(
            msg.field_topicId().value(),
            msg.field_msgId().value(),
            ReturnCodeVal::Congestion);
        return;
    }

    auto& topic = st.m_regMgr.mapTopicId(msg.field_topicId().value());
    if (topic.empty()) {
        sendPubackToClient(
            msg.field_topicId().value(),
            msg.field_msgId().value(),
            ReturnCodeVal::InvalidTopicId);
        sendToBroker(PingreqMsg());
        return;
    }

    bool retain = msg.field_flags().field_mid().getBitValue_Retain();
    bool dup = msg.field_flags().field_high().getBitValue_Dup();
    m_lastPubTopicId = msg.field_topicId().value();

    PublishMsg fwdMsg;
    auto& fwdFlags = fwdMsg.transportField_flags();

    fwdFlags.field_retain().setBitValue_bit(retain);
    fwdFlags.field_qos().value() = translateQosForBroker(translateQos(msg.field_flags().field_qos().value()));
    fwdFlags.field_dup().setBitValue_bit(dup);
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

void Forward::handle([[maybe_unused]] PingreqMsg_SN& msg)
{
    if (state().m_connStatus != ConnectionStatus::Connected) {
        return;
    }

    m_pingInProgress = true;
    sendToBroker(PingreqMsg());
}

void Forward::handle([[maybe_unused]] PingrespMsg_SN& msg)
{
    sendToBroker(PingrespMsg());
}

void Forward::handle(SubscribeMsg_SN& msg)
{
    auto sendSubackFunc =
        [this, &msg](ReturnCodeVal rc)
        {
            SubackMsg_SN respMsg;
            respMsg.field_flags().field_qos().value() = msg.field_flags().field_qos().value();
            respMsg.field_topicId().value() = msg.field_topicId().field().value();
            respMsg.field_msgId().value() = msg.field_msgId().value();
            respMsg.field_returnCode().value() = rc;
            sendToClient(respMsg);
        };

    if (state().m_connStatus != ConnectionStatus::Connected) {
        sendSubackFunc(ReturnCodeVal::NotSupported);
        return;
    }

    std::string topic;
    std::uint16_t topicId = 0U;

    do {
        if (msg.field_topicName().doesExist()) {
            assert(msg.field_topicId().isMissing());
            assert(msg.field_flags().field_topicIdType().value() == TopicIdTypeVal::Normal);
            auto& topicStorage = msg.field_topicName().field().value();
            topic.assign(topicStorage.begin(), topicStorage.end());

            if (topic.empty()) {
                sendSubackFunc(ReturnCodeVal::NotSupported);
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

        if (msg.field_flags().field_topicIdType().value() == TopicIdTypeVal::ShortTopicName) {
            assert(msg.field_topicId().doesExist());
            auto firstChar = static_cast<char>((msg.field_topicId().field().value() >> 8) & 0xff);
            auto secondChar = static_cast<char>(msg.field_topicId().field().value() & 0xff);
            topic.push_back(firstChar);
            topic.push_back(secondChar);
            topicId = msg.field_topicId().field().value();
            break;
        }

        if (msg.field_flags().field_topicIdType().value() != TopicIdTypeVal::PredefinedTopicId) {
            sendSubackFunc(ReturnCodeVal::NotSupported);
            sendToBroker(PingreqMsg());
            return;
        }

        assert(msg.field_topicId().doesExist());
        assert(msg.field_topicName().isMissing());

        auto& topicStr = state().m_regMgr.mapTopicId(msg.field_topicId().field().value());
        if (!topicStr.empty()) {
            topic = topicStr;
            topicId = msg.field_topicId().field().value();
            break;
        }

        sendSubackFunc(ReturnCodeVal::InvalidTopicId);
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
    auto& payloadContainer = fwdMsg.field_list().value();
    typedef std::decay<decltype(payloadContainer)>::type ContainerType;
    typedef ContainerType::value_type SubElemBundle;

    SubElemBundle subElem;
    assert(!topic.empty());
    subElem.field_topic().value() = std::move(topic);
    subElem.field_qos().value() =
            translateQosForBroker(translateQos(msg.field_flags().field_qos().value()));

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

        if (msg.field_flags().field_topicIdType().value() == TopicIdTypeVal::ShortTopicName) {
            assert(msg.field_topicId().doesExist());
            auto firstChar = static_cast<char>((msg.field_topicId().field().value() >> 8) & 0xff);
            auto secondChar = static_cast<char>(msg.field_topicId().field().value() & 0xff);
            topic.push_back(firstChar);
            topic.push_back(secondChar);
            break;
        }

        auto& topicStr = state().m_regMgr.mapTopicId(msg.field_topicId().field().value());
        if (topicStr.empty()) {
            return;
        }
        topic = topicStr;
    } while (false);

    UnsubscribeMsg fwdMsg;
    fwdMsg.field_packetId().value() = msg.field_msgId().value();
    auto& payloadContainer = fwdMsg.field_list().value();
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
        auto& flags = msg.transportField_flags();

        flags.field_qos().value() = PublishMsg::TransportField_flags::Field_qos::ValueType::AtMostOnceDelivery;
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
        ReturnCodeVal::Accepted);
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

void Forward::handle([[maybe_unused]] PingrespMsg& msg)
{
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

    auto qos = cc_mqttsn::field::QosVal::AtMostOnceDelivery;
    auto rc = ReturnCodeVal::NotSupported;
    do {
        auto& retCodesList = msg.field_list().value();
        if (retCodesList.empty()) {
            break;
        }

        auto& ackRetCode = retCodesList.front();
        using SubackRetCode = SubackMsg::Field_list::ValueType::value_type::ValueType;
        if (ackRetCode.value() == SubackRetCode::Failure) {
            break;
        }

        auto adjustedRetCode = std::min(ackRetCode.value(), SubackRetCode::Qos2);
        auto reportedQos = static_cast<cc_mqtt311::field::QosVal>(adjustedRetCode);
        qos = translateQosForClient(translateQos(reportedQos));
        rc = ReturnCodeVal::Accepted;
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
    ReturnCodeVal rc)
{
    PubackMsg_SN msg;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_returnCode().value() = rc;
    sendToClient(msg);
}

}  // namespace session_op

}  // namespace cc_mqttsn_gateway
