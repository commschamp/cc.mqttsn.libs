//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
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

#include <array>

#include "mqttsn/gateway/Gateway.h"
#include "mqttsn/Message.h"
#include "mqttsn/frame/Frame.h"

namespace mqttsn
{

namespace gateway
{

class GatewayImpl
{
public:
    using NextTickProgramReqCb = Gateway::NextTickProgramReqCb;
    using SendDataReqCb = Gateway::SendDataReqCb;

    using Message =
        mqttsn::Message<
        comms::option::IdInfoInterface,
            comms::option::WriteIterator<std::uint8_t*>,
            comms::option::LengthInfoInterface
        >;

    using AdvertiseMsg = mqttsn::message::Advertise<Message>;
    using AllMsgs = std::tuple<AdvertiseMsg>;
    using ProtStack = mqttsn::frame::Frame<Message, AllMsgs>;

    template <typename TFunc>
    void setNextTickProgramReqCb(TFunc&& func)
    {
        m_tickProgramCb = std::forward<TFunc>(func);
    }

    template <typename TFunc>
    void setSendDataReqCb(TFunc&& func)
    {
        m_sendDataCb = std::forward<TFunc>(func);
    }

    GatewayImpl() = default;
    ~GatewayImpl() = default;

    void setAdvertisePeriod(std::uint16_t value)
    {
        m_advertisePeriod = value;
        refresh();
    }

    void setGatewayId(std::uint8_t value)
    {
        m_gwId = value;
        refresh();
    }

    bool start();
    void stop();
    void tick();

private:

    void refresh();
    void sendAndReprogram();

    static const std::size_t AdvertiseLength = 5;

    NextTickProgramReqCb m_tickProgramCb;
    SendDataReqCb m_sendDataCb;
    std::uint16_t m_advertisePeriod = 0;
    std::uint8_t m_gwId = 0;
    bool m_running = false;
    std::array<std::uint8_t, AdvertiseLength> m_outputData;
    ProtStack m_protStack;
};

}  // namespace gateway

}  // namespace mqttsn
