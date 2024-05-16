//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewayIoClientSocket.h"

#include <type_traits>

namespace cc_mqttsn_gateway_app
{

GatewayIoClientSocket::~GatewayIoClientSocket() = default;

bool GatewayIoClientSocket::start()
{
    if (!m_dataReportCb) {
        m_logger.error() << "Not all callbacks are set for GatewayIoClientSocket" << std::endl;
        return false;
    }

    return startImpl();
}

} // namespace cc_mqttsn_gateway_app
