//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "AppClient.h"
#include "ProgramOptions.h"

#include <boost/asio.hpp>

namespace cc_mqttsn_client_app
{

class Sub : public AppClient
{
    using Base = AppClient;
public:
    Sub(boost::asio::io_context& io, int& result);

protected:
    virtual bool startImpl() override;
    virtual void messageReceivedImpl(const CC_MqttsnMessageInfo* info) override;
    virtual void connectCompleteImpl() override;

private:
    void subscribeCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info);
    static void subscribeCompleteCb(void* data, CC_MqttsnSubscribeHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info);

    unsigned m_subCount = 0U;
};

} // namespace cc_mqttsn_client_app
