//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "TestMsgHandler.h"

#include <type_traits>
#include <iostream>
#include <cassert>
#include <iterator>

namespace
{

bool isShortTopicName(const std::string& topic)
{
    return
        (topic.size() == 2U) &&
        (topic.find_first_of("#+") == std::string::npos);
}

std::uint16_t shortTopicNameToId(const std::string& topic)
{
    assert(isShortTopicName(topic));
    return
        static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(topic[0]) << 8) | static_cast<std::uint8_t>(topic[1]));
}

} // namespace

template <typename TStack>
void TestMsgHandler::processOutputInternal(TStack& stack, const DataBuf& data)
{
    typedef typename std::decay<decltype(stack)>::type StackType;
    typedef typename StackType::MsgPtr MsgPtr;
    typedef typename MsgPtr::element_type MsgType;

    auto iter = comms::readIteratorFor<MsgType>(&data[0]);
    MsgPtr msg;

    auto es = stack.read(msg, iter, data.size());
    static_cast<void>(es);
    if (es != comms::ErrorStatus::Success) {
        std::cout << "es=" << static_cast<unsigned>(es) << ": Output buffer: " << std::hex;
        std::copy(data.begin(), data.end(), std::ostream_iterator<unsigned>(std::cout, " "));
        std::cout << std::dec << std::endl;
    }
    assert(es == comms::ErrorStatus::Success);
    assert(msg);
    assert(static_cast<unsigned>(std::distance(comms::readIteratorFor<MsgType>(&data[0]), iter)) == data.size());
    msg->dispatch(*this);
}


template <typename TStack, typename TMsg>
TestMsgHandler::DataBuf TestMsgHandler::prepareInputInternal(TStack& stack, const TMsg& msg)
{
    typedef typename std::decay<decltype(stack)>::type StackType;
    typedef typename StackType::MsgPtr::element_type MsgType;

    DataBuf buf;
    buf.resize(stack.length(msg));

    auto iter = comms::writeIteratorFor<MsgType>(&buf[0]);
    auto es = stack.write(msg, iter, buf.size());
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    assert(buf.size() == static_cast<unsigned>(std::distance(comms::writeIteratorFor<MsgType>(&buf[0]), iter)));
    return buf;
}

TestMsgHandler::TestMsgHandler() = default;
TestMsgHandler::~TestMsgHandler() = default;

TestMsgHandler::GwinfoMsgHandlerFunc
TestMsgHandler::setGwinfoMsgHandler(GwinfoMsgHandlerFunc&& func)
{
    GwinfoMsgHandlerFunc old(std::move(m_gwInfoMsgHandler));
    m_gwInfoMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::ConnackMsgHandlerFunc TestMsgHandler::setConnackMsgHandler(
    ConnackMsgHandlerFunc&& func)
{
    ConnackMsgHandlerFunc old(std::move(m_connackMsgHandler));
    m_connackMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::WilltopicreqMsgHandlerFunc
TestMsgHandler::setWilltopicreqMsgHandler(WilltopicreqMsgHandlerFunc&& func)
{
    WilltopicreqMsgHandlerFunc old(std::move(m_willtopicreqMsgHandler));
    m_willtopicreqMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::WillmsgreqMsgHandlerFunc
TestMsgHandler::setWillmsgreqMsgHandler(WillmsgreqMsgHandlerFunc&& func)
{
    WillmsgreqMsgHandlerFunc old(std::move(m_willmsgreqMsgHandler));
    m_willmsgreqMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::DisconnectSnMsgHandlerFunc TestMsgHandler::setDisconnectSnMsgHandler(
    DisconnectSnMsgHandlerFunc&& func)
{
    DisconnectSnMsgHandlerFunc old(std::move(m_disconnectSnMsgHandler));
    m_disconnectSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::RegisterMsgHandlerFunc TestMsgHandler::setRegisterMsgHandler(
    RegisterMsgHandlerFunc&& func)
{
    RegisterMsgHandlerFunc old(std::move(m_registerMsgHandler));
    m_registerMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::RegackMsgHandlerFunc TestMsgHandler::setRegackMsgHandler(
    RegackMsgHandlerFunc&& func)
{
    RegackMsgHandlerFunc old(std::move(m_regackMsgHandler));
    m_regackMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PublishSnMsgHandlerFunc TestMsgHandler::setPublishSnMsgHandler(
    PublishSnMsgHandlerFunc&& func)
{
    PublishSnMsgHandlerFunc old(std::move(m_publishSnMsgHandler));
    m_publishSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubackSnMsgHandlerFunc TestMsgHandler::setPubackSnMsgHandler(
    PubackSnMsgHandlerFunc&& func)
{
    PubackSnMsgHandlerFunc old(std::move(m_pubackSnMsgHandler));
    m_pubackSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubrecSnMsgHandlerFunc TestMsgHandler::setPubrecSnMsgHandler(
    PubrecSnMsgHandlerFunc&& func)
{
    PubrecSnMsgHandlerFunc old(std::move(m_pubrecSnMsgHandler));
    m_pubrecSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubrelSnMsgHandlerFunc TestMsgHandler::setPubrelSnMsgHandler(
    PubrelSnMsgHandlerFunc&& func)
{
    PubrelSnMsgHandlerFunc old(std::move(m_pubrelSnMsgHandler));
    m_pubrelSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubcompSnMsgHandlerFunc TestMsgHandler::setPubcompSnMsgHandler(
    PubcompSnMsgHandlerFunc&& func)
{
    PubcompSnMsgHandlerFunc old(std::move(m_pubcompSnMsgHandler));
    m_pubcompSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PingreqSnMsgHandlerFunc TestMsgHandler::setPingreqSnMsgHandler(
    PingreqSnMsgHandlerFunc&& func)
{
    PingreqSnMsgHandlerFunc old(std::move(m_pingreqSnMsgHandler));
    m_pingreqSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PingrespSnMsgHandlerFunc TestMsgHandler::setPingrespSnMsgHandler(
    PingrespSnMsgHandlerFunc&& func)
{
    PingrespSnMsgHandlerFunc old(std::move(m_pingrespSnMsgHandler));
    m_pingrespSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::SubackSnMsgHandlerFunc TestMsgHandler::setSubackSnMsgHandler(
    SubackSnMsgHandlerFunc&& func)
{
    SubackSnMsgHandlerFunc old(std::move(m_subackSnMsgHandler));
    m_subackSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::UnsubackSnMsgHandlerFunc TestMsgHandler::setUnsubackSnMsgHandler(
    UnsubackSnMsgHandlerFunc&& func)
{
    UnsubackSnMsgHandlerFunc old(std::move(m_unsubackSnMsgHandler));
    m_unsubackSnMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::WilltopicrespMsgHandlerFunc TestMsgHandler::setWilltopicrespMsgHandler(
    WilltopicrespMsgHandlerFunc&& func)
{
    WilltopicrespMsgHandlerFunc old(std::move(m_willtopicrespMsgHandler));
    m_willtopicrespMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::WillmsgrespMsgHandlerFunc TestMsgHandler::setWillmsgrespMsgHandler(
    WillmsgrespMsgHandlerFunc&& func)
{
    WillmsgrespMsgHandlerFunc old(std::move(m_willmsgrespMsgHandler));
    m_willmsgrespMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::ConnectMsgHandlerFunc
TestMsgHandler::setConnectMsgHandler(ConnectMsgHandlerFunc&& func)
{
    ConnectMsgHandlerFunc old(std::move(m_connectMsgHandler));
    m_connectMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::DisconnectMsgHandlerFunc
TestMsgHandler::setDisconnectMsgHandler(DisconnectMsgHandlerFunc&& func)
{
    DisconnectMsgHandlerFunc old(std::move(m_disconnectMsgHandler));
    m_disconnectMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PingreqMsgHandlerFunc
TestMsgHandler::setPingreqMsgHandler(PingreqMsgHandlerFunc&& func)
{
    PingreqMsgHandlerFunc old(std::move(m_pingreqMsgHandler));
    m_pingreqMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PingrespMsgHandlerFunc
TestMsgHandler::setPingrespMsgHandler(PingrespMsgHandlerFunc&& func)
{
    PingrespMsgHandlerFunc old(std::move(m_pingrespMsgHandler));
    m_pingrespMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PublishMsgHandlerFunc
TestMsgHandler::setPublishMsgHandler(PublishMsgHandlerFunc&& func)
{
    PublishMsgHandlerFunc old(std::move(m_publishMsgHandler));
    m_publishMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubackMsgHandlerFunc
TestMsgHandler::setPubackMsgHandler(PubackMsgHandlerFunc&& func)
{
    PubackMsgHandlerFunc old(std::move(m_pubackMsgHandler));
    m_pubackMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubrecMsgHandlerFunc
TestMsgHandler::setPubrecMsgHandler(PubrecMsgHandlerFunc&& func)
{
    PubrecMsgHandlerFunc old(std::move(m_pubrecMsgHandler));
    m_pubrecMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubrelMsgHandlerFunc
TestMsgHandler::setPubrelMsgHandler(PubrelMsgHandlerFunc&& func)
{
    PubrelMsgHandlerFunc old(std::move(m_pubrelMsgHandler));
    m_pubrelMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::PubcompMsgHandlerFunc
TestMsgHandler::setPubcompMsgHandler(PubcompMsgHandlerFunc&& func)
{
    PubcompMsgHandlerFunc old(std::move(m_pubcompMsgHandler));
    m_pubcompMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::SubscribeMsgHandlerFunc
TestMsgHandler::setSubscribeMsgHandler(SubscribeMsgHandlerFunc&& func)
{
    SubscribeMsgHandlerFunc old(std::move(m_subscribeMsgHandler));
    m_subscribeMsgHandler = std::move(func);
    return old;
}

TestMsgHandler::UnsubscribeMsgHandlerFunc
TestMsgHandler::setUnsubscribeMsgHandler(UnsubscribeMsgHandlerFunc&& func)
{
    UnsubscribeMsgHandlerFunc old(std::move(m_unsubscribeMsgHandler));
    m_unsubscribeMsgHandler = std::move(func);
    return old;
}

void TestMsgHandler::handle(GwinfoMsg_SN& msg)
{
    assert(m_gwInfoMsgHandler);
    m_gwInfoMsgHandler(msg);
}

void TestMsgHandler::handle(ConnackMsg_SN& msg)
{
    assert(m_connackMsgHandler);
    m_connackMsgHandler(msg);
}

void TestMsgHandler::handle(WilltopicreqMsg_SN& msg)
{
    assert(m_willtopicreqMsgHandler);
    m_willtopicreqMsgHandler(msg);
}

void TestMsgHandler::handle(WillmsgreqMsg_SN& msg)
{
    assert(m_willmsgreqMsgHandler);
    m_willmsgreqMsgHandler(msg);
}

void TestMsgHandler::handle(DisconnectMsg_SN& msg)
{
    assert(m_disconnectSnMsgHandler);
    m_disconnectSnMsgHandler(msg);
}

void TestMsgHandler::handle(RegisterMsg_SN& msg)
{
    assert(m_registerMsgHandler);
    m_registerMsgHandler(msg);
}

void TestMsgHandler::handle(RegackMsg_SN& msg)
{
    assert(m_regackMsgHandler);
    m_regackMsgHandler(msg);
}

void TestMsgHandler::handle(PublishMsg_SN& msg)
{
    assert(m_publishSnMsgHandler);
    m_publishSnMsgHandler(msg);
}

void TestMsgHandler::handle(PubackMsg_SN& msg)
{
    assert(m_pubackSnMsgHandler);
    m_pubackSnMsgHandler(msg);
}

void TestMsgHandler::handle(PubrecMsg_SN& msg)
{
    assert(m_pubrecSnMsgHandler);
    m_pubrecSnMsgHandler(msg);
}

void TestMsgHandler::handle(PubrelMsg_SN& msg)
{
    assert(m_pubrelSnMsgHandler);
    m_pubrelSnMsgHandler(msg);
}

void TestMsgHandler::handle(PubcompMsg_SN& msg)
{
    assert(m_pubcompSnMsgHandler);
    m_pubcompSnMsgHandler(msg);
}

void TestMsgHandler::handle(PingreqMsg_SN& msg)
{
    assert(m_pingreqSnMsgHandler);
    m_pingreqSnMsgHandler(msg);
}

void TestMsgHandler::handle(PingrespMsg_SN& msg)
{
    assert(m_pingrespSnMsgHandler);
    m_pingrespSnMsgHandler(msg);
}

void TestMsgHandler::handle(SubackMsg_SN& msg)
{
    assert(m_subackSnMsgHandler);
    m_subackSnMsgHandler(msg);
}

void TestMsgHandler::handle(UnsubackMsg_SN& msg)
{
    assert(m_unsubackSnMsgHandler);
    m_unsubackSnMsgHandler(msg);
}

void TestMsgHandler::handle(WilltopicrespMsg_SN& msg)
{
    assert(m_willtopicrespMsgHandler);
    m_willtopicrespMsgHandler(msg);
}

void TestMsgHandler::handle(WillmsgrespMsg_SN& msg)
{
    assert(m_willmsgrespMsgHandler);
    m_willmsgrespMsgHandler(msg);
}

void TestMsgHandler::handle(TestMqttsnMessage& msg)
{
    std::cout << "Unhandled message sent to client: " << static_cast<unsigned>(msg.getId()) << std::endl;
    constexpr bool Unhandled_message = false;
    static_cast<void>(Unhandled_message);
    assert(Unhandled_message);
}

void TestMsgHandler::handle(ConnectMsg& msg)
{
    assert(m_connectMsgHandler);
    m_connectMsgHandler(msg);
}

void TestMsgHandler::handle(DisconnectMsg& msg)
{
    assert(m_disconnectMsgHandler);
    m_disconnectMsgHandler(msg);
}

void TestMsgHandler::handle(PingreqMsg& msg)
{
    assert(m_pingreqMsgHandler);
    m_pingreqMsgHandler(msg);
}

void TestMsgHandler::handle(PingrespMsg& msg)
{
    assert(m_pingrespMsgHandler);
    m_pingrespMsgHandler(msg);
}

void TestMsgHandler::handle(PublishMsg& msg)
{
    assert(m_publishMsgHandler);
    m_publishMsgHandler(msg);
}

void TestMsgHandler::handle(PubackMsg& msg)
{
    assert(m_pubackMsgHandler);
    m_pubackMsgHandler(msg);
}

void TestMsgHandler::handle(PubrecMsg& msg)
{
    assert(m_pubrecMsgHandler);
    m_pubrecMsgHandler(msg);
}

void TestMsgHandler::handle(PubrelMsg& msg)
{
    assert(m_pubrelMsgHandler);
    m_pubrelMsgHandler(msg);
}

void TestMsgHandler::handle(PubcompMsg& msg)
{
    assert(m_pubcompMsgHandler);
    m_pubcompMsgHandler(msg);
}

void TestMsgHandler::handle(SubscribeMsg& msg)
{
    assert(m_subscribeMsgHandler);
    m_subscribeMsgHandler(msg);
}

void TestMsgHandler::handle(UnsubscribeMsg& msg)
{
    assert(m_unsubscribeMsgHandler);
    m_unsubscribeMsgHandler(msg);
}

void TestMsgHandler::handle(TestMqttMessage& msg)
{
    std::cout << "Unhandled message sent to broker: " << static_cast<unsigned>(msg.getId()) << std::endl;
    constexpr bool Unhandled_message = false;
    static_cast<void>(Unhandled_message);
    assert(Unhandled_message);
}

void TestMsgHandler::processDataForClient(const DataBuf& data)
{
    processOutputInternal(m_mqttsnStack, data);
}

void TestMsgHandler::processDataForBroker(const DataBuf& data)
{
    processOutputInternal(m_mqttStack, data);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareInput(const TestMqttsnMessage& msg)
{
    return prepareInputInternal(m_mqttsnStack, msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareInput(const TestMqttMessage& msg)
{
    return prepareInputInternal(m_mqttStack, msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareSearchgw(std::uint8_t radius)
{
    SearchgwMsg_SN msg;
    msg.field_radius().value() = radius;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientConnect(
    const std::string& id,
    std::uint16_t keepAlive,
    bool hasWill,
    bool clean)
{
    ConnectMsg_SN msg;
    msg.field_clientId().value() = id;
    msg.field_duration().value() = keepAlive;
    msg.field_flags().field_mid().setBitValue_Will(hasWill);
    msg.field_flags().field_mid().setBitValue_CleanSession(clean);
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientWilltopic(
    const std::string& topic,
    cc_mqttsn::field::QosVal qos,
    bool retain)
{
    WilltopicMsg_SN msg;
    msg.field_flags().field().field_qos().value() = qos;
    msg.field_flags().field().field_mid().setBitValue_Retain(retain);
    msg.field_willTopic().value() = topic;
    msg.doRefresh();
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientWillmsg(const DataBuf& data)
{
    WillmsgMsg_SN msg;
    msg.field_willMsg().value() = data;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientDisconnect(std::uint16_t duration)
{
    DisconnectMsg_SN msg;
    msg.field_duration().field().value() = duration;
    auto mode = comms::field::OptionalMode::Missing;
    if (duration != 0) {
        mode = comms::field::OptionalMode::Exists;
    }
    msg.field_duration().setMode(mode);
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientRegister(
    const std::string& topic,
    std::uint16_t msgId)
{
    RegisterMsg_SN msg;
    msg.field_topicName().value() = topic;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientRegack(
    std::uint16_t topicId,
    std::uint16_t msgId,
    cc_mqttsn::field::ReturnCodeVal rc)
{
    RegackMsg_SN msg;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_returnCode().value() = rc;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientPublish(
    const DataBuf& data,
    std::uint16_t topicId,
    std::uint16_t msgId,
    TopicIdTypeVal topicIdType,
    cc_mqttsn::field::QosVal qos,
    bool retain,
    bool dup)
{
    PublishMsg_SN msg;
    msg.field_flags().field_topicIdType().value() = topicIdType;
    msg.field_flags().field_mid().setBitValue_Retain(retain);
    msg.field_flags().field_qos().value() = qos;
    msg.field_flags().field_high().setBitValue_Dup(dup);
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_data().value() = data;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientPuback(
    std::uint16_t topicId,
    std::uint16_t msgId,
    cc_mqttsn::field::ReturnCodeVal rc)
{
    PubackMsg_SN msg;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_returnCode().value() = rc;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientPubrec(std::uint16_t msgId)
{
    PubrecMsg_SN msg;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientPubrel(std::uint16_t msgId)
{
    PubrelMsg_SN msg;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientPubcomp(std::uint16_t msgId)
{
    PubcompMsg_SN msg;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientPingreq(
    const std::string& clientId)
{
    PingreqMsg_SN msg;
    msg.field_clientId().value() = clientId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientPingresp()
{
    return prepareInput(PingrespMsg_SN());
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientSubscribe(
    std::uint16_t topicId,
    std::uint16_t msgId,
    cc_mqttsn::field::QosVal qos)
{
    SubscribeMsg_SN msg;

    msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::PredefinedTopicId;
    msg.field_flags().field_qos().value() = qos;
    msg.field_msgId().value() = msgId;
    msg.field_topicId().field().value() = topicId;
    msg.doRefresh();
    assert(msg.field_topicId().doesExist());
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientSubscribe(
    const std::string& topic,
    std::uint16_t msgId,
    cc_mqttsn::field::QosVal qos)
{
    SubscribeMsg_SN msg;
    msg.field_flags().field_qos().value() = qos;

    bool shortName = isShortTopicName(topic);
    if (shortName) {
        msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::ShortTopicName;
        msg.field_topicId().field().value() = shortTopicNameToId(topic);
    }
    else {
        msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::Normal;
        msg.field_topicName().field().value() = topic;
    }

    msg.field_msgId().value() = msgId;
    msg.doRefresh();
    assert(shortName || msg.field_topicName().doesExist());
    assert((!shortName) || msg.field_topicId().doesExist());
    assert(msg.field_topicName().doesExist() || msg.field_topicId().doesExist());
    assert(msg.field_topicName().isMissing() || msg.field_topicId().isMissing());

    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientUnsubscribe(
    std::uint16_t topicId,
    std::uint16_t msgId)
{
    UnsubscribeMsg_SN msg;

    msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::PredefinedTopicId;
    msg.field_msgId().value() = msgId;
    msg.field_topicId().field().value() = topicId;
    msg.doRefresh();
    assert(msg.field_topicId().doesExist());
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientUnsubscribe(
    const std::string& topic,
    std::uint16_t msgId)
{
    UnsubscribeMsg_SN msg;
    bool shortName = isShortTopicName(topic);
    if (shortName) {
        msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::ShortTopicName;
        msg.field_topicId().field().value() = shortTopicNameToId(topic);
    }
    else {
        msg.field_flags().field_topicIdType().value() = TopicIdTypeVal::Normal;
        msg.field_topicName().field().value() = topic;
    }

    msg.field_msgId().value() = msgId;
    msg.doRefresh();
    assert(shortName || msg.field_topicName().doesExist());
    assert((!shortName) || msg.field_topicId().doesExist());
    assert(msg.field_topicName().doesExist() || msg.field_topicId().doesExist());
    assert(msg.field_topicName().isMissing() || msg.field_topicId().isMissing());
   return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientWilltopicupd(
    const std::string& topic,
    cc_mqttsn::field::QosVal qos,
    bool retain)
{
    WilltopicupdMsg_SN msg;
    msg.field_flags().field().field_qos().value() = qos;
    msg.field_flags().field().field_mid().setBitValue_Retain(retain);
    msg.field_willTopic().value() = topic;
    msg.doRefresh();
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareClientWillmsgupd(const DataBuf& data)
{
    WillmsgupdMsg_SN msg;
    msg.field_willMsg().value() = data;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerConnack(
    ConnackResponseCodeVal rc,
    bool sessionPresent)
{
    ConnackMsg msg;
    msg.field_flags().setBitValue(0, sessionPresent);
    msg.field_returnCode().value() = rc;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerDisconnect()
{
    return prepareInput(DisconnectMsg());
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerPingreq()
{
    return prepareInput(PingreqMsg());
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerPingresp()
{
    return prepareInput(PingrespMsg());
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerPublish(
    const std::string& topic,
    const DataBuf& msgData,
    std::uint16_t packetId,
    cc_mqtt311::field::QosVal qos,
    bool retain,
    bool duplicate)
{
    PublishMsg msg;
    auto& flags = msg.transportField_flags();

    msg.field_topic().value() = topic;
    msg.field_payload().value() = msgData;
    msg.field_packetId().field().value() = packetId;
    flags.field_qos().value() = qos;
    flags.field_retain().setBitValue_bit(retain);
    flags.field_dup().setBitValue_bit(duplicate);

    msg.doRefresh();
    assert((qos == cc_mqtt311::field::QosVal::AtMostOnceDelivery) ||
           (msg.field_packetId().doesExist()));
    assert((cc_mqtt311::field::QosVal::AtMostOnceDelivery < qos) ||
           (msg.field_packetId().isMissing()));
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerPuback(std::uint16_t packetId)
{
    PubackMsg msg;
    msg.field_packetId().value() = packetId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerPubrec(std::uint16_t packetId)
{
    PubrecMsg msg;
    msg.field_packetId().value() = packetId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerPubrel(std::uint16_t packetId)
{
    PubrelMsg msg;
    msg.field_packetId().value() = packetId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerPubcomp(std::uint16_t packetId)
{
    PubcompMsg msg;
    msg.field_packetId().value() = packetId;
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerSuback(
    std::uint16_t packetId,
    SubackReturnCodeVal rc)
{
    SubackMsg msg;

    msg.field_packetId().value() = packetId;
    using RcElemType = SubackMsg::Field_list::ValueType::value_type;
    RcElemType rcElem;
    rcElem.value() = rc;
    msg.field_list().value().push_back(rcElem);
    return prepareInput(msg);
}

TestMsgHandler::DataBuf TestMsgHandler::prepareBrokerUnsuback(
    std::uint16_t packetId)
{
    UnsubackMsg msg;
    msg.field_packetId().value() = packetId;
    return prepareInput(msg);
}


