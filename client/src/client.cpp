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
#include "mqttsn/client/Message.h"
#include "mqttsn/protocol/option.h"

extern "C"
{
void* mqttsn_client_new();
void mqttsn_client_process_data(const unsigned char** from, unsigned len);
}

namespace
{

typedef std::tuple<
        mqttsn::protocol::option::ClientIdStaticStorageSize<21>,
        mqttsn::protocol::option::GwAddStaticStorageSize<4>,
        mqttsn::protocol::option::TopicNameStaticStorageSize<128>,
        mqttsn::protocol::option::MessageDataStaticStorageSize<256>
    > ProtocolOptions;

typedef mqttsn::client::AllMessages<mqttsn::protocol::ParsedOptions<ProtocolOptions> > AllMsgs;

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

typedef mqttsn::client::Client<mqttsn::client::MsgHandler, ProtocolOptions> MqttsnClient;
MqttsnClient& getClient()
{
    static MqttsnClient Client;
    return Client;
}

}  // namespace

void* mqttsn_client_new()
{
    return &(getClient());
}

void mqttsn_client_process_data(const unsigned char** from, unsigned len)
{
    getClient().processData(*from, len);
}

