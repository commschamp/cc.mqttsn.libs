//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ProgramOptions.h"

#include <fstream>
#include <iosfwd>

namespace cc_mqttsn_client_afl_fuzz
{

class Logger
{
public:
    Logger();
    bool open(const ProgramOptions& opts);
    std::ostream& debugLog();
    std::ostream& infoLog();
    std::ostream& errorLog();
    void flush();

private:
    std::ostream* m_out = nullptr;
    std::ofstream m_fileStream;
};

} // namespace cc_mqttsn_client_afl_fuzz
