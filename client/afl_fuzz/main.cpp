//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "AflFuzz.h"
#include "Logger.h"
#include "ProgramOptions.h"


int main(int argc, const char* argv[]) 
{
    cc_mqttsn_client_afl_fuzz::ProgramOptions opts;
    opts.parseArgs(argc, argv);

    if (opts.helpRequested()) {
        opts.printHelp();
        return -1;
    }

    cc_mqttsn_client_afl_fuzz::Logger logger;
    if (!logger.open(opts)) {
        return -1;
    }

    cc_mqttsn_client_afl_fuzz::AflFuzz fuzz(opts, logger);
    if (!fuzz.init()) {
        return -1;
    }

    fuzz.run();
    return 0;
}