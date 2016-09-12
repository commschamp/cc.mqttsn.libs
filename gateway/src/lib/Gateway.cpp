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

#include "mqttsn/gateway/Gateway.h"

#include "GatewayImpl.h"

namespace mqttsn
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

}  // namespace mqttsn


