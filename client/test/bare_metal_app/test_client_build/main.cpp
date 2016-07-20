//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
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


// This function is required by common startup code
#include <type_traits>

#include "test_bare_metal_client.h"

extern "C"
void interruptHandler()
{
}

void programNextTick(void* data, unsigned duration)
{
    static_cast<void>(data);
    static_cast<void>(duration);
}

unsigned cancelTick(void* data)
{
    static_cast<void>(data);
    return 0U;
}

void connectStatus(void* data, MqttsnConnectStatus status)
{
    static_cast<void>(data);
    static_cast<void>(status);
}

void sendOutputData(void* data, const unsigned char* buf, unsigned bufLen, bool broadcast)
{
    static_cast<void>(data);
    static_cast<void>(buf);
    static_cast<void>(bufLen);
    static_cast<void>(broadcast);
}


int main(int argc, const char** argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    auto* client = mqttsn_test_bare_metal_client_new();
    mqttsn_test_bare_metal_client_set_gw_advertise_period(client, 15 * 60 * 1000);
    mqttsn_test_bare_metal_client_set_next_tick_program_callback(client, &programNextTick, nullptr);
    mqttsn_test_bare_metal_client_set_cancel_next_tick_wait_callback(client, &cancelTick, nullptr);
    mqttsn_test_bare_metal_client_set_send_output_data_callback(client, &sendOutputData, nullptr);
    if (mqttsn_test_bare_metal_client_start(client) == 0) {
        return -1;
    }

    static const unsigned char Seq[] = {0x00, 0xf0};
    static const std::size_t SeqSize = std::extent<decltype(Seq)>::value;

    const unsigned char* from = &Seq[0];
    mqttsn_test_bare_metal_client_process_data(client, from, SeqSize);
    mqttsn_test_bare_metal_client_tick(client, 10);
    mqttsn_test_bare_metal_client_connect(client, "my_id", 60, true, nullptr, &connectStatus, nullptr);
    mqttsn_test_bare_metal_client_free(client);
    while (true) {};
    return 0;
}
