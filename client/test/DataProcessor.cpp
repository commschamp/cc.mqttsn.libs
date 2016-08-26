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
    std::cout << "ERROR: unhandled message of type: " << (unsigned)msg.getId() << std::endl;
    assert(!"Provide handling function");
}

void DataProcessor::checkWrittenMsg(const std::uint8_t* buf, std::size_t len)
{
    ProtStack::ReadIterator readIter = buf;
    ProtStack::MsgPtr msg;
    auto es = m_stack.read(msg, readIter, len);
    static_cast<void>(es);
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
    ProtStack::WriteIterator writeIter = &buf[0];
    auto es = m_stack.write(msg, writeIter, buf.size());
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareGwinfoMsg(std::uint8_t id)
{
    GwinfoMsg msg;
    auto& fields = msg.fields();
    auto& gwIdField = std::get<GwinfoMsg::FieldIdx_gwId>(fields);
    gwIdField.value() = id;
    assert(msg.length() == 1U);
    auto buf = prepareInput(msg);
    assert(buf.size() == 3U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareAdvertiseMsg(std::uint8_t id, unsigned short duration)
{
    AdvertiseMsg msg;
    auto& fields = msg.fields();
    auto& gwIdField = std::get<AdvertiseMsg::FieldIdx_gwId>(fields);
    auto& durationField = std::get<AdvertiseMsg::FieldIdx_duration>(fields);
    gwIdField.value() = id;
    durationField.value() = duration;
    auto buf = prepareInput(msg);
    assert(buf.size() == 5U);
    return buf;
}

DataProcessor::DataBuf DataProcessor::prepareConnackMsg(mqttsn::protocol::field::ReturnCodeVal val)
{
    ConnackMsg msg;
    auto& fields = msg.fields();
    auto& retCodeField = std::get<ConnackMsg::FieldIdx_returnCode>(fields);
    retCodeField.value() = val;
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
    auto& fields = msg.fields();
    std::get<decltype(msg)::FieldIdx_topicId>(fields).value() = topicId;
    std::get<decltype(msg)::FieldIdx_msgId>(fields).value() = msgId;
    std::get<decltype(msg)::FieldIdx_topicName>(fields).value() = topicName;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareRegackMsg(
    std::uint16_t topicId,
    std::uint16_t msgId,
    mqttsn::protocol::field::ReturnCodeVal retCode)
{
    RegackMsg msg;
    auto& fields = msg.fields();
    std::get<RegackMsg::FieldIdx_topicId>(fields).value() = topicId;
    std::get<RegackMsg::FieldIdx_msgId>(fields).value() = msgId;
    std::get<RegackMsg::FieldIdx_returnCode>(fields).value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePublishMsg(
    std::uint16_t topicId,
    std::uint16_t msgId,
    const std::vector<std::uint8_t>& data,
    mqttsn::protocol::field::TopicIdTypeVal topicIdType,
    mqttsn::protocol::field::QosType qos,
    bool retain,
    bool duplicate)
{
    PublishMsg msg;
    auto& fields = msg.fields();
    auto& flagsField = std::get<PublishMsg::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& topicIdTypeField = std::get<mqttsn::protocol::field::FlagsMemberIdx_topicId>(flagsMembers);
    auto& midFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_midFlags>(flagsMembers);
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& dupFlagsField = std::get<mqttsn::protocol::field::FlagsMemberIdx_dupFlags>(flagsMembers);
    auto& topicIdField = std::get<PublishMsg::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<PublishMsg::FieldIdx_msgId>(fields);
    auto& dataField = std::get<PublishMsg::FieldIdx_data>(fields);

    topicIdTypeField.value() = topicIdType;
    midFlagsField.setBitValue(mqttsn::protocol::field::MidFlagsBits_retain, retain);
    qosField.value() = qos;
    dupFlagsField.setBitValue(mqttsn::protocol::field::DupFlagsBits_dup, duplicate);
    topicIdField.value() = topicId;
    msgIdField.value() = msgId;
    dataField.value().assign(data.begin(), data.end());

    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubackMsg(
    MqttsnTopicId topicId,
    std::uint16_t msgId,
    mqttsn::protocol::field::ReturnCodeVal retCode)
{
    PubackMsg msg;
    auto& fields = msg.fields();
    std::get<PubackMsg::FieldIdx_topicId>(fields).value() = topicId;
    std::get<PubackMsg::FieldIdx_msgId>(fields).value() = msgId;
    std::get<PubackMsg::FieldIdx_returnCode>(fields).value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubrecMsg(std::uint16_t msgId)
{
    PubrecMsg msg;
    auto& fields = msg.fields();
    std::get<decltype(msg)::FieldIdx_msgId>(fields).value() = msgId;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubrelMsg(std::uint16_t msgId)
{
    PubrelMsg msg;
    auto& fields = msg.fields();
    std::get<decltype(msg)::FieldIdx_msgId>(fields).value() = msgId;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::preparePubcompMsg(std::uint16_t msgId)
{
    PubcompMsg msg;
    auto& fields = msg.fields();
    std::get<decltype(msg)::FieldIdx_msgId>(fields).value() = msgId;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareSubackMsg(
    mqttsn::protocol::field::QosType qos,
    MqttsnTopicId topicId,
    std::uint16_t msgId,
    mqttsn::protocol::field::ReturnCodeVal retCode)
{
    SubackMsg msg;
    auto& fields = msg.fields();
    auto& flagsField = std::get<decltype(msg)::FieldIdx_flags>(fields);
    auto& flagsMembers = flagsField.value();
    auto& qosField = std::get<mqttsn::protocol::field::FlagsMemberIdx_qos>(flagsMembers);
    auto& topicIdField = std::get<decltype(msg)::FieldIdx_topicId>(fields);
    auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);
    auto& retCodeField = std::get<decltype(msg)::FieldIdx_returnCode>(fields);

    qosField.value() = qos;
    topicIdField.value() = topicId;
    msgIdField.value() = msgId;
    retCodeField.value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareUnsubackMsg(
    std::uint16_t msgId)
{
    UnsubackMsg msg;
    auto& fields = msg.fields();
    auto& msgIdField = std::get<decltype(msg)::FieldIdx_msgId>(fields);

    msgIdField.value() = msgId;
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

DataProcessor::DataBuf DataProcessor::prepareDisconnectMsg()
{
    return prepareInput(DisconnectMsg());
}

DataProcessor::DataBuf DataProcessor::prepareWilltopicrespMsg(
    mqttsn::protocol::field::ReturnCodeVal retCode)
{
    WilltopicrespMsg msg;
    auto& fields = msg.fields();
    auto& retCodeField = std::get<decltype(msg)::FieldIdx_returnCode>(fields);
    retCodeField.value() = retCode;
    return prepareInput(msg);
}

DataProcessor::DataBuf DataProcessor::prepareWillmsgrespMsg(
    mqttsn::protocol::field::ReturnCodeVal retCode)
{
    WillmsgrespMsg msg;
    auto& fields = msg.fields();
    auto& retCodeField = std::get<decltype(msg)::FieldIdx_returnCode>(fields);
    retCodeField.value() = retCode;
    return prepareInput(msg);
}

