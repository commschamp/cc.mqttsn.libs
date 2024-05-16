//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "cc_mqttsn_gateway/Config.h"

#include <fstream>
#include <iostream>
#include <memory>

namespace cc_mqttsn_gateway_app
{

class GatewayLogger
{
public:
    GatewayLogger();

    void configure(const cc_mqttsn_gateway::Config& config);

    std::ostream& error();
    std::ostream& info();
    std::ostream& warning();
    
private:
    std::unique_ptr<std::ofstream> m_fstream;
    std::ostream* m_out = nullptr;
};

} // namespace cc_mqttsn_gateway_app
