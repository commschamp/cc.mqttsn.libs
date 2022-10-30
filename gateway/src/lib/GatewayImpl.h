//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <algorithm>

#include "cc_mqttsn/gateway/Gateway.h"
#include "cc_mqttsn/Message.h"
#include "cc_mqttsn/frame/Frame.h"

namespace cc_mqttsn
{

namespace gateway
{

class GatewayImpl
{
public:
    using NextTickProgramReqCb = Gateway::NextTickProgramReqCb;
    using SendDataReqCb = Gateway::SendDataReqCb;

    using Message =
        cc_mqttsn::Message<
        comms::option::IdInfoInterface,
            comms::option::WriteIterator<std::uint8_t*>,
            comms::option::LengthInfoInterface
        >;

    using AdvertiseMsg = cc_mqttsn::message::Advertise<Message>;
    using AllMsgs = std::tuple<AdvertiseMsg>;
    using ProtStack = cc_mqttsn::frame::Frame<Message, AllMsgs>;

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

}  // namespace cc_mqttsn
