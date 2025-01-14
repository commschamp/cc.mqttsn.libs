//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla GwDiscoverlic
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GwDiscover.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <type_traits>

namespace cc_mqttsn_client_app
{

namespace 
{

GwDiscover* asThis(void* data)
{
    return reinterpret_cast<GwDiscover*>(data);
}

} // namespace 
    

GwDiscover::GwDiscover(boost::asio::io_context& io, int& result) : 
    Base(io, result),
    m_timeoutTimer(io)
{
    opts().addCommon();
    opts().addNetwork();
    opts().addDiscover();
}    

bool GwDiscover::startImpl()
{
    auto timeout = opts().discoverTimeout();
    if (timeout > 0U) {
        m_timeoutTimer.expires_after(std::chrono::seconds(timeout));
        m_timeoutTimer.async_wait(
            [this](const boost::system::error_code& ec)
            {
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                }

                doTerminate(0);
            });
    }

    cc_mqttsn_client_set_gw_status_report_callback(client(), &GwDiscover::gwStatusReportCb, this);
    auto ec = cc_mqttsn_client_search(client(), &GwDiscover::searchCompleteCb, this);
    if (ec != CC_MqttsnErrorCode_Success) {
        logError() << "Failed to initiate search operation" << std::endl;
    }

    return true;
}

void GwDiscover::gwStatusReportInternal(CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info)
{
    using Elem = std::pair<char, std::string>;
    static const Elem Map[] = {
        /* CC_MqttsnGwStatus_AddedByGateway */ {'+', " (gw)"},
        /* CC_MqttsnGwStatus_AddedByClient */ {'+', " (client)" },
        /* CC_MqttsnGwStatus_UpdatedByClient */ {'=', " (client)"},
        /* CC_MqttsnGwStatus_Alive */ {'=', std::string()},
        /* CC_MqttsnGwStatus_Tentative */ {'?', std::string()},
        /* CC_MqttsnGwStatus_Removed */ {'-', std::string()},
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnGwStatus_ValuesLimit);

    auto idx = static_cast<unsigned>(status);
    if (MapSize <= idx) {
        logError() << "Unexpected gateway status: " << status << std::endl;
        assert(false);
        return;
    }

    auto& prefixSuffixInfo = Map[idx];
    auto infoTmp = *info;

    static const CC_MqttsnGwStatus GwUpdateStatuses[] = {
        CC_MqttsnGwStatus_AddedByGateway,
        CC_MqttsnGwStatus_Alive,
    };

    auto iter = std::find(std::begin(GwUpdateStatuses), std::end(GwUpdateStatuses), status);

    if (iter != std::end(GwUpdateStatuses)) {
        infoTmp.m_addr = lastAddr().data();
        infoTmp.m_addrLen = static_cast<decltype(infoTmp.m_addrLen)>(lastAddr().size());

        auto ec = cc_mqttsn_client_set_available_gateway_info(client(), &infoTmp);
        if (ec != CC_MqttsnErrorCode_Success) {
            logError() << "Failed to update gateway info" << std::endl;
            assert(false);
        }
    }

    std::cout << prefixSuffixInfo.first << ' ' << static_cast<unsigned>(infoTmp.m_gwId) << 
        ": " << addrToString(infoTmp) << prefixSuffixInfo.second << std::endl;

    if (opts().discoverExitOnFirst()) {
        doTerminate(0);
    }
}

void GwDiscover::searchCompleteInternal(CC_MqttsnAsyncOpStatus status, [[maybe_unused]] const CC_MqttsnGatewayInfo* info)
{
    if (status != CC_MqttsnAsyncOpStatus_Complete) {
        logError() << "The initial gateway search has failed with status: " << toString(status) << std::endl;
    }

    // The gateway status report will follow
}

std::string GwDiscover::addrToString(const CC_MqttsnGatewayInfo& info)
{
    using Func = std::string (GwDiscover::*)(const CC_MqttsnGatewayInfo&);
    static const Func Map[] = {
        /* ConnectionType_Udp */ &GwDiscover::addrToString_ipv4
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == ProgramOptions::ConnectionType_ValuesLimit);

    auto idx = static_cast<unsigned>(opts().connectionType());
    if (MapSize <= idx) {
        logError() << "Unsupported connection type" << std::endl;
        assert(false);
        return std::string();
    }

    auto func = Map[idx];
    return (this->*func)(info);
}

std::string GwDiscover::addrToString_ipv4(const CC_MqttsnGatewayInfo& info)
{
    std::string result;
    static const unsigned MaxLen = 4U;
    for (auto idx = 0U; idx < std::min(info.m_addrLen, MaxLen); ++idx) {
        if (!result.empty()) {
            result += '.';
        }
        result += std::to_string(static_cast<unsigned>(info.m_addr[idx]));
    }
    return result;
}

void GwDiscover::gwStatusReportCb(void* data, CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info)
{
    asThis(data)->gwStatusReportInternal(status, info);
}

void GwDiscover::searchCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info)
{
    asThis(data)->searchCompleteInternal(status, info);
}

} // namespace cc_mqttsn_client_app
