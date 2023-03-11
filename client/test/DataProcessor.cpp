//
// Copyright 2016 - 2023 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "DataProcessor.h"

#include <iostream>

DataProcessor::~DataProcessor() = default;

DataProcessor::SearchgwMsgReportCallback DataProcessor::setSearchgwMsgReportCallback(SearchgwMsgReportCallback&& func)
{
    SearchgwMsgReportCallback old(std::move(m_searchgwMsgReportCallback));
    m_searchgwMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::ConnectMsgReportCallback DataProcessor::setConnectMsgReportCallback(ConnectMsgReportCallback&& func)
{
    ConnectMsgReportCallback old(std::move(m_connectMsgReportCallback));
    m_connectMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::WilltopicMsgReportCallback DataProcessor::setWilltopicMsgReportCallback(
    WilltopicMsgReportCallback&& func)
{
    WilltopicMsgReportCallback old = std::move(m_willtopicMsgReportCallback);
    m_willtopicMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::WillmsgMsgReportCallback DataProcessor::setWillmsgMsgReportCallback(
    WillmsgMsgReportCallback&& func)
{
    WillmsgMsgReportCallback old = std::move(m_willmsgMsgReportCallback);
    m_willmsgMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::RegisterMsgReportCallback DataProcessor::setRegisterMsgReportCallback(
    RegisterMsgReportCallback&& func)
{
    RegisterMsgReportCallback old = std::move(m_registerMsgReportCallback);
    m_registerMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::RegackMsgReportCallback DataProcessor::setRegackMsgReportCallback(
    RegackMsgReportCallback&& func)
{
    RegackMsgReportCallback old = std::move(m_regackMsgReportCallback);
    m_regackMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::PublishMsgReportCallback DataProcessor::setPublishMsgReportCallback(
    PublishMsgReportCallback&& func)
{
    PublishMsgReportCallback old = std::move(m_publishMsgReportCallback);
    m_publishMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::PubackMsgReportCallback DataProcessor::setPubackMsgReportCallback(
    PubackMsgReportCallback&& func)
{
    PubackMsgReportCallback old = std::move(m_pubackMsgReportCallback);
    m_pubackMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::PubrecMsgReportCallback DataProcessor::setPubrecMsgReportCallback(
    PubrecMsgReportCallback&& func)
{
    PubrecMsgReportCallback old = std::move(m_pubrecMsgReportCallback);
    m_pubrecMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::PubrelMsgReportCallback DataProcessor::setPubrelMsgReportCallback(
    PubrelMsgReportCallback&& func)
{
    PubrelMsgReportCallback old = std::move(m_pubrelMsgReportCallback);
    m_pubrelMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::PubcompMsgReportCallback DataProcessor::setPubcompMsgReportCallback(
    PubcompMsgReportCallback&& func)
{
    PubcompMsgReportCallback old = std::move(m_pubcompMsgReportCallback);
    m_pubcompMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::SubscribeMsgReportCallback DataProcessor::setSubscribeMsgReportCallback(
    SubscribeMsgReportCallback&& func)
{
    SubscribeMsgReportCallback old = std::move(m_subscribeMsgReportCallback);
    m_subscribeMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::UnsubscribeMsgReportCallback DataProcessor::setUnsubscribeMsgReportCallback(
    UnsubscribeMsgReportCallback&& func)
{
    UnsubscribeMsgReportCallback old = std::move(m_unsubscribeMsgReportCallback);
    m_unsubscribeMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::PingreqMsgReportCallback DataProcessor::setPingreqMsgReportCallback(
    PingreqMsgReportCallback&& func)
{
    PingreqMsgReportCallback old = std::move(m_pingreqMsgReportCallback);
    m_pingreqMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::PingrespMsgReportCallback DataProcessor::setPingrespMsgReportCallback(
    PingrespMsgReportCallback&& func)
{
    PingrespMsgReportCallback old = std::move(m_pingrespMsgReportCallback);
    m_pingrespMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::DisconnectMsgReportCallback DataProcessor::setDisconnectMsgReportCallback(
    DisconnectMsgReportCallback&& func)
{
    DisconnectMsgReportCallback old = std::move(m_disconnectMsgReportCallback);
    m_disconnectMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::WilltopicupdMsgReportCallback
DataProcessor::setWilltopicupdMsgReportCallback(
    WilltopicupdMsgReportCallback&& func)
{
    WilltopicupdMsgReportCallback old = std::move(m_willtopicupdMsgReportCallback);
    m_willtopicupdMsgReportCallback = std::move(func);
    return old;
}

DataProcessor::WillmsgupdMsgReportCallback
DataProcessor::setWillmsgupdMsgReportCallback(
    WillmsgupdMsgReportCallback&& func)
{
    WillmsgupdMsgReportCallback old = std::move(m_willmsgupdMsgReportCallback);
    m_willmsgupdMsgReportCallback = std::move(func);
    return old;
}

void DataProcessor::handle(SearchgwMsg& msg)
{
    if (m_searchgwMsgReportCallback) {
        m_searchgwMsgReportCallback(msg);
    }
}

void DataProcessor::handle(ConnectMsg& msg)
{
    if (m_connectMsgReportCallback) {
        m_connectMsgReportCallback(msg);
    }
}

void DataProcessor::handle(WilltopicMsg& msg)
{
    if (m_willtopicMsgReportCallback) {
        m_willtopicMsgReportCallback(msg);
    }
}

void DataProcessor::handle(WillmsgMsg& msg)
{
    if (m_willmsgMsgReportCallback) {
        m_willmsgMsgReportCallback(msg);
    }
}

void DataProcessor::handle(RegisterMsg& msg)
{
    if (m_registerMsgReportCallback) {
        m_registerMsgReportCallback(msg);
    }
}

void DataProcessor::handle(RegackMsg& msg)
{
    if (m_regackMsgReportCallback) {
        m_regackMsgReportCallback(msg);
    }
}

void DataProcessor::handle(PublishMsg& msg)
{
    if (m_publishMsgReportCallback) {
        m_publishMsgReportCallback(msg);
    }
}

void DataProcessor::handle(PubackMsg& msg)
{
    if (m_pubackMsgReportCallback) {
        m_pubackMsgReportCallback(msg);
    }
}

void DataProcessor::handle(PubrecMsg& msg)
{
    if (m_pubrecMsgReportCallback) {
        m_pubrecMsgReportCallback(msg);
    }
}

void DataProcessor::handle(PubrelMsg& msg)
{
    if (m_pubrelMsgReportCallback) {
        m_pubrelMsgReportCallback(msg);
    }
}

void DataProcessor::handle(PubcompMsg& msg)
{
    if (m_pubcompMsgReportCallback) {
        m_pubcompMsgReportCallback(msg);
    }
}

void DataProcessor::handle(SubscribeMsg& msg)
{
    if (m_subscribeMsgReportCallback) {
        m_subscribeMsgReportCallback(msg);
    }
}

void DataProcessor::handle(UnsubscribeMsg& msg)
{
    if (m_unsubscribeMsgReportCallback) {
        m_unsubscribeMsgReportCallback(msg);
    }
}

void DataProcessor::handle(PingreqMsg& msg)
{
    if (m_pingreqMsgReportCallback) {
        m_pingreqMsgReportCallback(msg);
    }
}

void DataProcessor::handle(PingrespMsg& msg)
{
    if (m_pingrespMsgReportCallback) {
        m_pingrespMsgReportCallback(msg);
    }
}

void DataProcessor::handle(DisconnectMsg& msg)
{
    if (m_disconnectMsgReportCallback) {
        m_disconnectMsgReportCallback(msg);
    }
}

void DataProcessor::handle(WilltopicupdMsg& msg)
{
    if (m_willtopicupdMsgReportCallback) {
        m_willtopicupdMsgReportCallback(msg);
    }
}

void DataProcessor::handle(WillmsgupdMsg& msg)
{
    if (m_willmsgupdMsgReportCallback) {
        m_willmsgupdMsgReportCallback(msg);
    }
}

void DataProcessor::handle(TestMessage& msg)
{
    std::cout << "ERROR: unhandled message of type: " << static_cast<unsigned>(msg.getId()) << std::endl;
    constexpr bool Handling_function_not_provided = false;
    static_cast<void>(Handling_function_not_provided);
    assert(Handling_function_not_provided);
}

void DataProcessor::checkWrittenMsg(const std::uint8_t* buf, std::size_t len)
{
    ProtStack::MsgPtr msg;
    auto readIter = comms::readIteratorFor<ProtStack::MsgPtr::element_type>(buf);
    auto es = m_stack.read(msg, readIter, len);
    static_cast<void>(es);
    if (es != comms::ErrorStatus::Success) {
        std::cout << "es = " << static_cast<unsigned>(es) << std::endl;
        std::cout << "Data: " << std::hex;
        std::copy_n(buf, len, std::ostream_iterator<unsigned>(std::cout, " "));
        std::cout << std::dec << std::endl;
    }
    assert(es == comms::ErrorStatus::Success);
    assert(msg);
    msg->dispatch(*this);
}

void DataProcessor::checkWrittenMsg(const DataBuf& data)
{
    checkWrittenMsg(&data[0], data.size());
}

DataProcessor::DataBuf DataProcessor::prepareInput(const TestMessage& msg)
{
    DataBuf buf(m_stack.length(msg));
    assert(buf.size() == m_stack.length(msg));
    auto writeIter = comms::writeIteratorFor<TestMessage>(&buf[0]);
    auto es = m_stack.write(msg, writeIter, buf.size());
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareGwinfoMsg(std::uint8_t id)
{
    GwinfoMsg msg;
    msg.field_gwId().value() = id;
    assert(msg.length() == 1U);
    auto buf = prepareInput(msg);
    assert(buf.size() == 3U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareAdvertiseMsg(std::uint8_t id, unsigned short duration)
{
    AdvertiseMsg msg;
    msg.field_gwId().value() = id;
    msg.field_duration().value() = duration;
    auto buf = prepareInput(msg);
    assert(buf.size() == 5U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareConnackMsg(cc_mqttsn::field::ReturnCodeVal val)
{
    ConnackMsg msg;
    msg.field_returnCode().value() = val;
    auto buf = prepareInput(msg);
    assert(buf.size() == 3U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareWilltopicreqMsg()
{
    return prepareInput(WilltopicreqMsg());
}

DataProcessor::DataBuf DataProcessor::prepareWillmsgreqMsg()
{
    return prepareInput(WillmsgreqMsg());
}

DataProcessor::DataBuf DataProcessor::prepareRegisterMsg(
    std::uint16_t topicId,
    std::uint16_t msgId,
    const std::string& topicName)
{
    RegisterMsg msg;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_topicName().value() = topicName;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareRegackMsg(
    std::uint16_t topicId,
    std::uint16_t msgId,
    cc_mqttsn::field::ReturnCodeVal retCode)
{
    RegackMsg msg;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_returnCode().value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePublishMsg(
    std::uint16_t topicId,
    std::uint16_t msgId,
    const std::vector<std::uint8_t>& data,
    TopicIdTypeVal topicIdType,
    cc_mqttsn::field::QosVal qos,
    bool retain,
    bool duplicate)
{
    PublishMsg msg;
    msg.field_flags().field_topicIdType().value() = topicIdType;
    msg.field_flags().field_mid().setBitValue_Retain(retain);
    msg.field_flags().field_qos().value() = qos;
    msg.field_flags().field_high().setBitValue_Dup(duplicate);
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_data().value().assign(data.begin(), data.end());
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubackMsg(
    MqttsnTopicId topicId,
    std::uint16_t msgId,
    cc_mqttsn::field::ReturnCodeVal retCode)
{
    PubackMsg msg;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_returnCode().value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubrecMsg(std::uint16_t msgId)
{
    PubrecMsg msg;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubrelMsg(std::uint16_t msgId)
{
    PubrelMsg msg;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubcompMsg(std::uint16_t msgId)
{
    PubcompMsg msg;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareSubackMsg(
    cc_mqttsn::field::QosVal qos,
    MqttsnTopicId topicId,
    std::uint16_t msgId,
    cc_mqttsn::field::ReturnCodeVal retCode)
{
    SubackMsg msg;
    msg.field_flags().field_qos().value() = qos;
    msg.field_topicId().value() = topicId;
    msg.field_msgId().value() = msgId;
    msg.field_returnCode().value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareUnsubackMsg(
    std::uint16_t msgId)
{
    UnsubackMsg msg;
    msg.field_msgId().value() = msgId;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePingreqMsg()
{
    return prepareInput(PingreqMsg());
}

DataProcessor::DataBuf DataProcessor::preparePingrespMsg()
{
    return prepareInput(PingrespMsg());
}

DataProcessor::DataBuf DataProcessor::prepareDisconnectMsg(std::uint16_t duration)
{
    DisconnectMsg msg;

    if (duration != 0U) {
        msg.field_duration().setExists();
        msg.field_duration().field().value() = duration;
    }
    else {
        msg.field_duration().setMissing();
    }
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareWilltopicrespMsg(
    cc_mqttsn::field::ReturnCodeVal retCode)
{
    WilltopicrespMsg msg;
    msg.field_returnCode().value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareWillmsgrespMsg(
    cc_mqttsn::field::ReturnCodeVal retCode)
{
    WillmsgrespMsg msg;
    msg.field_returnCode().value() = retCode;
    return prepareInput(msg);
}

