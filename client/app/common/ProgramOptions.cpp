//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ProgramOptions.h"

#include <iostream>

namespace po = boost::program_options;

namespace cc_mqttsn_client_app
{

namespace 
{

constexpr std::uint16_t DefaultPort = 1883U;
constexpr unsigned DefaultKeepAlive = 60;

} // namespace 
    

void ProgramOptions::addCommon()
{
    po::options_description opts("Common Options");
    opts.add_options()
        ("help,h", "Display help message")
        ("verbose,v", "Verbose output")
    ;    

    m_desc.add(opts);
}

void ProgramOptions::addNetwork()
{
    po::options_description opts("Network Options");
    opts.add_options()
        ("network-gateway,g", po::value<std::string>()->default_value("127.0.0.1"), "Gateway address to connect to")
        ("network-broadcast,b", po::value<std::string>()->default_value("255.255.255.255"), "Address to broadcast to")
        ("network-port,p", po::value<std::uint16_t>()->default_value(DefaultPort), "Network remote port")
        ("network-local-port,P", po::value<std::uint16_t>()->default_value(0), "Network local port")
    ;    

    m_desc.add(opts);
}

void ProgramOptions::addDiscover()
{
    po::options_description opts("Gateway Discover Options");
    opts.add_options()
        ("discover-exit-on-first,f", "Exit after first discovered gateway.")
        ("discover-timeout,t", po::value<unsigned>()->default_value(0), "Terminate after specified number of seconds, 0 means infinite");
    ;    

    m_desc.add(opts);
}

void ProgramOptions::addConnect()
{
    po::options_description opts("Connect Options");
    opts.add_options()
        ("connect-client-id,c", po::value<std::string>()->default_value(std::string()), "Client ID")
        ("connect-keep-alive,k", po::value<unsigned>()->default_value(DefaultKeepAlive), "Protocol \"keep alive\" configuration")
        ("connect-no-clean-session,l", "Do not force clean session upon connection")
    ;    

    m_desc.add(opts);
}

void ProgramOptions::addWill()
{
    po::options_description opts("Will Options");
    opts.add_options()
        ("will-topic", po::value<std::string>()->default_value(std::string()), "Will topic, when not provided means no will")
        ("will-message", po::value<std::string>()->default_value(std::string()), "Will message data, use \"\\x\" prefix before hex value of each byte for binary string")
        ("will-qos", po::value<unsigned>()->default_value(0U), "Will QoS value")
    ;    

    m_desc.add(opts);
}

void ProgramOptions::addPublish()
{
    po::options_description opts("Publish Options");
    opts.add_options()
        ("pub-topic,t", po::value<std::string>()->default_value(std::string()), "Publish topic, must be empty when topic ID is not 0")
        ("pub-topic-id,i", po::value<std::uint16_t>()->default_value(0U), "Publish topic id, must be 0 when topic is specified")
        ("pub-message,m", po::value<std::string>()->default_value(std::string()), "Publish message data, use \"\\x\" prefix before hex value of each byte for binary string")
        ("pub-qos,q", po::value<unsigned>()->default_value(0U), "Publish QoS value")
        ("pub-retain", "Publish retained message")
    ;    

    m_desc.add(opts);
}

void ProgramOptions::printHelp()
{
    std::cout << m_desc << std::endl;
}

bool ProgramOptions::parseArgs(int argc, const char* argv[])
{
    po::store(po::parse_command_line(argc, argv, m_desc), m_vm);
    po::notify(m_vm);  

    return true;
}

bool ProgramOptions::helpRequested() const
{
    return m_vm.count("help") > 0U;
}

bool ProgramOptions::verbose() const
{
    return m_vm.count("verbose") > 0U;
}

ProgramOptions::ConnectionType ProgramOptions::connectionType() const
{
    // Hardcoded for now
    return ConnectionType_Udp;
}

std::string ProgramOptions::networkAddress() const
{
    return m_vm["network-gateway"].as<std::string>();
}

std::string ProgramOptions::networkBroadcastAddress() const
{
    return m_vm["network-broadcast"].as<std::string>();
}

std::uint16_t ProgramOptions::networkRemotePort() const
{
    return m_vm["network-port"].as<std::uint16_t>();
}

std::uint16_t ProgramOptions::networkLocalPort() const
{
    return m_vm["network-local-port"].as<std::uint16_t>();
}

bool ProgramOptions::discoverExitOnFirst() const
{
    return m_vm.count("discover-exit-on-first") > 0U;
}

unsigned ProgramOptions::discoverTimeout() const
{
    return m_vm["discover-timeout"].as<unsigned>();
}

std::string ProgramOptions::connectClientId() const
{
    return m_vm["connect-client-id"].as<std::string>();
}

unsigned ProgramOptions::connectKeepAlive() const
{
    return m_vm["connect-keep-alive"].as<unsigned>();
}

bool ProgramOptions::connectNoCleanSession() const
{
    return m_vm.count("connect-no-clean-session") > 0U;
}

std::string ProgramOptions::willTopic() const
{
    return m_vm["will-topic"].as<std::string>();
}

std::string ProgramOptions::willMessage() const
{
    return m_vm["will-message"].as<std::string>();
}

unsigned ProgramOptions::willQos() const
{
    return m_vm["will-qos"].as<unsigned>();
}

std::string ProgramOptions::pubTopic() const
{
    return m_vm["pub-topic"].as<std::string>();
}

std::uint16_t ProgramOptions::pubTopicId() const
{
    return m_vm["pub-topic-id"].as<std::uint16_t>();
}

std::string ProgramOptions::pubMessage() const
{
    return m_vm["pub-message"].as<std::string>();
}

unsigned ProgramOptions::pubQos() const
{
    return m_vm["pub-qos"].as<unsigned>();
}

bool ProgramOptions::pubRetain() const
{
    return m_vm.count("pub-retain") > 0U;
}

} // namespace cc_mqttsn_client_app
