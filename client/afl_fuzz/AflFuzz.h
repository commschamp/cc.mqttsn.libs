//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Logger.h"
#include "ProgramOptions.h"

#include <memory>

namespace cc_mqttsn_client_afl_fuzz
{

class AflFuzzImpl;
class AflFuzz
{
public:
    AflFuzz(const ProgramOptions& opts, Logger& logger);
    ~AflFuzz();

    bool init();
    void run();
private:
    std::unique_ptr<AflFuzzImpl> m_impl;
};

} // namespace cc_mqttsn_client_afl_fuzz
