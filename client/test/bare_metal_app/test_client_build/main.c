//
// Copyright 2014 - 2020 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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

void gwStatusReport(void* data, unsigned char gwId, MqttsnGwStatus status)
{
}

void msgReport(void* data, const MqttsnMessageInfo* msgInfo)
{
}

void willUpdated(void* data, MqttsnAsyncOpStatus status)
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
    MqttsnWillInfo willInfo;


    mqttsn_test_bare_metal_client_set_next_tick_program_callback(client, &programNextTick, NULL);
    mqttsn_test_bare_metal_client_set_cancel_next_tick_wait_callback(client, &cancelTick, NULL);
    mqttsn_test_bare_metal_client_set_send_output_data_callback(client, &sendOutputData, NULL);
    mqttsn_test_bare_metal_client_set_gw_disconnect_report_callback(client, &gwDisconnected, NULL);
    mqttsn_test_bare_metal_client_set_gw_status_report_callback(client, &gwStatusReport, NULL);
    mqttsn_test_bare_metal_client_set_message_report_callback(client, &msgReport, NULL);
    if (mqttsn_test_bare_metal_client_start(client) == 0) {
        return -1;
    }

    mqttsn_test_bare_metal_client_process_data(client, from, SeqSize);
    mqttsn_test_bare_metal_client_tick(client);
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

    mqttsn_test_bare_metal_client_set_retry_period(client, 10);
    mqttsn_test_bare_metal_client_set_retry_count(client, 3);
    mqttsn_test_bare_metal_client_set_broadcast_radius(client, 0);
    mqttsn_test_bare_metal_client_set_searchgw_enabled(client, true);
    mqttsn_test_bare_metal_client_search_gw(client);
    mqttsn_test_bare_metal_client_discard_gw(client, 0);
    mqttsn_test_bare_metal_client_discard_all_gw(client);
    mqttsn_test_bare_metal_client_reconnect(client, &connectComplete, NULL);

    mqttsn_test_bare_metal_client_will_update(client, &willInfo, &willUpdated, NULL);
    mqttsn_test_bare_metal_client_will_topic_update(client, Topic, MqttsnQoS_ExactlyOnceDelivery, false, &willUpdated, NULL);
    mqttsn_test_bare_metal_client_will_msg_update(client, Data, DataSize, &willUpdated, NULL);

    mqttsn_test_bare_metal_client_disconnect(client, &disconnectComplete, NULL);
    mqttsn_test_bare_metal_client_stop(client);
    mqttsn_test_bare_metal_client_free(client);
    while (true) {};
    return 0;
}
