#pragma once

#include "cc_mqttsn_client/common.h"

#include "UnitTestProtocolDefs.h"

#include <functional>
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
        void (*m_init_gateway_info)(CC_MqttsnGatewayInfo* info) = nullptr;
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
        CC_MqttsnConnectHandle (*m_connect_prepare)(CC_MqttsnClientHandle, CC_MqttsnErrorCode*) = nullptr;
        CC_MqttsnErrorCode (*m_connect_set_retry_period)(CC_MqttsnConnectHandle, unsigned ms) = nullptr;
        unsigned (*m_connect_get_retry_period)(CC_MqttsnConnectHandle) = nullptr;
        CC_MqttsnErrorCode (*m_connect_set_retry_count)(CC_MqttsnConnectHandle, unsigned count) = nullptr;
        unsigned (*m_connect_get_retry_count)(CC_MqttsnConnectHandle) = nullptr;
        void (*m_connect_init_config)(CC_MqttsnConnectConfig* config) = nullptr;
        CC_MqttsnErrorCode (*m_connect_config)(CC_MqttsnConnectHandle, const CC_MqttsnConnectConfig*) = nullptr;
        void (*m_connect_init_config_will)(CC_MqttsnWillConfig*) = nullptr;
        CC_MqttsnErrorCode (*m_connect_config_will)(CC_MqttsnConnectHandle, const CC_MqttsnWillConfig*) = nullptr;
        CC_MqttsnErrorCode (*m_connect_send)(CC_MqttsnConnectHandle, CC_MqttsnConnectCompleteCb, void*) = nullptr;
        CC_MqttsnErrorCode (*m_connect_cancel)(CC_MqttsnConnectHandle) = nullptr;
        CC_MqttsnErrorCode (*m_connect)(CC_MqttsnClientHandle, const CC_MqttsnConnectConfig*, const CC_MqttsnWillConfig*, CC_MqttsnConnectCompleteCb, void*) = nullptr;
        bool (*m_is_connected)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnDisconnectHandle (*m_disconnect_prepare)(CC_MqttsnClientHandle, CC_MqttsnErrorCode*) = nullptr;
        CC_MqttsnErrorCode (*m_disconnect_set_retry_period)(CC_MqttsnDisconnectHandle, unsigned ms) = nullptr;
        unsigned (*m_disconnect_get_retry_period)(CC_MqttsnDisconnectHandle) = nullptr;
        CC_MqttsnErrorCode (*m_disconnect_set_retry_count)(CC_MqttsnDisconnectHandle, unsigned count) = nullptr;
        unsigned (*m_disconnect_get_retry_count)(CC_MqttsnDisconnectHandle) = nullptr;
        CC_MqttsnErrorCode (*m_disconnect_send)(CC_MqttsnDisconnectHandle, CC_MqttsnDisconnectCompleteCb, void*) = nullptr;
        CC_MqttsnErrorCode (*m_disconnect_cancel)(CC_MqttsnDisconnectHandle) = nullptr;
        CC_MqttsnErrorCode (*m_disconnect)(CC_MqttsnClientHandle, CC_MqttsnDisconnectCompleteCb, void*) = nullptr;

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

    struct UnitTestOutputDataInfo
    {
        UnitTestData m_data;
        unsigned m_broadcastRadius = 0U;

        UnitTestOutputDataInfo(const std::uint8_t* buf, unsigned bufLen, unsigned broadcastRadius);
    };

    using UnitTestOutputDataInfosList = std::list<UnitTestOutputDataInfo>;

    struct UnitTestGwInfo
    {
        std::uint8_t m_gwId = 0U;
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

    struct UnitTestSearchCompleteReport
    {
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;
        UnitTestGwInfo m_info;

        UnitTestSearchCompleteReport(CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info);
        void assignInfo(CC_MqttsnGatewayInfo& info) const;
    };
    using UnitTestSearchCompleteReportsList = std::list<UnitTestSearchCompleteReport>;

    using UnitTestSearchCompleteCb = std::function<bool (const UnitTestSearchCompleteReport& info)>;
    using UnitTestSearchCompleteCbList = std::list<UnitTestSearchCompleteCb>;

    struct UnitTestConnectInfo
    {
        CC_MqttsnReturnCode m_returnCode = CC_MqttsnReturnCode_ValuesLimit;
        
        UnitTestConnectInfo() = default;
        UnitTestConnectInfo(const UnitTestConnectInfo&) = default;
        UnitTestConnectInfo& operator=(const UnitTestConnectInfo&) = default;
        UnitTestConnectInfo& operator=(const CC_MqttsnConnectInfo& info);
    };    

    struct UnitTestConnectCompleteReport
    {
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;
        UnitTestConnectInfo m_info;

        UnitTestConnectCompleteReport(CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info);
    };

    using UnitTestConnectCompleteReportList = std::list<UnitTestConnectCompleteReport>;    

    struct UnitTestDisconnectCompleteReport
    {
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;

        UnitTestDisconnectCompleteReport(CC_MqttsnAsyncOpStatus status) : m_status(status) {};
    };

    using UnitTestDisconnectCompleteReportList = std::list<UnitTestDisconnectCompleteReport>;    

    using UnitTestClientPtr = std::unique_ptr<CC_MqttsnClient, UnitTestDeleter>;

    void unitTestSetUp();
    void unitTestTearDown();

    UnitTestClientPtr unitTestAllocClient(bool enableLog = false);
    void unitTestClientInputData(CC_MqttsnClient* client, const UnitTestData& data);
    void unitTestClientInputMessage(CC_MqttsnClient* client, const UnitTestMessage& msg);

    bool unitTestHasTickReq() const;
    const UnitTestTickInfo* unitTestTickInfo(bool mustExist = true) const;
    void unitTestTick(CC_MqttsnClient* client, unsigned ms = 0U);

    bool unitTestHasOutputData() const;
    const UnitTestOutputDataInfo* unitTestOutputDataInfo(bool mustExist = true) const;
    void unitTestPopOutputData();
    std::vector<UniTestsMsgPtr> unitTestPopAllOuputMessages(bool mustExist = true);
    UniTestsMsgPtr unitTestPopOutputMessage(bool mustExist = true);

    bool unitTestHasGwInfoReport() const;
    const UnitTestGwInfoReport* unitTestGetGwInfoReport(bool mustExist = true) const;
    void unitTestPopGwInfoReport();

    bool unitTestHasSearchCompleteReport() const;
    const UnitTestSearchCompleteReport* unitTestSearchCompleteReport(bool mustExist = true) const;
    void unitTestPopSearchCompleteReport();

    CC_MqttsnErrorCode unitTestSearchSend(CC_MqttsnSearchHandle search, UnitTestSearchCompleteCb&& cb = UnitTestSearchCompleteCb());
    void unitTestSearch(CC_MqttsnClient* client, UnitTestSearchCompleteCb&& cb = UnitTestSearchCompleteCb());
    void unitTestSearchUpdateAddr(CC_MqttsnClient* client, const UnitTestData& addr);

    bool unitTestHasConnectCompleteReport() const;
    const UnitTestConnectCompleteReport* unitTestConnectCompleteReport(bool mustExist = true) const;
    void unitTestPopConnectCompleteReport();

    CC_MqttsnErrorCode unitTestConnectSend(CC_MqttsnConnectHandle connect);
    void unitTestDoConnect(CC_MqttsnClient* client, const CC_MqttsnConnectConfig* config, const CC_MqttsnWillConfig* willConfig);
    void unitTestDoConnectBasic(CC_MqttsnClient* client, const std::string& clientId = std::string(), bool cleanSession = true);

    bool unitTestHasDisconnectCompleteReport() const;
    const UnitTestDisconnectCompleteReport* unitTestDisconnectCompleteReport(bool mustExist = true) const;
    void unitTestPopDisconnectCompleteReport();

    CC_MqttsnErrorCode unitTestDisconnectSend(CC_MqttsnDisconnectHandle disconnect);

    void apiProcessData(CC_MqttsnClient* client, const unsigned char* buf, unsigned bufLen);
    CC_MqttsnErrorCode apiSetDefaultRetryPeriod(CC_MqttsnClient* client, unsigned value);
    CC_MqttsnErrorCode apiSetDefaultRetryCount(CC_MqttsnClient* client, unsigned value);
    CC_MqttsnSearchHandle apiSearchPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec = nullptr);
    CC_MqttsnErrorCode apiSearchSetRetryPeriod(CC_MqttsnSearchHandle search, unsigned value);
    CC_MqttsnErrorCode apiSearchSetRetryCount(CC_MqttsnSearchHandle search, unsigned value);
    CC_MqttsnErrorCode apiSearchSetBroadcastRadius(CC_MqttsnSearchHandle search, unsigned value);

    CC_MqttsnConnectHandle apiConnectPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec = nullptr);
    CC_MqttsnErrorCode apiConnectSetRetryCount(CC_MqttsnConnectHandle connect, unsigned count);
    void apiConnectInitConfig(CC_MqttsnConnectConfig* config);
    void apiConnectInitConfigWill(CC_MqttsnWillConfig* config);
    CC_MqttsnErrorCode apiConnectConfig(CC_MqttsnConnectHandle connect, const CC_MqttsnConnectConfig* config);
    CC_MqttsnErrorCode apiConnectConfigWill(CC_MqttsnConnectHandle connect, const CC_MqttsnWillConfig* config);
    bool apiIsConnected(CC_MqttsnClient* client);

    CC_MqttsnDisconnectHandle apiDisconnectPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec = nullptr);
    CC_MqttsnErrorCode apiDisconnectSetRetryCount(CC_MqttsnDisconnectHandle disconnect, unsigned count);

protected:
    explicit UnitTestCommonBase(const LibFuncs& funcs);

private:
    struct ClientData
    {
        UnitTestTickInfosList m_ticks;
        UnitTestOutputDataInfosList m_outData;
        UnitTestGwInfoReportsList m_gwInfoReports;
        UnitTestSearchCompleteReportsList m_searchCompleteReports;
        UnitTestSearchCompleteCbList m_searchCompleteCallbacks;
        UnitTestConnectCompleteReportList m_connectCompleteReports;
        UnitTestDisconnectCompleteReportList m_disconnectCompleteReports;
    };

    static void unitTestTickProgramCb(void* data, unsigned duration);
    static unsigned unitTestCancelTickWaitCb(void* data);
    static void unitTestSendOutputDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);
    static void unitTestGwStatusReportCb(void* data, CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info);
    static void unitTestGwDisconnectReportCb(void* data, CC_MqttsnGatewayDisconnectReason reason);
    static void unitTestMessageReportCb(void* data, const CC_MqttsnMessageInfo* msgInfo);
    static unsigned unitTestGwinfoDelayRequestCb(void* data);
    static void unitTestErrorLogCb(void* data, const char* msg);
    static void unitTestSearchCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info);
    static void unitTestConnectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info);
    static void unitTestDisconnectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status);

    LibFuncs m_funcs;  
    ClientData m_data;
};