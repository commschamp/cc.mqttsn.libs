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
        CC_MqttsnErrorCode (*m_set_verify_outgoing_topic_enabled)(CC_MqttsnClientHandle, bool) = nullptr;
        bool (*m_get_verify_outgoing_topic_enabled)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_verify_incoming_topic_enabled)(CC_MqttsnClientHandle, bool) = nullptr;
        bool (*m_get_verify_incoming_topic_enabled)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_verify_incoming_msg_subscribed)(CC_MqttsnClientHandle, bool) = nullptr;
        bool (*m_get_verify_incoming_msg_subscribed)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_outgoing_topic_id_storage_limit)(CC_MqttsnClientHandle, unsigned long long) = nullptr;
        unsigned long long (*m_get_outgoing_topic_id_storage_limit)(CC_MqttsnClientHandle) = nullptr;
        CC_MqttsnErrorCode (*m_set_incoming_topic_id_storage_limit)(CC_MqttsnClientHandle, unsigned long long) = nullptr;
        unsigned long long (*m_get_incoming_topic_id_storage_limit)(CC_MqttsnClientHandle) = nullptr;
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
        CC_MqttsnSubscribeHandle (*m_subscribe_prepare)(CC_MqttsnClientHandle, CC_MqttsnErrorCode*) = nullptr;     
        CC_MqttsnErrorCode (*m_subscribe_set_retry_period)(CC_MqttsnSubscribeHandle, unsigned) = nullptr; 
        unsigned (*m_subscribe_get_retry_period)(CC_MqttsnSubscribeHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_subscribe_set_retry_count)(CC_MqttsnSubscribeHandle, unsigned) = nullptr; 
        unsigned (*m_subscribe_get_retry_count)(CC_MqttsnSubscribeHandle) = nullptr; 
        void (*m_subscribe_init_config)(CC_MqttsnSubscribeConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_subscribe_config)(CC_MqttsnSubscribeHandle, const CC_MqttsnSubscribeConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_subscribe_send)(CC_MqttsnSubscribeHandle, CC_MqttsnSubscribeCompleteCb, void*) = nullptr; 
        CC_MqttsnErrorCode (*m_subscribe_cancel)(CC_MqttsnSubscribeHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_subscribe)(CC_MqttsnClientHandle, const CC_MqttsnSubscribeConfig*, CC_MqttsnSubscribeCompleteCb, void* cbData) = nullptr;             
        CC_MqttsnUnsubscribeHandle (*m_unsubscribe_prepare)(CC_MqttsnClientHandle, CC_MqttsnErrorCode*) = nullptr;     
        CC_MqttsnErrorCode (*m_unsubscribe_set_retry_period)(CC_MqttsnUnsubscribeHandle, unsigned) = nullptr; 
        unsigned (*m_unsubscribe_get_retry_period)(CC_MqttsnUnsubscribeHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_unsubscribe_set_retry_count)(CC_MqttsnUnsubscribeHandle, unsigned) = nullptr; 
        unsigned (*m_unsubscribe_get_retry_count)(CC_MqttsnUnsubscribeHandle) = nullptr; 
        void (*m_unsubscribe_init_config)(CC_MqttsnUnsubscribeConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_unsubscribe_config)(CC_MqttsnUnsubscribeHandle, const CC_MqttsnUnsubscribeConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_unsubscribe_send)(CC_MqttsnUnsubscribeHandle, CC_MqttsnUnsubscribeCompleteCb, void*) = nullptr; 
        CC_MqttsnErrorCode (*m_unsubscribe_cancel)(CC_MqttsnUnsubscribeHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_unsubscribe)(CC_MqttsnClientHandle, const CC_MqttsnUnsubscribeConfig*, CC_MqttsnUnsubscribeCompleteCb, void* cbData) = nullptr;             
        CC_MqttsnPublishHandle (*m_publish_prepare)(CC_MqttsnClientHandle, CC_MqttsnErrorCode*) = nullptr;     
        CC_MqttsnErrorCode (*m_publish_set_retry_period)(CC_MqttsnPublishHandle, unsigned) = nullptr; 
        unsigned (*m_publish_get_retry_period)(CC_MqttsnPublishHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_publish_set_retry_count)(CC_MqttsnPublishHandle, unsigned) = nullptr; 
        unsigned (*m_publish_get_retry_count)(CC_MqttsnPublishHandle) = nullptr; 
        void (*m_publish_init_config)(CC_MqttsnPublishConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_publish_config)(CC_MqttsnPublishHandle, const CC_MqttsnPublishConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_publish_send)(CC_MqttsnPublishHandle, CC_MqttsnPublishCompleteCb, void*) = nullptr; 
        CC_MqttsnErrorCode (*m_publish_cancel)(CC_MqttsnPublishHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_publish)(CC_MqttsnClientHandle, const CC_MqttsnPublishConfig*, CC_MqttsnPublishCompleteCb, void* cbData) = nullptr;             
        CC_MqttsnWillHandle (*m_will_prepare)(CC_MqttsnClientHandle, CC_MqttsnErrorCode*) = nullptr;     
        CC_MqttsnErrorCode (*m_will_set_retry_period)(CC_MqttsnWillHandle, unsigned) = nullptr; 
        unsigned (*m_will_get_retry_period)(CC_MqttsnWillHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_will_set_retry_count)(CC_MqttsnWillHandle, unsigned) = nullptr; 
        unsigned (*m_will_get_retry_count)(CC_MqttsnWillHandle) = nullptr; 
        void (*m_will_init_config)(CC_MqttsnWillConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_will_config)(CC_MqttsnWillHandle, const CC_MqttsnWillConfig*) = nullptr; 
        CC_MqttsnErrorCode (*m_will_send)(CC_MqttsnWillHandle, CC_MqttsnWillCompleteCb, void*) = nullptr; 
        CC_MqttsnErrorCode (*m_will_cancel)(CC_MqttsnWillHandle) = nullptr; 
        CC_MqttsnErrorCode (*m_will)(CC_MqttsnClientHandle, const CC_MqttsnWillConfig*, CC_MqttsnWillCompleteCb, void* cbData) = nullptr;             

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

    using UnitTestGwInfoReportPtr = std::unique_ptr<UnitTestGwInfoReport>;
    using UnitTestGwInfoReportsList = std::list<UnitTestGwInfoReportPtr>;

    struct UnitTestGwDisconnectReport
    {
        CC_MqttsnGatewayDisconnectReason m_reason = CC_MqttsnGatewayDisconnectReason_ValuesLimit;

        UnitTestGwDisconnectReport(CC_MqttsnGatewayDisconnectReason reason) : m_reason(reason) {}
    };    

    using UnitTestGwDisconnectReportPtr = std::unique_ptr<UnitTestGwDisconnectReport>;
    using UnitTestGwDisconnectReportsList = std::list<UnitTestGwDisconnectReportPtr>;

    struct UnitTestSearchCompleteReport
    {
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;
        UnitTestGwInfo m_info;

        UnitTestSearchCompleteReport(CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info);
        void assignInfo(CC_MqttsnGatewayInfo& info) const;
    };

    using UnitTestSearchCompleteReportPtr = std::unique_ptr<UnitTestSearchCompleteReport>;
    using UnitTestSearchCompleteReportsList = std::list<UnitTestSearchCompleteReportPtr>;

    using UnitTestSearchCompleteCb = std::function<bool (const UnitTestSearchCompleteReport& info)>;
    using UnitTestSearchCompleteCbList = std::list<UnitTestSearchCompleteCb>;
    using UnitTestSearchgwResponseDelayList = std::list<unsigned>;

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

    using UnitTestConnectCompleteReportPtr = std::unique_ptr<UnitTestConnectCompleteReport>;
    using UnitTestConnectCompleteReportList = std::list<UnitTestConnectCompleteReportPtr>;    

    struct UnitTestDisconnectCompleteReport
    {
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;

        UnitTestDisconnectCompleteReport(CC_MqttsnAsyncOpStatus status) : m_status(status) {};
    };

    using UnitTestDisconnectCompleteReportPtr = std::unique_ptr<UnitTestDisconnectCompleteReport>;
    using UnitTestDisconnectCompleteReportList = std::list<UnitTestDisconnectCompleteReportPtr>;    

    struct UnitTestSubscribeInfo
    {
        CC_MqttsnReturnCode m_returnCode = CC_MqttsnReturnCode_ValuesLimit;
        CC_MqttsnQoS m_qos;        
        UnitTestSubscribeInfo() = default;
        UnitTestSubscribeInfo(const UnitTestSubscribeInfo&) = default;
        UnitTestSubscribeInfo& operator=(const UnitTestSubscribeInfo&) = default;
        UnitTestSubscribeInfo& operator=(const CC_MqttsnSubscribeInfo& info);
    };    

    struct UnitTestSubscribeCompleteReport
    {
        CC_MqttsnSubscribeHandle m_handle = nullptr;
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;
        UnitTestSubscribeInfo m_info;

        UnitTestSubscribeCompleteReport(CC_MqttsnSubscribeHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info);
        UnitTestSubscribeCompleteReport(UnitTestSubscribeCompleteReport&&) = default;
        UnitTestSubscribeCompleteReport& operator=(const UnitTestSubscribeCompleteReport&) = default;
    };    

    using UnitTestSubscribeCompleteReportPtr = std::unique_ptr<UnitTestSubscribeCompleteReport>;
    using UnitTestSubscribeCompleteReportList = std::list<UnitTestSubscribeCompleteReportPtr>;    

    struct UnitTestSubscribeResponseConfig
    {
        CC_MqttsnTopicId m_topicId = 0U;
    };     


    struct UnitTestUnsubscribeCompleteReport
    {
        CC_MqttsnUnsubscribeHandle m_handle = nullptr;
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;

        UnitTestUnsubscribeCompleteReport(CC_MqttsnUnsubscribeHandle handle, CC_MqttsnAsyncOpStatus status);
        UnitTestUnsubscribeCompleteReport(UnitTestUnsubscribeCompleteReport&&) = default;
        UnitTestUnsubscribeCompleteReport& operator=(const UnitTestUnsubscribeCompleteReport&) = default;
    };    

    using UnitTestUnsubscribeCompleteReportPtr = std::unique_ptr<UnitTestUnsubscribeCompleteReport>;
    using UnitTestUnsubscribeCompleteReportList = std::list<UnitTestUnsubscribeCompleteReportPtr>;           

    struct UnitTestPublishInfo
    {
        CC_MqttsnReturnCode m_returnCode = CC_MqttsnReturnCode_ValuesLimit;

        UnitTestPublishInfo() = default;
        UnitTestPublishInfo(const UnitTestPublishInfo&) = default;
        UnitTestPublishInfo& operator=(const UnitTestPublishInfo&) = default;
        UnitTestPublishInfo& operator=(const CC_MqttsnPublishInfo& info);
    };    

    struct UnitTestPublishCompleteReport
    {
        CC_MqttsnPublishHandle m_handle = nullptr;
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;
        UnitTestPublishInfo m_info;

        UnitTestPublishCompleteReport(CC_MqttsnPublishHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info);
        UnitTestPublishCompleteReport(UnitTestPublishCompleteReport&&) = default;
        UnitTestPublishCompleteReport& operator=(const UnitTestPublishCompleteReport&) = default;
    };    

    using UnitTestPublishCompleteReportPtr = std::unique_ptr<UnitTestPublishCompleteReport>;
    using UnitTestPublishCompleteReportList = std::list<UnitTestPublishCompleteReportPtr>;    

    struct UnitTestWillInfo
    {
        CC_MqttsnReturnCode m_topicUpdReturnCode = CC_MqttsnReturnCode_ValuesLimit;
        CC_MqttsnReturnCode m_msgUpdReturnCode = CC_MqttsnReturnCode_ValuesLimit;

        UnitTestWillInfo() = default;
        UnitTestWillInfo(const UnitTestWillInfo&) = default;
        UnitTestWillInfo& operator=(const UnitTestWillInfo&) = default;
        UnitTestWillInfo& operator=(const CC_MqttsnWillInfo& info);
    };    


    struct UnitTestWillCompleteReport
    {
        CC_MqttsnAsyncOpStatus m_status = CC_MqttsnAsyncOpStatus_ValuesLimit;
        UnitTestWillInfo m_info;

        UnitTestWillCompleteReport(CC_MqttsnAsyncOpStatus status, const CC_MqttsnWillInfo* info);
        UnitTestWillCompleteReport(UnitTestWillCompleteReport&&) = default;
        UnitTestWillCompleteReport& operator=(const UnitTestWillCompleteReport&) = default;
    };    

    using UnitTestWillCompleteReportPtr = std::unique_ptr<UnitTestWillCompleteReport>;
    using UnitTestWillCompleteReportList = std::list<UnitTestWillCompleteReportPtr>;            


    struct UnitTestMessageInfo
    {
        std::string m_topic;
        UnitTestData m_data;
        CC_MqttsnQoS m_qos = CC_MqttsnQoS_AtMostOnceDelivery; 
        CC_MqttsnTopicId m_topicId = 0U; 
        bool m_retained = false; ///< Retain flag of the message.        

        UnitTestMessageInfo() = default;
        explicit UnitTestMessageInfo(const CC_MqttsnMessageInfo& info);
    };

    using UnitTestMessageInfoPtr = std::unique_ptr<UnitTestMessageInfo>;
    using UnitTestMessageInfosList = std::list<UnitTestMessageInfoPtr>;            

    using UnitTestClientPtr = std::unique_ptr<CC_MqttsnClient, UnitTestDeleter>;

    void unitTestSetUp();
    void unitTestTearDown();

    UnitTestClientPtr unitTestAllocClient(bool enableLog = false);
    void unitTestClientInputData(CC_MqttsnClient* client, const UnitTestData& data);
    void unitTestClientInputMessage(CC_MqttsnClient* client, const UnitTestMessage& msg);
    void unitTestPushSearchgwResponseDelay(unsigned val);

    static CC_MqttsnTopicId unitTestShortTopicNameToId(const std::string& topic);

    bool unitTestHasTickReq() const;
    const UnitTestTickInfo* unitTestTickInfo(bool mustExist = true) const;
    void unitTestTick(CC_MqttsnClient* client, unsigned ms = 0U);

    bool unitTestHasOutputData() const;
    const UnitTestOutputDataInfo* unitTestOutputDataInfo(bool mustExist = true) const;
    void unitTestPopOutputData();
    std::vector<UniTestsMsgPtr> unitTestPopAllOuputMessages(bool mustExist = true);
    UniTestsMsgPtr unitTestPopOutputMessage(bool mustExist = true);

    bool unitTestHasGwInfoReport() const;
    UnitTestGwInfoReportPtr unitTestGetGwInfoReport(bool mustExist = true);

    bool unitTestHasGwDisconnectReport() const;
    UnitTestGwDisconnectReportPtr unitTestGetGwDisconnectReport(bool mustExist = true);

    bool unitTestHasSearchCompleteReport() const;
    UnitTestSearchCompleteReportPtr unitTestSearchCompleteReport(bool mustExist = true);

    CC_MqttsnErrorCode unitTestSearchSend(CC_MqttsnSearchHandle search, UnitTestSearchCompleteCb&& cb = UnitTestSearchCompleteCb());
    void unitTestSearch(CC_MqttsnClient* client, UnitTestSearchCompleteCb&& cb = UnitTestSearchCompleteCb());
    void unitTestSearchUpdateAddr(CC_MqttsnClient* client, const UnitTestData& addr);

    bool unitTestHasConnectCompleteReport() const;
    UnitTestConnectCompleteReportPtr unitTestConnectCompleteReport(bool mustExist = true);

    CC_MqttsnErrorCode unitTestConnectSend(CC_MqttsnConnectHandle connect);
    void unitTestDoConnect(CC_MqttsnClient* client, const CC_MqttsnConnectConfig* config, const CC_MqttsnWillConfig* willConfig);
    void unitTestDoConnectBasic(CC_MqttsnClient* client, const std::string& clientId = std::string(), bool cleanSession = true);

    bool unitTestHasDisconnectCompleteReport() const;
    UnitTestDisconnectCompleteReportPtr unitTestDisconnectCompleteReport(bool mustExist = true);

    void unitTestDoDisconnect(CC_MqttsnClient* client);
    CC_MqttsnErrorCode unitTestDisconnectSend(CC_MqttsnDisconnectHandle disconnect);

    bool unitTestHasSubscribeCompleteReport() const;
    UnitTestSubscribeCompleteReportPtr unitTestSubscribeCompleteReport(bool mustExist = true);

    CC_MqttsnErrorCode unitTestSubscribeSend(CC_MqttsnSubscribeHandle subscribe);
    void unitTestDoSubscribe(CC_MqttsnClient* client, const CC_MqttsnSubscribeConfig* config, const UnitTestSubscribeResponseConfig* respConfig = nullptr);
    void unitTestDoSubscribeTopic(CC_MqttsnClient* client, const std::string& topic, CC_MqttsnQoS qos = CC_MqttsnQoS_ExactlyOnceDelivery);
    void unitTestDoSubscribeTopicId(CC_MqttsnClient* client, CC_MqttsnTopicId topicId, CC_MqttsnQoS qos = CC_MqttsnQoS_ExactlyOnceDelivery);

    bool unitTestHasUnsubscribeCompleteReport() const;
    UnitTestUnsubscribeCompleteReportPtr unitTestUnsubscribeCompleteReport(bool mustExist = true);

    CC_MqttsnErrorCode unitTestUnsubscribeSend(CC_MqttsnUnsubscribeHandle unsubscribe);    

    bool unitTestHasPublishCompleteReport() const;
    UnitTestPublishCompleteReportPtr unitTestPublishCompleteReport(bool mustExist = true);

    CC_MqttsnErrorCode unitTestPublishSend(CC_MqttsnPublishHandle publish);    

    bool unitTestHasWillCompleteReport() const;
    UnitTestWillCompleteReportPtr unitTestWillCompleteReport(bool mustExist = true);

    CC_MqttsnErrorCode unitTestWillSend(CC_MqttsnWillHandle will);    

    bool unitTestHasReceivedMessage() const;
    UnitTestMessageInfoPtr unitTestReceivedMessage(bool mustExist = true);

    void apiProcessData(CC_MqttsnClient* client, const unsigned char* buf, unsigned bufLen);
    CC_MqttsnErrorCode apiSetDefaultRetryPeriod(CC_MqttsnClient* client, unsigned value);
    CC_MqttsnErrorCode apiSetDefaultRetryCount(CC_MqttsnClient* client, unsigned value);
    CC_MqttsnErrorCode apiSetVerifyIncomingMsgSubscribed(CC_MqttsnClient* client, bool enabled);
    void apiInitGatewayInfo(CC_MqttsnGatewayInfo* info);
    CC_MqttsnErrorCode apiSetAvailableGatewayInfo(CC_MqttsnClient* client, const CC_MqttsnGatewayInfo* info);
    CC_MqttsnErrorCode apiSetOutgoingTopicIdStorageLimit(CC_MqttsnClient* client, unsigned long long limit);
    CC_MqttsnErrorCode apiSetIncomingTopicIdStorageLimit(CC_MqttsnClient* client, unsigned long long limit);

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

    CC_MqttsnSubscribeHandle apiSubscribePrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec = nullptr);
    CC_MqttsnErrorCode apiSubscribeSetRetryCount(CC_MqttsnSubscribeHandle subscribe, unsigned count);
    void apiSubscribeInitConfig(CC_MqttsnSubscribeConfig* config);
    CC_MqttsnErrorCode apiSubscribeConfig(CC_MqttsnSubscribeHandle subscribe, const CC_MqttsnSubscribeConfig* config);
    CC_MqttsnErrorCode apiSubscribeCancel(CC_MqttsnSubscribeHandle subscribe);

    CC_MqttsnUnsubscribeHandle apiUnsubscribePrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec = nullptr);
    CC_MqttsnErrorCode apiUnsubscribeSetRetryCount(CC_MqttsnUnsubscribeHandle unsubscribe, unsigned count);
    void apiUnsubscribeInitConfig(CC_MqttsnUnsubscribeConfig* config);
    CC_MqttsnErrorCode apiUnsubscribeConfig(CC_MqttsnUnsubscribeHandle unsubscribe, const CC_MqttsnUnsubscribeConfig* config);
    CC_MqttsnErrorCode apiUnsubscribeCancel(CC_MqttsnUnsubscribeHandle unsubscribe);    

    CC_MqttsnPublishHandle apiPublishPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec = nullptr);
    CC_MqttsnErrorCode apiPublishSetRetryCount(CC_MqttsnPublishHandle publish, unsigned count);
    void apiPublishInitConfig(CC_MqttsnPublishConfig* config);
    CC_MqttsnErrorCode apiPublishConfig(CC_MqttsnPublishHandle publish, const CC_MqttsnPublishConfig* config);
    CC_MqttsnErrorCode apiPublishCancel(CC_MqttsnPublishHandle publish);  

    CC_MqttsnWillHandle apiWillPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec = nullptr);
    CC_MqttsnErrorCode apiWillSetRetryCount(CC_MqttsnWillHandle will, unsigned count);
    void apiWillInitConfig(CC_MqttsnWillConfig* config);
    CC_MqttsnErrorCode apiWillConfig(CC_MqttsnWillHandle will, const CC_MqttsnWillConfig* config);
    CC_MqttsnErrorCode apiWillCancel(CC_MqttsnWillHandle will);        


protected:
    explicit UnitTestCommonBase(const LibFuncs& funcs);

private:
    struct ClientData
    {
        UnitTestTickInfosList m_ticks;
        UnitTestOutputDataInfosList m_outData;
        UnitTestGwInfoReportsList m_gwInfoReports;
        UnitTestGwDisconnectReportsList m_gwDisconnectReports;
        UnitTestSearchCompleteReportsList m_searchCompleteReports;
        UnitTestSearchCompleteCbList m_searchCompleteCallbacks;
        UnitTestSearchgwResponseDelayList m_searchgwResponseDelays;
        UnitTestConnectCompleteReportList m_connectCompleteReports;
        UnitTestDisconnectCompleteReportList m_disconnectCompleteReports;
        UnitTestSubscribeCompleteReportList m_subscribeCompleteReports;
        UnitTestUnsubscribeCompleteReportList m_unsubscribeCompleteReports;
        UnitTestPublishCompleteReportList m_publishCompleteReports;
        UnitTestWillCompleteReportList m_willCompleteReports;
        UnitTestMessageInfosList m_recvMsgs;
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
    static void unitTestSubscribeCompleteCb(void* data, CC_MqttsnSubscribeHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info);
    static void unitTestUnsubscribeCompleteCb(void* data, CC_MqttsnUnsubscribeHandle handle, CC_MqttsnAsyncOpStatus status);
    static void unitTestPublishCompleteCb(void* data, CC_MqttsnPublishHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info);
    static void unitTestWillCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnWillInfo* info);

    LibFuncs m_funcs;  
    ClientData m_data;
};