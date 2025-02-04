//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/program_options.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace cc_mqttsn_client_afl_fuzz
{

class ProgramOptions
{
public:
    using OptDesc = boost::program_options::options_description;
    using StringsList = std::vector<std::string>;
    using UnsignedsList = std::vector<unsigned>;

    ProgramOptions();

    void printHelp();

    bool parseArgs(int argc, const char* argv[]);

    // Common options
    bool helpRequested() const;
    bool verbose() const;
    bool hasLogFile() const;
    std::string logFile() const;
    std::string genInputFile() const;

    // Connect options
    std::string clientId() const;
    std::string willTopic() const;
    std::string willData() const;

    // Subscribe options
    StringsList subTopics() const;

    // Publish options
    unsigned minPubCount() const;    

    // Will update options
    std::string willUpdTopic() const;
    std::string willUpdData() const;

private:
    StringsList stringListOpts(const std::string& name) const;

    boost::program_options::variables_map m_vm;
    OptDesc m_desc;
};

} // namespace cc_mqttsn_client_afl_fuzz
