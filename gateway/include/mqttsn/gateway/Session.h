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


#pragma once

#include <memory>
#include <functional>
#include <cstdint>
#include <vector>

#include "mqttsn/gateway/Api.h"

namespace mqttsn
{

namespace gateway
{

class SessionImpl;
class MQTTSN_GATEWAY_API Session
{
public:

    typedef std::vector<std::uint8_t> BinaryData;
    typedef std::pair<std::string, BinaryData> AuthInfo;

    typedef std::function<void (unsigned value)> NextTickProgramReqCb;
    typedef std::function<unsigned ()> CancelTickWaitReqCb;
    typedef std::function<void (const std::uint8_t* buf, std::size_t bufSize)> SendDataReqCb;
    typedef std::function<void ()> TerminationReqCb;
    typedef std::function<void ()> BrokerReconnectReqCb;
    typedef std::function<void (const std::string&)> ClientConnectedReportCb;
    typedef std::function<AuthInfo (const std::string& clientId)> AuthInfoReqCb;


    Session();
    ~Session();

    void setNextTickProgramReqCb(NextTickProgramReqCb&& func);
    void setCancelTickWaitReqCb(CancelTickWaitReqCb&& func);
    void setSendDataClientReqCb(SendDataReqCb&& func);
    void setSendDataBrokerReqCb(SendDataReqCb&& func);
    void setTerminationReqCb(TerminationReqCb&& func);
    void setBrokerReconnectReqCb(BrokerReconnectReqCb&& func);
    void setClientConnectedReportCb(ClientConnectedReportCb&& func);
    void setAuthInfoReqCb(AuthInfoReqCb&& func);
    void setGatewayId(std::uint8_t value);
    void setRetryPeriod(unsigned value);
    void setRetryCount(unsigned value);
    void setSleepingClientMsgLimit(std::size_t value);
    void setDefaultClientId(const std::string& value);
    void setPubOnlyKeepAlive(std::uint16_t value);

    bool start();
    void stop();
    bool isRunning() const;
    void tick(unsigned ms);

    std::size_t dataFromClient(const std::uint8_t* buf, std::size_t len);
    std::size_t dataFromBroker(const std::uint8_t* buf, std::size_t len);

    void setBrokerConnected(bool connected);
    bool addPredefinedTopic(const std::string& topic, std::uint16_t topicId);
    bool setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal);


private:
    std::unique_ptr<SessionImpl> m_pImpl;
};

}  // namespace gateway

}  // namespace mqttsn

