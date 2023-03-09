//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayWrapper.h"

#include <iostream>
#include <algorithm>

namespace cc_mqttsn_gateway
{

namespace app
{

namespace udp
{

GatewayWrapper::GatewayWrapper(const Config& config)
  : m_config(config)
{
    connect(
        &m_timer, SIGNAL(timeout()),
        this, SLOT(tickTimeout()));
}

bool GatewayWrapper::start(SendDataReqCb&& sendCb)
{
    m_gw.setAdvertisePeriod(m_config.advertisePeriod());
    m_gw.setGatewayId(m_config.gatewayId());
    m_gw.setNextTickProgramReqCb(
        [this](unsigned ms)
        {
            m_timer.setSingleShot(true);
            m_timer.start(ms);
        });

    m_gw.setSendDataReqCb(std::move(sendCb));
    return m_gw.start();
}

void GatewayWrapper::tickTimeout()
{
    m_gw.tick();
}

}  // namespace udp

}  // namespace app

}  // namespace cc_mqttsn_gateway


