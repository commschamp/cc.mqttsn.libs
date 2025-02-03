//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Generator.h"

#include "comms/process.h"

#include <cassert>
#include <type_traits>

namespace cc_mqttsn_client_afl_fuzz
{

namespace 
{

std::string pubTopicFromFilter(const std::string& filter)
{
    std::string result;
    std::size_t pos = 0U;
    while (pos < filter.size()) {
        auto wildcardPos = filter.find_first_of("#+", pos);
        if (wildcardPos == std::string::npos) {
            break;
        }

        result.append(filter.substr(pos, wildcardPos - pos));
        pos = wildcardPos + 1U;
        
        if (filter[wildcardPos] == '#') {
            result.append("hash");
            pos = filter.size();
            break;
        }

        assert(filter[wildcardPos] == '+');
        result.append("plus");
    }

    if (pos < filter.size()) {
        result.append(filter.substr(pos));
    }

    return result;
}

} // namespace 
    

bool Generator::prepare(const std::string& inputFile)
{
    m_stream.open(inputFile);
    if (!m_stream) {
        m_logger.errorLog() << "Failed to open " << inputFile << " for writing" << std::endl;
        return false;
    }

    return true;
}

void Generator::processData(const std::uint8_t* buf, unsigned bufLen)
{
    [[maybe_unused]] auto consumed = comms::processAllWithDispatch(buf, bufLen, m_frame, *this);
    assert(consumed == bufLen);
}

void Generator::handle(const MqttsnSearchgwMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";
    MqttsnGwinfoMsg outMsg;
    outMsg.field_gwId().setValue(1);
    sendMessage(outMsg, msg.field_radius().value());

    MqttsnAdvertiseMsg advMsg;
    advMsg.field_gwId().setValue(1);
    advMsg.field_duration().setValue(100);
    sendMessage(advMsg, msg.field_radius().value());
}

void Generator::handle(const MqttsnConnectMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";

    if (msg.field_flags().field_mid().getBitValue_Will()) {
        MqttsnWilltopicreqMsg willTopicReqMsg;
        sendMessage(willTopicReqMsg);    
        return;
    }

    MqttsnConnackMsg outMsg;
    sendMessage(outMsg);
}

void Generator::handle(const MqttsnWilltopicMsg& msg)
{
    if (msg.field_willTopic().value().empty()) {
        MqttsnConnackMsg outMsg;
        sendMessage(outMsg);     
        return;   
    }

    MqttsnWillmsgreqMsg willMsgReqMsg;
    sendMessage(willMsgReqMsg);     
}

void Generator::handle([[maybe_unused]] const MqttsnWillmsgMsg& msg)
{
    MqttsnConnackMsg outMsg;
    sendMessage(outMsg);     
}

void Generator::handle(const MqttsnRegackMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";
    doNextPublishIfNeeded();
}

void Generator::handle(const MqttsnPublishMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";

    using QosValueType = MqttsnPublishMsg::Field_flags::Field_qos::ValueType;
    auto qos = msg.field_flags().field_qos().value();
    if (qos == QosValueType::AtMostOnceDelivery) {
        m_logger.infoLog() << "at most once" << "\n";
        doNextPublishIfNeeded();
        return;
    }

    if (qos == QosValueType::AtLeastOnceDelivery) {
        MqttsnPubackMsg outMsg;
        outMsg.field_topicId().setValue(msg.field_topicId().getValue());
        outMsg.field_msgId().setValue(msg.field_msgId().getValue());
        sendMessage(outMsg);
        doNextPublishIfNeeded();
        return;
    }

    assert(qos == QosValueType::ExactlyOnceDelivery);
    MqttsnPubrecMsg outMsg;
    outMsg.field_msgId().setValue(msg.field_msgId().getValue());
    sendMessage(outMsg);
    return;
}

void Generator::handle(const MqttsnPubrecMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";
    MqttsnPubrelMsg outMsg;
    outMsg.field_msgId().setValue(msg.field_msgId().getValue());
    sendMessage(outMsg);
}

void Generator::handle(const MqttsnPubrelMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";
    MqttsnPubcompMsg outMsg;
    outMsg.field_msgId().setValue(msg.field_msgId().getValue());
    sendMessage(outMsg);
    doNextPublishIfNeeded();
}

void Generator::handle(const MqttsnSubscribeMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";
    MqttsnSubackMsg outMsg;
    outMsg.field_flags().field_qos().setValue(msg.field_flags().field_qos().getValue());
    outMsg.field_msgId().setValue(msg.field_msgId().getValue());
    sendMessage(outMsg);     

    PubInfo pub;
    pub.m_maxQos = static_cast<decltype(pub.m_maxQos)>(msg.field_flags().field_qos().getValue());
    if (msg.field_topicName().doesExist()) {
        pub.m_topicId = allocTopicId();

        m_pubs.push_back(pub);

        MqttsnRegisterMsg regMsg;
        regMsg.field_topicId().setValue(pub.m_topicId);
        regMsg.field_msgId().setValue(allocPacketId());
        regMsg.field_topicName().setValue(pubTopicFromFilter(msg.field_topicName().field().getValue()));
        sendMessage(regMsg); 
        return;

    }

    assert(msg.field_topicId().doesExist());
    pub.m_topicId = msg.field_topicId().field().value();
    pub.m_type = msg.field_flags().field_topicIdType().getValue();
    m_pubs.push_back(pub);
    doNextPublishIfNeeded();
}

void Generator::handle(const MqttsnUnsubscribeMsg& msg)
{
    m_logger.infoLog() << "Processing " << msg.name() << "\n";
    MqttsnUnsubackMsg outMsg;
    outMsg.field_msgId().value() = msg.field_msgId().value();
    sendMessage(outMsg);
}

void Generator::handle([[maybe_unused]] const MqttsnMessage& msg)
{
    m_logger.infoLog() << "Ignoring " << msg.name() << "\n";
}

unsigned Generator::allocPacketId()
{
    ++m_lastPacketId;
    return m_lastPacketId;
}

std::uint16_t Generator::allocTopicId()
{
    ++m_lastTopicId;
    return m_lastTopicId;
}

void Generator::sendMessage(MqttsnMessage& msg, unsigned broadcastRadius)
{
    m_logger.infoLog() << "Generating " << msg.name() << "\n";
    msg.refresh();
    RawDataBuf outBuf;
    outBuf.reserve(m_frame.length(msg));
    auto iter = std::back_inserter(outBuf);
    [[maybe_unused]] auto es = m_frame.write(msg, iter, outBuf.max_size());
    assert(es == comms::ErrorStatus::Success);
    assert(m_dataReportCb);
    
    std::ostreambuf_iterator<char> outIter(m_stream);
    std::copy(outBuf.begin(), outBuf.end(), outIter);
    m_dataReportCb(outBuf.data(), outBuf.size(), broadcastRadius);
}

void Generator::doPublish()
{
    assert(!m_pubs.empty());
    auto pub = m_pubs.front();
    m_pubs.pop_front();

    m_logger.infoLog() << "Sending publish with qos=" << pub.m_nextQos << "\n";
    MqttsnPublishMsg outMsg;
    outMsg.field_flags().field_qos().setValue(pub.m_nextQos);
    outMsg.field_flags().field_topicIdType().setValue(pub.m_type);
    outMsg.field_topicId().setValue(pub.m_topicId);
    if (pub.m_nextQos != 0) {
        outMsg.field_msgId().setValue(allocPacketId());
    }

    static const std::string data("bla");
    comms::util::assign(outMsg.field_data().value(), data.begin(), data.end());

    ++pub.m_nextQos;
    if (pub.m_maxQos < pub.m_nextQos) {
        pub.m_nextQos = 0;
    }

    m_pubs.push_back(pub);
    sendMessage(outMsg);
    ++m_pubCount;
} 

void Generator::doNextPublishIfNeeded()
{
    if (m_pubCount < m_minPubCount) {
        doPublish();
    }
}


} // namespace cc_mqttsn_client_afl_fuzz
