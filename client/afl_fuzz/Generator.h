//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Logger.h"

#include "cc_mqttsn/Message.h"
#include "cc_mqttsn/frame/Frame.h"

#include "comms/options.h"

#include <cstdint>
#include <iterator>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cc_mqttsn_client_afl_fuzz
{

class Generator
{
public:
    using RawDataBuf = std::vector<std::uint8_t>;

    using MqttsnMessage = 
        cc_mqttsn::Message<
            comms::option::app::ReadIterator<const std::uint8_t*>,
            comms::option::app::WriteIterator<std::back_insert_iterator<RawDataBuf>>,
            comms::option::app::LengthInfoInterface,
            comms::option::app::IdInfoInterface,
            comms::option::app::NameInterface,
            comms::option::app::RefreshInterface,
            comms::option::app::Handler<Generator>
        >;

    CC_MQTTSN_ALIASES_FOR_ALL_MESSAGES_DEFAULT_OPTIONS(Mqttsn, Msg, MqttsnMessage);

    using DataReportCb = std::function<void (const std::uint8_t* buf, std::size_t bufLen, unsigned broadcastRadius)>;

    Generator(Logger& logger, unsigned minPubCount) : m_logger(logger), m_minPubCount(minPubCount) {};

    bool prepare(const std::string& inputFile);
    void processData(const std::uint8_t* buf, unsigned bufLen);

    template <typename TFunc>
    void setDataReportCb(TFunc&& func)
    {
        m_dataReportCb = std::forward<TFunc>(func);
    }

    void handle(const MqttsnSearchgwMsg& msg);
    void handle(const MqttsnConnectMsg& msg);
    void handle(const MqttsnPublishMsg& msg);
    void handle(const MqttsnPubrecMsg& msg);
    void handle(const MqttsnPubrelMsg& msg);
    void handle(const MqttsnSubscribeMsg& msg);
    void handle(const MqttsnUnsubscribeMsg& msg);
    void handle(const MqttsnMessage& msg);

private:
    using MqttsnFrame = cc_mqttsn::frame::Frame<MqttsnMessage>;

    unsigned allocPacketId();
    unsigned allocTopicAlias(const std::string& topic);
    void sendMessage(MqttsnMessage& msg, unsigned broadcastRadius = 0);
    void sendConnack(const MqttsnConnectMsg& msg);
    void sendPublish(const std::string& topic, unsigned qos);
    void doPublish();
    void doNextPublishIfNeeded();

    Logger& m_logger;
    unsigned m_minPubCount = 0U;
    std::ofstream m_stream;
    DataReportCb m_dataReportCb;
    MqttsnFrame m_frame;
    std::unique_ptr<MqttsnConnectMsg> m_cachedConnect;
    unsigned m_lastPacketId = 0U;
    unsigned m_nextPubQos = 0U;
    std::string m_lastPubTopic;
    unsigned m_pubCount = 0U;
};

using GeneratorPtr = std::unique_ptr<Generator>;

} // namespace cc_mqttsn_client_afl_fuzz
