//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "mqttsn/client/Client.h"
#include "mqttsn/client/ClientMgr.h"
#include "mqttsn/client/Message.h"
#include "mqttsn/client/option.h"
#include "mqttsn/client/ParsedOptions.h"
#include "mqttsn/protocol/option.h"

extern "C"
{
void* mqttsn_client_new();
void mqttsn_client_free(void* client);
void mqttsn_client_process_data(void* client, const unsigned char** from, unsigned len);
void mqttsn_client_tick(void* client, unsigned ms);
}

namespace
{

typedef std::tuple<
        mqttsn::protocol::option::ClientIdStaticStorageSize<21>,
        mqttsn::protocol::option::GwAddStaticStorageSize<4>,
        mqttsn::protocol::option::TopicNameStaticStorageSize<128>,
        mqttsn::protocol::option::MessageDataStaticStorageSize<256>
    > ProtocolOptions;

typedef mqttsn::protocol::ParsedOptions<ProtocolOptions> ParsedProtocolOptions;

typedef mqttsn::client::AllMessages<ParsedProtocolOptions> AllMsgs;

typedef std::tuple<
    mqttsn::client::option::ClientsAllocLimit<1>,
    mqttsn::client::option::TrackedGatewaysLimit<1>
> ClientOptions;

typedef mqttsn::client::ParsedOptions<ClientOptions> ParsedClientOptions;

}  // namespace


namespace mqttsn
{

namespace client
{

class MsgHandler : public comms::GenericHandler<Message, AllMsgs>
{
};

}  // namespace client

}  // namespace mqttsn


namespace
{

typedef mqttsn::client::Client<mqttsn::client::MsgHandler, ParsedClientOptions, ParsedProtocolOptions> MqttsnClient;
typedef mqttsn::client::ClientMgr<MqttsnClient, ParsedClientOptions> MqttsnClientMgr;

MqttsnClientMgr& getClientMgr()
{
    static MqttsnClientMgr Mgr;
    return Mgr;
}

}  // namespace

void* mqttsn_client_new()
{
    auto client = getClientMgr().alloc();
    return client.release();
}

void mqttsn_client_free(void* client)
{
    getClientMgr().free(reinterpret_cast<MqttsnClient*>(client));
}

void mqttsn_client_process_data(
    void* client,
    const unsigned char** from,
    unsigned len)
{
    auto* clientObj = reinterpret_cast<MqttsnClient*>(client);
    clientObj->processData(*from, len);
}

void mqttsn_client_tick(void* client, unsigned ms)
{
    auto* clientObj = reinterpret_cast<MqttsnClient*>(client);
    clientObj->tick(ms);
}

