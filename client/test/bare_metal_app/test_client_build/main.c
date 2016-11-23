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
#include <stdint.h>
#include <stddef.h>

#include "test_bare_metal_client.h"

void interruptHandler()
{
}

void programNextTick(void* data, unsigned ms)
{
}

unsigned cancelTick(void* data)
{
    return 0U;
}

void connectComplete(void* data, MqttsnAsyncOpStatus status)
{
}

void disconnectComplete(void* data, MqttsnAsyncOpStatus status)
{
}

void gwDisconnected(void* data)
{
}

void sendOutputData(void* data, const unsigned char* buf, unsigned bufSize, bool broadcast)
{
}

void publishCallback(void* data, MqttsnAsyncOpStatus status)
{
}

void subscribeCallback(void* data, MqttsnAsyncOpStatus status, MqttsnQoS qos)
{
}

void unsubscribeCallback(void* data, MqttsnAsyncOpStatus status)
{
}

int main(int argc, const char** argv)
{
    static const unsigned char Seq[] = {0x00, 0xf0};
    static const unsigned SeqSize = sizeof(Seq)/sizeof(Seq[0]);
    static const char* Topic = "/this/is/topic";
    static const unsigned char Data[] = {
        0x00, 0x01, 0x02, 0x03
    };
    static const unsigned DataSize = sizeof(Data)/sizeof(Data[0]);
    const unsigned char* from = &Seq[0];
    MqttsnClientHandle client = mqttsn_test_bare_metal_client_new();


    mqttsn_test_bare_metal_client_set_next_tick_program_callback(client, &programNextTick, NULL);
    mqttsn_test_bare_metal_client_set_cancel_next_tick_wait_callback(client, &cancelTick, NULL);
    mqttsn_test_bare_metal_client_set_send_output_data_callback(client, &sendOutputData, NULL);
    mqttsn_test_bare_metal_client_set_gw_disconnect_report_callback(client, &gwDisconnected, NULL);
    if (mqttsn_test_bare_metal_client_start(client) == 0) {
        return -1;
    }

    mqttsn_test_bare_metal_client_process_data(client, from, SeqSize);
    mqttsn_test_bare_metal_client_tick(client, 10);
    mqttsn_test_bare_metal_client_connect(client, "my_id", 60, true, NULL, &connectComplete, NULL);
    mqttsn_test_bare_metal_client_publish(
        client,
        Topic,
        &Data[0],
        DataSize,
        MqttsnQoS_ExactlyOnceDelivery,
        false,
        &publishCallback,
        NULL);

    mqttsn_test_bare_metal_client_publish_id(
        client,
        0x1234,
        &Data[0],
        DataSize,
        MqttsnQoS_ExactlyOnceDelivery,
        false,
        &publishCallback,
        NULL);

    mqttsn_test_bare_metal_client_subscribe(
        client,
        Topic,
        MqttsnQoS_ExactlyOnceDelivery,
        &subscribeCallback,
        NULL);

    mqttsn_test_bare_metal_client_subscribe_id(
        client,
        0x1111,
        MqttsnQoS_ExactlyOnceDelivery,
        &subscribeCallback,
        NULL);

    mqttsn_test_bare_metal_client_cancel(client);

    mqttsn_test_bare_metal_client_unsubscribe(
        client,
        Topic,
        &unsubscribeCallback,
        NULL);

    mqttsn_test_bare_metal_client_unsubscribe_id(
        client,
        0x1111,
        &unsubscribeCallback,
        NULL);

    mqttsn_test_bare_metal_client_sleep(
        client,
        10000,
        &unsubscribeCallback,
        NULL);

    mqttsn_test_bare_metal_client_check_messages(
        client,
        &unsubscribeCallback,
        NULL);

    mqttsn_test_bare_metal_client_disconnect(client, &disconnectComplete, NULL);
    mqttsn_test_bare_metal_client_free(client);
    while (true) {};
    return 0;
}
