#include "UnitTestDefaultBase.h"

#include "client.h"

const UnitTestDefaultBase::LibFuncs& UnitTestDefaultBase::getFuncs()
{
    static LibFuncs funcs;
    funcs.m_alloc = &cc_mqttsn_client_alloc;
    funcs.m_free = &cc_mqttsn_client_free;
    funcs.m_tick = &cc_mqttsn_client_tick;
    funcs.m_process_data = &cc_mqttsn_client_process_data;
    funcs.m_set_default_retry_period = &cc_mqttsn_client_set_default_retry_period;
    funcs.m_get_default_retry_period = &cc_mqttsn_client_get_default_retry_period;
    funcs.m_set_default_retry_count = &cc_mqttsn_client_set_default_retry_count;
    funcs.m_get_default_retry_count = &cc_mqttsn_client_get_default_retry_count;
    funcs.m_set_default_broadcast_radius = &cc_mqttsn_client_set_default_broadcast_radius;
    funcs.m_get_default_broadcast_radius = &cc_mqttsn_client_get_default_broadcast_radius;
    funcs.m_get_available_gateways_count = &cc_mqttsn_client_get_available_gateways_count;
    funcs.m_init_gateway_info = &cc_mqttsn_client_init_gateway_info;
    funcs.m_get_available_gateway_info = &cc_mqttsn_client_get_available_gateway_info;
    funcs.m_set_available_gateway_info = &cc_mqttsn_client_set_available_gateway_info;
    funcs.m_discard_available_gateway_info = &cc_mqttsn_client_discard_available_gateway_info;
    funcs.m_discard_all_gateway_infos = &cc_mqttsn_client_discard_all_gateway_infos;
    funcs.m_set_default_gw_adv_duration = &cc_mqttsn_client_set_default_gw_adv_duration;
    funcs.m_get_default_gw_adv_duration = &cc_mqttsn_client_get_default_gw_adv_duration;
    funcs.m_set_allowed_adv_losses = &cc_mqttsn_client_set_allowed_adv_losses;
    funcs.m_get_allowed_adv_losses = &cc_mqttsn_client_get_allowed_adv_losses;
    funcs.m_search_prepare = &cc_mqttsn_client_search_prepare;
    funcs.m_search_set_retry_period = &cc_mqttsn_client_search_set_retry_period;
    funcs.m_search_get_retry_period = &cc_mqttsn_client_search_get_retry_period;
    funcs.m_search_set_retry_count = &cc_mqttsn_client_search_set_retry_count;
    funcs.m_search_get_retry_count = &cc_mqttsn_client_search_get_retry_count;
    funcs.m_search_set_broadcast_radius = &cc_mqttsn_client_search_set_broadcast_radius;
    funcs.m_search_get_broadcast_radius = &cc_mqttsn_client_search_get_broadcast_radius;
    funcs.m_search_send = &cc_mqttsn_client_search_send;
    funcs.m_search_cancel = &cc_mqttsn_client_search_cancel;
    funcs.m_search = &cc_mqttsn_client_search;
    funcs.m_connect_prepare = &cc_mqttsn_client_connect_prepare;
    funcs.m_connect_set_retry_period = &cc_mqttsn_client_connect_set_retry_period;
    funcs.m_connect_get_retry_period = &cc_mqttsn_client_connect_get_retry_period;
    funcs.m_connect_set_retry_count = &cc_mqttsn_client_connect_set_retry_count;
    funcs.m_connect_get_retry_count = &cc_mqttsn_client_connect_get_retry_count;
    funcs.m_connect_init_config = &cc_mqttsn_client_connect_init_config;
    funcs.m_connect_config = &cc_mqttsn_client_connect_config;
    funcs.m_connect_init_config_will = &cc_mqttsn_client_connect_init_config_will;
    funcs.m_connect_config_will = &cc_mqttsn_client_connect_config_will;
    funcs.m_connect_send = &cc_mqttsn_client_connect_send;
    funcs.m_connect_cancel = &cc_mqttsn_client_connect_cancel;
    funcs.m_connect = &cc_mqttsn_client_connect;
    funcs.m_is_connected = &cc_mqttsn_client_is_connected;
    funcs.m_disconnect_prepare = &cc_mqttsn_client_disconnect_prepare;
    funcs.m_disconnect_set_retry_period = &cc_mqttsn_client_disconnect_set_retry_period;
    funcs.m_disconnect_get_retry_period = &cc_mqttsn_client_disconnect_get_retry_period;
    funcs.m_disconnect_set_retry_count = &cc_mqttsn_client_disconnect_set_retry_count;
    funcs.m_disconnect_get_retry_count = &cc_mqttsn_client_disconnect_get_retry_count;
    funcs.m_disconnect_send = &cc_mqttsn_client_disconnect_send;
    funcs.m_disconnect_cancel = &cc_mqttsn_client_disconnect_cancel;
    funcs.m_disconnect = &cc_mqttsn_client_disconnect;    

    funcs.m_set_next_tick_program_callback = &cc_mqttsn_client_set_next_tick_program_callback;
    funcs.m_set_cancel_next_tick_wait_callback = &cc_mqttsn_client_set_cancel_next_tick_wait_callback;
    funcs.m_set_send_output_data_callback = &cc_mqttsn_client_set_send_output_data_callback;
    funcs.m_set_gw_status_report_callback = &cc_mqttsn_client_set_gw_status_report_callback;
    funcs.m_set_gw_disconnect_report_callback = &cc_mqttsn_client_set_gw_disconnect_report_callback;
    funcs.m_set_message_report_callback = &cc_mqttsn_client_set_message_report_callback;
    funcs.m_set_error_log_callback = &cc_mqttsn_client_set_error_log_callback;
    funcs.m_set_gwinfo_delay_request_callback = &cc_mqttsn_client_set_gwinfo_delay_request_callback;
    
    return funcs;
}
