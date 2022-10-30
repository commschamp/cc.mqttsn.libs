//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "cc_mqttsn/gateway/Gateway.h"

#include "GatewayImpl.h"

namespace cc_mqttsn
{

namespace gateway
{

Gateway::Gateway()
  : m_pImpl(new GatewayImpl)
{
}

Gateway::~Gateway() = default;

void Gateway::setNextTickProgramReqCb(NextTickProgramReqCb&& func)
{
    m_pImpl->setNextTickProgramReqCb(std::move(func));
}

void Gateway::setSendDataReqCb(SendDataReqCb&& func)
{
    m_pImpl->setSendDataReqCb(std::move(func));
}

void Gateway::setAdvertisePeriod(std::uint16_t value)
{
    m_pImpl->setAdvertisePeriod(value);
}

void Gateway::setGatewayId(std::uint8_t value)
{
    m_pImpl->setGatewayId(value);
}

bool Gateway::start()
{
    return m_pImpl->start();
}

void Gateway::stop()
{
    m_pImpl->stop();
}

void Gateway::tick()
{
    m_pImpl->tick();
}

}  // namespace gateway

}  // namespace cc_mqttsn


