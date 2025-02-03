//
// Copyright 2023 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ProgramOptions.h"

#include <algorithm>
#include <iostream>

namespace po = boost::program_options;

namespace cc_mqttsn_client_afl_fuzz
{

ProgramOptions::ProgramOptions()
{
    po::options_description commonOpts("Common Options");
    commonOpts.add_options()
        ("help,h", "Display help message")
        ("verbose,v", "Verbose output")
        ("log-file,f", po::value<std::string>(), "Output log file")
        ("gen-input,g", po::value<std::string>()->default_value(std::string()), "Generate fuzz input file")
    ;  

    po::options_description connectOpts("Connect Options");
    connectOpts.add_options()
        ("client-id,i", po::value<std::string>()->default_value("afl_client"), "Client ID")
        ("will-topic", po::value<std::string>()->default_value(std::string()), 
            "Will topic to connect with")
        ("will-data", po::value<std::string>()->default_value(std::string("data")), 
            "Will data to connect with, applicable only if will-topic is not empty")            
    ;  

    po::options_description subOpts("Subscribe Options");
    subOpts.add_options()
        ("sub-topic,t", po::value<StringsList>(), "Subscribe topic filters. Can be used multiple times. "
            "If not specified, single subscribe to \"#\" is assumed")
    ;    

    po::options_description pubOpts("Publish Options");
    subOpts.add_options()
        ("min-pub-count", po::value<unsigned>()->default_value(3U), 
            "Number of successful publish counts before unsubscribing to previously subscribed topics")
    ;            

    m_desc.add(commonOpts);
    m_desc.add(connectOpts);  
    m_desc.add(subOpts);
}

bool ProgramOptions::parseArgs(int argc, const char* argv[])
{
    po::store(po::parse_command_line(argc, argv, m_desc), m_vm);
    po::notify(m_vm);  

    return true;
}

void ProgramOptions::printHelp()
{
    std::cout << m_desc << std::endl;
}

bool ProgramOptions::helpRequested() const
{
    return m_vm.count("help") > 0U;
}

bool ProgramOptions::verbose() const
{
    return m_vm.count("verbose") > 0U;
}

bool ProgramOptions::hasLogFile() const
{
    return m_vm.count("log-file") > 0U;
}

std::string ProgramOptions::logFile() const
{
    return m_vm["log-file"].as<std::string>();
}

std::string ProgramOptions::genInputFile() const
{
    return m_vm["gen-input"].as<std::string>();
}

std::string ProgramOptions::clientId() const
{
    return m_vm["client-id"].as<std::string>();
}

std::string ProgramOptions::willTopic() const
{
    return m_vm["will-topic"].as<std::string>();
}

std::string ProgramOptions::willData() const
{
    return m_vm["will-data"].as<std::string>();
}

ProgramOptions::StringsList ProgramOptions::subTopics() const
{
    auto values = stringListOpts("sub-topic");
    if (values.empty()) {
        values.push_back("#");
    }
    
    return values;
}

unsigned ProgramOptions::minPubCount() const
{
    return m_vm["min-pub-count"].as<unsigned>();
}

ProgramOptions::StringsList ProgramOptions::stringListOpts(const std::string& name) const
{
    StringsList result;
    if (m_vm.count(name) > 0) {
        result = m_vm[name].as<StringsList>();
    }

    return result;    
}

} // namespace cc_mqttsn_client_afl_fuzz
