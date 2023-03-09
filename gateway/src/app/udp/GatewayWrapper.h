//
// Copyright 2016 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "cc_mqttsn_gateway/Config.h"
#include "cc_mqttsn_gateway/Gateway.h"

namespace cc_mqttsn_gateway
{

namespace app
{

namespace udp
{

class GatewayWrapper : public QObject
{
    Q_OBJECT
public:
    typedef unsigned short PortType;

    explicit GatewayWrapper(const Config& config);

    typedef cc_mqttsn_gateway::Gateway::SendDataReqCb SendDataReqCb;
    bool start(SendDataReqCb&& sendCb);

private slots:
    void tickTimeout();

private:
    const Config& m_config;
    cc_mqttsn_gateway::Gateway m_gw;
    QTimer m_timer;
};

}  // namespace udp

}  // namespace app

}  // namespace cc_mqttsn_gateway
