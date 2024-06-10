#pragma once

#include "cc_mqttsn_client/common.h"

#include "UnitTestProtocolDefs.h"

#include <list>

class UnitTestCommonBase
{
public:
    using UnitTestData = std::vector<std::uint8_t>;

    struct LibFuncs
    {
        CC_MqttsnClientHandle (*m_alloc)() = nullptr;
        void (*m_free)(CC_MqttsnClientHandle) = nullptr;
        void (*m_tick)(CC_MqttsnClientHandle, unsigned) = nullptr;
        void (*m_process_data)(CC_MqttsnClientHandle, const unsigned char*, unsigned) = nullptr;
        CC_MqttsnErrorCode (*m_set_default_retry_period)(CC_MqttsnClientHandle, unsigned) = nullptr;
        unsigned (*m_get_default_retry_period)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_default_retry_count)(CC_MqttsnClientHandle, unsigned) = nullptr;
        unsigned (*m_get_default_retry_count)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_default_broadcast_radius)(CC_MqttsnClientHandle, unsigned) = nullptr;
        unsigned (*m_get_default_broadcast_radius)(CC_MqttsnClientHandle) = nullptr;        
        unsigned (*m_get_available_gateways_count)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_get_available_gateway_info)(CC_MqttsnClientHandle, unsigned, CC_MqttsnGatewayInfo*) = nullptr;
        CC_MqttsnErrorCode (*m_set_available_gateway_info)(CC_MqttsnClientHandle, const CC_MqttsnGatewayInfo*) = nullptr;
        CC_MqttsnErrorCode (*m_discard_available_gateway_info)(CC_MqttsnClientHandle, unsigned char) = nullptr;
        void (*m_discard_all_gateway_infos)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_default_gw_adv_duration)(CC_MqttsnClientHandle, unsigned) = nullptr;
        unsigned (*m_get_default_gw_adv_duration)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_allowed_adv_losses)(CC_MqttsnClientHandle, unsigned) = nullptr;
        unsigned (*m_get_allowed_adv_losses)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnSearchHandle (*m_search_prepare)(CC_MqttsnClientHandle, CC_MqttsnErrorCode*) = nullptr;
        CC_MqttsnErrorCode (*m_search_set_retry_period)(CC_MqttsnSearchHandle, unsigned) = nullptr;
        unsigned (*m_search_get_retry_period)(CC_MqttsnSearchHandle) = nullptr;
        CC_MqttsnErrorCode (*m_search_set_retry_count)(CC_MqttsnSearchHandle, unsigned) = nullptr;
        unsigned (*m_search_get_retry_count)(CC_MqttsnSearchHandle) = nullptr;
        CC_MqttsnErrorCode (*m_search_set_broadcast_radius)(CC_MqttsnSearchHandle, unsigned) = nullptr;
        unsigned (*m_search_get_broadcast_radius)(CC_MqttsnSearchHandle) = nullptr;
        CC_MqttsnErrorCode (*m_search_send)(CC_MqttsnSearchHandle, CC_MqttsnSearchCompleteCb, void*) = nullptr;
        CC_MqttsnErrorCode (*m_search_cancel)(CC_MqttsnSearchHandle) = nullptr;
        CC_MqttsnErrorCode (*m_search)(CC_MqttsnClientHandle, CC_MqttsnSearchCompleteCb, void*) = nullptr;

        
        void (*m_set_next_tick_program_callback)(CC_MqttsnClientHandle, CC_MqttsnNextTickProgramCb, void*) = nullptr;
        void (*m_set_cancel_next_tick_wait_callback)(CC_MqttsnClientHandle, CC_MqttsnCancelNextTickWaitCb, void*) = nullptr;
        void (*m_set_send_output_data_callback)(CC_MqttsnClientHandle, CC_MqttsnSendOutputDataCb, void*) = nullptr;  
        void (*m_set_gw_status_report_callback)(CC_MqttsnClientHandle, CC_MqttsnGwStatusReportCb, void*) = nullptr;  
        void (*m_set_gw_disconnect_report_callback)(CC_MqttsnClientHandle, CC_MqttsnGwDisconnectedReportCb, void*) = nullptr; 
        void (*m_set_message_report_callback)(CC_MqttsnClientHandle, CC_MqttsnMessageReportCb, void*) = nullptr;  
        void (*m_set_error_log_callback)(CC_MqttsnClientHandle, CC_MqttsnErrorLogCb, void*) = nullptr;     
        void (*m_set_gwinfo_delay_request_callback)(CC_MqttsnClientHandle, CC_MqttsnGwinfoDelayRequestCb, void*) = nullptr;

    };

    struct UnitTestDeleter
    {
        UnitTestDeleter() = default;
        explicit UnitTestDeleter(const LibFuncs& ops) : 
            m_free(ops.m_free)
        {
        }

        void operator()(CC_MqttsnClient* ptr)
        {
            m_free(ptr);
        }

    private:
        void (*m_free)(CC_MqttsnClientHandle) = nullptr;
    }; 

    struct UnitTestTickInfo
    {
        unsigned m_req = 0U;
        unsigned m_elapsed = 0U;

        UnitTestTickInfo() = default;
        explicit UnitTestTickInfo(unsigned req) : m_req(req) {}
    };

    using UnitTestTickInfosList = std::list<UnitTestTickInfo>;

    struct UnitTestGwInfo
    {
        unsigned m_gwId = 0U;
        UnitTestData m_addr;
        
        UnitTestGwInfo() = default;
        UnitTestGwInfo(const UnitTestGwInfo&) = default;
        UnitTestGwInfo& operator=(const UnitTestGwInfo&) = default;
        UnitTestGwInfo& operator=(const CC_MqttsnGatewayInfo& info);
    };

    struct UnitTestGwInfoReport
    {
        CC_MqttsnGwStatus m_status = CC_MqttsnGwStatus_ValuesLimit;
        UnitTestGwInfo m_info;

        UnitTestGwInfoReport(CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info);
    };

    using UnitTestGwInfoReportsList = std::list<UnitTestGwInfoReport>;

    using UnitTestClientPtr = std::unique_ptr<CC_MqttsnClient, UnitTestDeleter>;

    void unitTestSetUp();
    void unitTestTearDown();

    UnitTestClientPtr unitTestAllocClient(bool enableLog = false);
    void unitTestClientInputData(CC_MqttsnClient* client, const UnitTestData& data);
    void unitTestClientInputMessage(CC_MqttsnClient* client, const UnitTestMessage& msg);

    bool unitTestHasTickReq() const;
    const UnitTestTickInfo* unitTestTickInfo(bool mustExist = true) const;
    void unitTestTick(CC_MqttsnClient* client, unsigned ms = 0U);

    bool unitTestHasGwInfoReport() const;
    const UnitTestGwInfoReport* unitTestGetGwInfoReport(bool mustExist = true) const;
    void unitTestPopGwInfoReport();

    void apiProcessData(CC_MqttsnClient* client, const unsigned char* buf, unsigned bufLen);

protected:
    explicit UnitTestCommonBase(const LibFuncs& funcs);

private:

    static void unitTestTickProgramCb(void* data, unsigned duration);
    static unsigned unitTestCancelTickWaitCb(void* data);
    static void unitTestSendOutputDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);
    static void unitTestGwStatusReportCb(void* data, CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info);
    static void unitTestGwDisconnectReportCb(void* data, CC_MqttsnGatewayDisconnectReason reason);
    static void unitTestMessageReportCb(void* data, const CC_MqttsnMessageInfo* msgInfo);
    static unsigned unitTestGwinfoDelayRequestCb(void* data);
    static void unitTestErrorLogCb(void* data, const char* msg);

    LibFuncs m_funcs;  
    UnitTestTickInfosList m_ticks;
    UnitTestGwInfoReportsList m_gwInfoReports;
};