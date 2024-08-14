//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Sub.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <type_traits>

namespace cc_mqttsn_client_app
{

namespace 
{

Sub* asThis(void* data)
{
    return reinterpret_cast<Sub*>(data);
}

} // namespace 
    

Sub::Sub(boost::asio::io_context& io, int& result) : 
    Base(io, result)
{
    opts().addCommon();
    opts().addNetwork();
    opts().addConnect();
    opts().addWill();
    opts().addSubscribe();
}    

bool Sub::startImpl()
{
    return doConnect();
}

void Sub::messageReceivedImpl(const CC_MqttsnMessageInfo* info)
{
    assert(info != nullptr);
    assert(info->m_topic != nullptr);
    if (info->m_retained && opts().subNoRetained()) {
        return;
    }

    if (opts().verbose()) {
        print(*info);
    }   
    else {
        std::cout << info->m_topic << ": " << toString(info->m_data, info->m_dataLen, opts().subBinary()) << std::endl;
    }
}

void Sub::connectCompleteImpl()
{
    auto topics = opts().subTopics();
    auto topicIds = opts().subTopicIds();

    if (topics.empty() && topicIds.empty()) {
        logInfo() << "Not subscription topics provided";
        return;
    }

    if (opts().verbose()) {
        logInfo() << "Subscribing..." << std::endl;
    }     

    auto doSubscribe = 
        [this](const std::string& topic, std::uint16_t topicId)
        {
            auto config = CC_MqttsnSubscribeConfig();
            cc_mqttsn_client_subscribe_init_config(&config);

            if (!topic.empty()) {
                config.m_topic = topic.c_str();
            }
            else {
                config.m_topicId = topicId;
            }

            config.m_qos = static_cast<decltype(config.m_qos)>(opts().subQos());

            auto ec = cc_mqttsn_client_subscribe(client(), &config, &Sub::subscribeCompleteCb, this);
            if (ec != CC_MqttsnErrorCode_Success) {
                if (!topic.empty()) {
                    logError() << "Failed to initiate subscribe subscribe to topic \"" << topic << "\" with error: " << toString(ec) << std::endl;
                }
                else {
                    logError() << "Failed to initiate subscribe subscribe to topic ID " << topicId << " with error: " << toString(ec) << std::endl;
                }
                return;
            }

            ++m_subCount;
        };
    
    for (auto& t : topics) {
        doSubscribe(t, 0U);
    }

    for (auto id : topicIds) {
        doSubscribe(std::string(), id);
    }    
}

void Sub::subscribeCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info)
{
    do {
        if (status != CC_MqttsnAsyncOpStatus_Complete) {
            logError() << "Subscribe failed with status: " << toString(status) << std::endl;
            break;
        }

        if ((info != nullptr) && (info->m_returnCode != CC_MqttsnReturnCode_Accepted)) {
            logError() << "Subscribe rejected with return code: " << toString(info->m_returnCode) << std::endl;
            break;
        }

    } while (false);

    --m_subCount;
    if ((m_subCount == 0) && opts().verbose()) {
        logInfo() << "Subscription complete, waiting for messages" << std::endl;
    }
}

void Sub::subscribeCompleteCb(
    void* data, 
    [[maybe_unused]] CC_MqttsnSubscribeHandle handle, 
    CC_MqttsnAsyncOpStatus status, 
    const CC_MqttsnSubscribeInfo* info)
{
    asThis(data)->subscribeCompleteInternal(status, info);
}

} // namespace cc_mqttsn_client_app
