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

class GwDiscover : public AppClient
{
    using Base = AppClient;
public:
    GwDiscover(boost::asio::io_context& io, int& result);

protected:
    virtual bool startImpl() override;
private:
    void gwStatusReportInternal(CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info);
    void searchCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info);
    std::string addrToString(const CC_MqttsnGatewayInfo& info);
    std::string addrToString_ipv4(const CC_MqttsnGatewayInfo& info);

    static void gwStatusReportCb(void* data, CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info);
    static void searchCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info);

    Timer m_timeoutTimer;
};

} // namespace cc_mqttsn_client_app
