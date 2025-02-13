//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Logger.h"

#include <iostream>

namespace cc_mqttsn_client_afl_fuzz
{

Logger::Logger() : 
    m_out(&std::cout)
{
}

bool Logger::open(const ProgramOptions& opts)
{
    if (opts.hasLogFile()) {
        m_fileStream.open(opts.logFile(), std::ios_base::app);
        if (!m_fileStream) {
            std::cerr << "Failed to open log file: " << opts.logFile() << std::endl;
            return false;
        }

        m_out = &m_fileStream;
    } 
    return true;
}

std::ostream& Logger::debugLog()
{
    return *m_out << "DEBUG: ";
}
std::ostream& Logger::infoLog()
{
    return *m_out << "INFO: ";
}

std::ostream& Logger::errorLog()
{
    return *m_out << "ERROR: ";
}

void Logger::flush()
{
    m_out->flush();
}

} // namespace cc_mqttsn_client_afl_fuzz
