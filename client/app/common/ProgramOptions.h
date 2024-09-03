//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/program_options.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace cc_mqttsn_client_app
{

class ProgramOptions
{
public:
    using OptDesc = boost::program_options::options_description;

    enum ConnectionType
    {
        ConnectionType_Udp,
        ConnectionType_ValuesLimit
    };

    void addCommon();
    void addNetwork();
    void addDiscover();
    void addConnect();
    void addWill();
    void addEncapsulate();
    void addPublish();
    void addSubscribe();

    void printHelp();

    bool parseArgs(int argc, const char* argv[]);

    // Common options
    bool helpRequested() const;
    bool verbose() const;
    ConnectionType connectionType() const;

    // Network Options
    std::string networkAddress() const;
    std::string networkBroadcastAddress() const;
    std::uint16_t networkRemotePort() const;    
    std::uint16_t networkLocalPort() const;    

    // Discover Options
    bool discoverExitOnFirst() const;
    unsigned discoverTimeout() const;

    // Connect Options
    std::string connectClientId() const;
    unsigned connectKeepAlive() const;
    bool connectNoCleanSession() const;    

    // Will Options
    std::string willTopic() const;
    std::string willMessage() const;
    unsigned willQos() const;  

    // Forwarder Encapsulation options
    std::string fwdEncNodeId() const;     

    // Publish Options
    std::string pubTopic() const;
    std::uint16_t pubTopicId() const;
    std::string pubMessage() const;
    unsigned pubQos() const; 
    bool pubRetain() const; 
    bool pubNoDisconnect() const;   
    unsigned pubCount() const; 
    unsigned pubDelay() const;

    // Subscribe Options
    std::vector<std::string> subTopics() const;
    std::vector<std::uint16_t> subTopicIds() const;
    unsigned subQos() const; 
    bool subNoRetained() const;
    bool subBinary() const;

private:
    boost::program_options::variables_map m_vm;
    OptDesc m_desc;
};

} // namespace cc_mqttsn_client_app
