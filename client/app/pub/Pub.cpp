//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Pub.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <type_traits>

namespace cc_mqttsn_client_app
{

namespace 
{

Pub* asThis(void* data)
{
    return reinterpret_cast<Pub*>(data);
}

} // namespace 
    

Pub::Pub(boost::asio::io_context& io, int& result) : 
    Base(io, result)
{
    opts().addCommon();
    opts().addNetwork();
    opts().addConnect();
    opts().addWill();
    opts().addPublish();
}    

bool Pub::startImpl()
{
    auto topic = opts().pubTopic();
    auto topicId = opts().pubTopicId();
    if (topic.empty() && topicId == 0) {
        logError() << "Neither topic nor topic ID are specified" << std::endl;
        return false;
    }

    if ((!topic.empty()) && (topicId != 0)) {
        logError() << "Both topic topic topic ID are specified" << std::endl;
        return false;        
    }

    return doConnect();
}

void Pub::connectCompleteImpl()
{
    auto config = CC_MqttsnPublishConfig();
    cc_mqttsn_client_publish_init_config(&config);

    auto topic = opts().pubTopic();
    auto data = parseBinaryData(opts().pubMessage());

    if (!topic.empty()) {
        config.m_topic = topic.c_str();
    }

    config.m_topicId = opts().pubTopicId();
    config.m_data = data.data();
    config.m_dataLen = static_cast<decltype(config.m_dataLen)>(data.size());
    config.m_qos = static_cast<decltype(config.m_qos)>(opts().pubQos());
    config.m_retain = opts().pubRetain();

    auto ec = cc_mqttsn_client_publish(client(), &config, &Pub::publishCompleteCb, this);
    if (ec == CC_MqttsnErrorCode_Success) {
        return;
    }

    logError() << "Failed to initiate publish operation with error code: " << toString(ec) << std::endl;
    doTerminate(); 
}

void Pub::publishCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info)
{
    if (status != CC_MqttsnAsyncOpStatus_Complete) {
        logError() << "Publish failed with status: " << toString(status) << std::endl;
        doTerminate();
        return;
    }

    if ((info != nullptr) && (info->m_returnCode != CC_MqttsnReturnCode_Accepted)) {
        logError() << "Publish rejected with return code: " << toString(info->m_returnCode) << std::endl;
        doTerminate();
        return;
    }

    if (opts().verbose()) {
        logInfo() << "Publish complete" << std::endl;
    }

    if (opts().pubNoDisconnect()) {
        doComplete();
        return;
    }

    doDisconnect();
}

void Pub::publishCompleteCb(
    void* data, 
    [[maybe_unused]] CC_MqttsnPublishHandle handle, 
    CC_MqttsnAsyncOpStatus status, 
    const CC_MqttsnPublishInfo* info)
{
    asThis(data)->publishCompleteInternal(status, info);
}

} // namespace cc_mqttsn_client_app
