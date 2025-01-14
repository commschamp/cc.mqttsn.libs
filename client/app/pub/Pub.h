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

class Pub : public AppClient
{
    using Base = AppClient;
public:
    Pub(boost::asio::io_context& io, int& result);

protected:
    virtual bool startImpl() override;
    virtual void connectCompleteImpl() override;

private:
    void doPublish();
    void doCompleteInternal();
    void publishCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info);
    static void publishCompleteCb(void* data, CC_MqttsnPublishHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info);

    boost::asio::steady_timer m_timer;
    unsigned m_remCount = 0U;
};

} // namespace cc_mqttsn_client_app
