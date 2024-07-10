#include "UnitTestCommonBase.h"

#include "comms/iterator.h"

#include <cassert>
#include <iostream>

namespace 
{

#define test_assert(cond_) \
    assert(cond_); \
    if (!(cond_)) { \
        std::cerr << "\nAssertion failure (" << #cond_ << ") in " << __FILE__ << ":" << __LINE__ << std::endl; \
        std::exit(1); \
    }

UnitTestCommonBase* asThis(void* data)
{
    return reinterpret_cast<UnitTestCommonBase*>(data);
}    

} // namespace 


UnitTestCommonBase::UnitTestCommonBase(const LibFuncs& funcs) :
    m_funcs(funcs)
{
    test_assert(m_funcs.m_alloc != nullptr);
    test_assert(m_funcs.m_free != nullptr);
    test_assert(m_funcs.m_tick != nullptr);
    test_assert(m_funcs.m_process_data != nullptr);
    test_assert(m_funcs.m_set_default_retry_period != nullptr);
    test_assert(m_funcs.m_get_default_retry_period != nullptr);
    test_assert(m_funcs.m_set_default_retry_count != nullptr);
    test_assert(m_funcs.m_get_default_retry_count != nullptr);    
    test_assert(m_funcs.m_set_default_broadcast_radius != nullptr); 
    test_assert(m_funcs.m_init_gateway_info != nullptr);
    test_assert(m_funcs.m_get_available_gateway_info != nullptr);
    test_assert(m_funcs.m_set_available_gateway_info != nullptr);
    test_assert(m_funcs.m_discard_available_gateway_info != nullptr);
    test_assert(m_funcs.m_discard_all_gateway_infos != nullptr);
    test_assert(m_funcs.m_set_default_gw_adv_duration != nullptr);
    test_assert(m_funcs.m_get_default_gw_adv_duration != nullptr);
    test_assert(m_funcs.m_set_allowed_adv_losses != nullptr);
    test_assert(m_funcs.m_get_allowed_adv_losses != nullptr);
    test_assert(m_funcs.m_search_prepare != nullptr);
    test_assert(m_funcs.m_search_set_retry_period != nullptr);
    test_assert(m_funcs.m_search_get_retry_period != nullptr);
    test_assert(m_funcs.m_search_set_retry_count != nullptr);
    test_assert(m_funcs.m_search_get_retry_count != nullptr);
    test_assert(m_funcs.m_search_set_broadcast_radius != nullptr);
    test_assert(m_funcs.m_search_get_broadcast_radius != nullptr);
    test_assert(m_funcs.m_search_send != nullptr);
    test_assert(m_funcs.m_search_cancel != nullptr);
    test_assert(m_funcs.m_search != nullptr);
    test_assert(m_funcs.m_connect_prepare != nullptr);
    test_assert(m_funcs.m_connect_set_retry_period != nullptr);
    test_assert(m_funcs.m_connect_get_retry_period != nullptr);
    test_assert(m_funcs.m_connect_set_retry_count != nullptr);
    test_assert(m_funcs.m_connect_get_retry_count != nullptr);
    test_assert(m_funcs.m_connect_init_config != nullptr);
    test_assert(m_funcs.m_connect_config != nullptr);
    test_assert(m_funcs.m_connect_init_config_will != nullptr);
    test_assert(m_funcs.m_connect_config_will != nullptr);
    test_assert(m_funcs.m_connect_send != nullptr);
    test_assert(m_funcs.m_connect_cancel != nullptr);
    test_assert(m_funcs.m_connect != nullptr);
    test_assert(m_funcs.m_is_connected != nullptr);
    test_assert(m_funcs.m_disconnect_prepare != nullptr);
    test_assert(m_funcs.m_disconnect_set_retry_period != nullptr);
    test_assert(m_funcs.m_disconnect_get_retry_period != nullptr);
    test_assert(m_funcs.m_disconnect_set_retry_count != nullptr);
    test_assert(m_funcs.m_disconnect_get_retry_count != nullptr);
    test_assert(m_funcs.m_disconnect_send != nullptr);
    test_assert(m_funcs.m_disconnect_cancel != nullptr);
    test_assert(m_funcs.m_disconnect != nullptr);  
    test_assert(m_funcs.m_subscribe_prepare != nullptr);
    test_assert(m_funcs.m_subscribe_set_retry_period != nullptr);
    test_assert(m_funcs.m_subscribe_get_retry_period != nullptr);
    test_assert(m_funcs.m_subscribe_set_retry_count != nullptr);
    test_assert(m_funcs.m_subscribe_get_retry_count != nullptr);
    test_assert(m_funcs.m_subscribe_init_config != nullptr);
    test_assert(m_funcs.m_subscribe_config != nullptr);
    test_assert(m_funcs.m_subscribe_send != nullptr);
    test_assert(m_funcs.m_subscribe_cancel != nullptr);
    test_assert(m_funcs.m_subscribe != nullptr);      

    test_assert(m_funcs.m_set_next_tick_program_callback != nullptr); 
    test_assert(m_funcs.m_set_cancel_next_tick_wait_callback != nullptr); 
    test_assert(m_funcs.m_set_send_output_data_callback != nullptr); 
    test_assert(m_funcs.m_set_gw_status_report_callback != nullptr); 
    test_assert(m_funcs.m_set_gw_disconnect_report_callback != nullptr); 
    test_assert(m_funcs.m_set_message_report_callback != nullptr); 
    test_assert(m_funcs.m_set_error_log_callback != nullptr); 
    test_assert(m_funcs.m_set_gwinfo_delay_request_callback != nullptr); 
}

UnitTestCommonBase::UnitTestOutputDataInfo::UnitTestOutputDataInfo(const std::uint8_t* buf, unsigned bufLen, unsigned broadcastRadius) :
    m_data(buf, buf + bufLen),
    m_broadcastRadius(broadcastRadius)
{
}

UnitTestCommonBase::UnitTestGwInfo& UnitTestCommonBase::UnitTestGwInfo::operator=(const CC_MqttsnGatewayInfo& info)
{
    m_gwId = info.m_gwId;
    if (info.m_addrLen > 0U) {
        test_assert(info.m_addr != nullptr);
        m_addr.assign(info.m_addr, info.m_addr + info.m_addrLen);
    }

    return *this;
}

UnitTestCommonBase::UnitTestGwInfoReport::UnitTestGwInfoReport(CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info) :
    m_status(status)
{
    if (info != nullptr) {
        m_info = *info;
    }
}

UnitTestCommonBase::UnitTestSearchCompleteReport::UnitTestSearchCompleteReport(CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info) :
    m_status(status)
{
    if (info != nullptr) {
        m_info = *info;
    }
}

void UnitTestCommonBase::UnitTestSearchCompleteReport::assignInfo(CC_MqttsnGatewayInfo& info) const
{
    info.m_gwId = m_info.m_gwId;
    info.m_addr = m_info.m_addr.data();
    info.m_addrLen = static_cast<decltype(info.m_addrLen)>(m_info.m_addr.size());
}

UnitTestCommonBase::UnitTestConnectInfo& UnitTestCommonBase::UnitTestConnectInfo::operator=(const CC_MqttsnConnectInfo& info)
{
    m_returnCode = info.m_returnCode;
    return *this;
}

UnitTestCommonBase::UnitTestConnectCompleteReport::UnitTestConnectCompleteReport(CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info) : 
    m_status(status)
{
    if (info != nullptr) {
        m_info = *info;
    }
}

UnitTestCommonBase::UnitTestSubscribeInfo& UnitTestCommonBase::UnitTestSubscribeInfo::operator=(const CC_MqttsnSubscribeInfo& info)
{
    m_returnCode = info.m_returnCode;
    m_topicId = info.m_topicId;
    m_qos = info.m_qos;
    return *this;
}

UnitTestCommonBase::UnitTestSubscribeCompleteReport::UnitTestSubscribeCompleteReport(CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info) : 
    m_status(status)
{
    if (info != nullptr) {
        m_info = *info;
    }
}


void UnitTestCommonBase::unitTestSetUp()
{
}

void UnitTestCommonBase::unitTestTearDown()
{
    m_data = ClientData();
}

UnitTestCommonBase::UnitTestClientPtr UnitTestCommonBase::unitTestAllocClient(bool enableLog)
{
    UnitTestClientPtr client(m_funcs.m_alloc(), UnitTestDeleter(m_funcs));
    m_funcs.m_set_next_tick_program_callback(client.get(), &UnitTestCommonBase::unitTestTickProgramCb, this);
    m_funcs.m_set_cancel_next_tick_wait_callback(client.get(), &UnitTestCommonBase::unitTestCancelTickWaitCb, this);
    m_funcs.m_set_send_output_data_callback(client.get(), &UnitTestCommonBase::unitTestSendOutputDataCb, this);
    m_funcs.m_set_gw_status_report_callback(client.get(), &UnitTestCommonBase::unitTestGwStatusReportCb, this);
    m_funcs.m_set_gw_disconnect_report_callback(client.get(), &UnitTestCommonBase::unitTestGwDisconnectReportCb, this);
    m_funcs.m_set_message_report_callback(client.get(), &UnitTestCommonBase::unitTestMessageReportCb, this);
    m_funcs.m_set_gwinfo_delay_request_callback(client.get(), &UnitTestCommonBase::unitTestGwinfoDelayRequestCb, this);

    if (enableLog) {
        m_funcs.m_set_error_log_callback(client.get(), &UnitTestCommonBase::unitTestErrorLogCb, this);
    }

    return client;
}

void UnitTestCommonBase::unitTestClientInputData(CC_MqttsnClient* client, const UnitTestData& data)
{
    apiProcessData(client, data.data(), static_cast<unsigned>(data.size()));
}

void UnitTestCommonBase::unitTestClientInputMessage(CC_MqttsnClient* client, const UnitTestMessage& msg)
{
    UnitTestData data;
    UnitTestsFrame frame;
    data.resize(frame.length(msg));
    auto writeIter = comms::writeIteratorFor<UnitTestMessage>(data.data());
    auto ec = frame.write(msg, writeIter, data.size());
    test_assert(ec == comms::ErrorStatus::Success);
    unitTestClientInputData(client, data);
}

bool UnitTestCommonBase::unitTestHasTickReq() const
{
    return !m_data.m_ticks.empty();
}

const UnitTestCommonBase::UnitTestTickInfo* UnitTestCommonBase::unitTestTickInfo(bool mustExist) const
{
    if (!unitTestHasTickReq()) {
        test_assert(!mustExist);
        return nullptr;
    }

    return &m_data.m_ticks.front();
}

void UnitTestCommonBase::unitTestTick(CC_MqttsnClient* client, unsigned ms)
{
    test_assert(!m_data.m_ticks.empty());
    auto& info = m_data.m_ticks.front();
    if (ms == 0U) {
        ms = info.m_req;
    }

    if (ms < info.m_req) {
        info.m_elapsed = ms;
        return;
    }

    auto msToReport = info.m_req;
    m_data.m_ticks.pop_front();
    m_funcs.m_tick(client, msToReport);
}

bool UnitTestCommonBase::unitTestHasOutputData() const
{
    return !m_data.m_outData.empty();
}

const UnitTestCommonBase::UnitTestOutputDataInfo* UnitTestCommonBase::unitTestOutputDataInfo(bool mustExist) const
{
    if (!unitTestHasOutputData()) {
        test_assert(!mustExist);
        return nullptr;
    }

    return &m_data.m_outData.front();
}

void UnitTestCommonBase::unitTestPopOutputData()
{
    test_assert(unitTestHasOutputData());
    m_data.m_outData.pop_front();
}

std::vector<UniTestsMsgPtr> UnitTestCommonBase::unitTestPopAllOuputMessages(bool mustExist)
{
    std::vector<UniTestsMsgPtr> result;
    do {
        if (!unitTestHasOutputData()) {
            break;
        }

        auto readPtr = comms::readIteratorFor<UnitTestMessage>(m_data.m_outData.front().m_data.data());
        auto endPtr = readPtr + m_data.m_outData.front().m_data.size();
        UnitTestsFrame frame;
        while (readPtr < endPtr) {
            UniTestsMsgPtr msg;
            auto remLen = static_cast<std::size_t>(std::distance(readPtr, endPtr));
            auto es = frame.read(msg, readPtr, remLen);
            if (es != comms::ErrorStatus::Success) {
                break;
            }

            result.push_back(std::move(msg));
            // readPtr is advanced in read operation above
        }

        m_data.m_outData.pop_front();
    } while (false);

    test_assert((!mustExist) || (!result.empty()))
    return result;
}

UniTestsMsgPtr UnitTestCommonBase::unitTestPopOutputMessage(bool mustExist)
{
    auto allMessages = unitTestPopAllOuputMessages(mustExist);
    if (allMessages.empty()) {
        return UniTestsMsgPtr();
    }

    test_assert(allMessages.size() == 1U);
    return std::move(allMessages.front());
}

bool UnitTestCommonBase::unitTestHasGwInfoReport() const
{
    return !m_data.m_gwInfoReports.empty();
}

UnitTestCommonBase::UnitTestGwInfoReportPtr UnitTestCommonBase::unitTestGetGwInfoReport(bool mustExist)
{
    if (!unitTestHasGwInfoReport()) {
        test_assert(!mustExist);
        return UnitTestGwInfoReportPtr();
    }

    auto ptr = std::move(m_data.m_gwInfoReports.front());
    m_data.m_gwInfoReports.pop_front();
    return ptr;       
}

bool UnitTestCommonBase::unitTestHasGwDisconnectReport() const
{
    return !m_data.m_gwDisconnectReports.empty();
}

UnitTestCommonBase::UnitTestGwDisconnectReportPtr UnitTestCommonBase::unitTestGetGwDisconnectReport(bool mustExist)
{
    if (!unitTestHasGwDisconnectReport()) {
        test_assert(!mustExist);
        return UnitTestGwDisconnectReportPtr();
    }

    auto ptr = std::move(m_data.m_gwDisconnectReports.front());
    m_data.m_gwDisconnectReports.pop_front();
    return ptr;    
}

bool UnitTestCommonBase::unitTestHasSearchCompleteReport() const
{
    return !m_data.m_searchCompleteReports.empty();
}

UnitTestCommonBase::UnitTestSearchCompleteReportPtr UnitTestCommonBase::unitTestSearchCompleteReport(bool mustExist)
{
    if (!unitTestHasSearchCompleteReport()) {
        test_assert(!mustExist);
        return UnitTestSearchCompleteReportPtr();
    }

    auto ptr = std::move(m_data.m_searchCompleteReports.front());
    m_data.m_searchCompleteReports.pop_front();
    return ptr;      
}

CC_MqttsnErrorCode UnitTestCommonBase::unitTestSearchSend(CC_MqttsnSearchHandle search, UnitTestSearchCompleteCb&& cb)
{
    if (cb) {
        m_data.m_searchCompleteCallbacks.push_back(std::move(cb));
    }

    return m_funcs.m_search_send(search, &UnitTestCommonBase::unitTestSearchCompleteCb, this);
}

void UnitTestCommonBase::unitTestSearch(CC_MqttsnClient* client, UnitTestSearchCompleteCb&& cb)
{
    if (cb) {
        m_data.m_searchCompleteCallbacks.push_back(std::move(cb));
    }

    m_funcs.m_search(client, &UnitTestCommonBase::unitTestSearchCompleteCb, this);    
}

void UnitTestCommonBase::unitTestSearchUpdateAddr(CC_MqttsnClient* client, const UnitTestData& addr)
{
    unitTestSearch(
        client,
        [this, client, addr](const UnitTestSearchCompleteReport& report)
        {
            if (report.m_status != CC_MqttsnAsyncOpStatus_Complete) {
                return false;
            }

            auto prevCount = m_funcs.m_get_available_gateways_count(client);

            CC_MqttsnGatewayInfo updInfo;
            m_funcs.m_init_gateway_info(&updInfo);
            updInfo.m_gwId = report.m_info.m_gwId;
            updInfo.m_addr = addr.data();
            updInfo.m_addrLen = static_cast<decltype(updInfo.m_addrLen)>(addr.size());
            auto ec = m_funcs.m_set_available_gateway_info(client, &updInfo);
            test_assert(ec == CC_MqttsnErrorCode_Success);

            auto afterUpdateCount = m_funcs.m_get_available_gateways_count(client);
            test_assert(prevCount == afterUpdateCount); // Mustn't change
            return false;
        });
}

bool UnitTestCommonBase::unitTestHasConnectCompleteReport() const
{
    return !m_data.m_connectCompleteReports.empty();
}

UnitTestCommonBase::UnitTestConnectCompleteReportPtr UnitTestCommonBase::unitTestConnectCompleteReport(bool mustExist)
{
    if (!unitTestHasConnectCompleteReport()) {
        test_assert(!mustExist);
        return UnitTestConnectCompleteReportPtr();
    }

    auto ptr = std::move(m_data.m_connectCompleteReports.front());
    m_data.m_connectCompleteReports.pop_front();
    return ptr;     
}

CC_MqttsnErrorCode UnitTestCommonBase::unitTestConnectSend(CC_MqttsnConnectHandle connect)
{
    return m_funcs.m_connect_send(connect, &UnitTestCommonBase::unitTestConnectCompleteCb, this);
}

void UnitTestCommonBase::unitTestDoConnect(CC_MqttsnClient* client, const CC_MqttsnConnectConfig* config, const CC_MqttsnWillConfig* willConfig)
{
    auto ec = m_funcs.m_connect(client, config, willConfig, &UnitTestCommonBase::unitTestConnectCompleteCb, this);
    test_assert(ec == CC_MqttsnErrorCode_Success);

    {
        test_assert(unitTestHasOutputData());
        auto sentMsg = unitTestPopOutputMessage();
        auto* connectMsg = dynamic_cast<UnitTestConnectMsg*>(sentMsg.get());
        test_assert(connectMsg != nullptr);
        if (config != nullptr) {
            test_assert(connectMsg->field_clientId().value() == config->m_clientId);
            test_assert(connectMsg->field_duration().value() == config->m_duration);
            test_assert(connectMsg->field_flags().field_mid().getBitValue_CleanSession() == config->m_cleanSession);
        }
        test_assert(!unitTestHasOutputData());
    }    

    test_assert(!unitTestHasConnectCompleteReport());
    test_assert(unitTestHasTickReq());
    unitTestTick(client, 100);

    if (willConfig != nullptr) {
        UnitTestWilltopicreqMsg willtopicreqMsg;
        unitTestClientInputMessage(client, willtopicreqMsg);

        {
            test_assert(unitTestHasOutputData());
            auto sentMsg = unitTestPopOutputMessage();
            auto* willtopicMsg = dynamic_cast<UnitTestWilltopicMsg*>(sentMsg.get());
            test_assert(willtopicMsg != nullptr);
            test_assert(willtopicMsg->field_flags().doesExist());
            test_assert(static_cast<CC_MqttsnQoS>(willtopicMsg->field_flags().field().field_qos().getValue()) == willConfig->m_qos);
            test_assert(willtopicMsg->field_flags().field().field_mid().getBitValue_Retain() == willConfig->m_retain);
            test_assert((willConfig->m_topic == nullptr) || (willtopicMsg->field_willTopic().value() == willConfig->m_topic));
            test_assert(!unitTestHasOutputData());
        }    

        test_assert(!unitTestHasConnectCompleteReport());
        test_assert(unitTestHasTickReq());
        unitTestTick(client, 100);         

        UnitTestWillmsgreqMsg willmsgreqMsg;
        unitTestClientInputMessage(client, willmsgreqMsg);

        {
            test_assert(unitTestHasOutputData());
            auto sentMsg = unitTestPopOutputMessage();
            auto* willmsgMsg = dynamic_cast<UnitTestWillmsgMsg*>(sentMsg.get());
            test_assert(willmsgMsg != nullptr);
            test_assert((willConfig->m_data == nullptr) || (willmsgMsg->field_willMsg().value() == UnitTestData(willConfig->m_data, willConfig->m_data + willConfig->m_dataLen)));
        }          

        test_assert(!unitTestHasConnectCompleteReport());
        test_assert(!apiIsConnected(client));
        test_assert(unitTestHasTickReq());
        unitTestTick(client, 100);               
    }

    UnitTestConnackMsg connackMsg;
    connackMsg.field_returnCode().setValue(CC_MqttsnReturnCode_Accepted);
    unitTestClientInputMessage(client, connackMsg);    

    test_assert(unitTestHasConnectCompleteReport());
    auto connectReport = unitTestConnectCompleteReport();
    test_assert(connectReport->m_status== CC_MqttsnAsyncOpStatus_Complete)
    test_assert(connectReport->m_info.m_returnCode == CC_MqttsnReturnCode_Accepted)
    test_assert(apiIsConnected(client));
}

void UnitTestCommonBase::unitTestDoConnectBasic(CC_MqttsnClient* client, const std::string& clientId, bool cleanSession)
{
    CC_MqttsnConnectConfig config;
    apiConnectInitConfig(&config);
    config.m_clientId = clientId.c_str();
    config.m_cleanSession = cleanSession;   
    unitTestDoConnect(client, &config, nullptr);
}

bool UnitTestCommonBase::unitTestHasDisconnectCompleteReport() const
{
    return !m_data.m_disconnectCompleteReports.empty();
}

UnitTestCommonBase::UnitTestDisconnectCompleteReportPtr UnitTestCommonBase::unitTestDisconnectCompleteReport(bool mustExist)
{
    if (!unitTestHasDisconnectCompleteReport()) {
        test_assert(!mustExist);
        return nullptr;
    }

    auto ptr = std::move(m_data.m_disconnectCompleteReports.front());
    m_data.m_disconnectCompleteReports.pop_front();
    return ptr;    

}

CC_MqttsnErrorCode UnitTestCommonBase::unitTestDisconnectSend(CC_MqttsnDisconnectHandle disconnect)
{
    return m_funcs.m_disconnect_send(disconnect, &UnitTestCommonBase::unitTestDisconnectCompleteCb, this);
}

bool UnitTestCommonBase::unitTestHasSubscribeCompleteReport() const
{
    return !m_data.m_subscribeCompleteReports.empty();
}

UnitTestCommonBase::UnitTestSubscribeCompleteReportPtr UnitTestCommonBase::unitTestSubscribeCompleteReport(bool mustExist)
{
    if (!unitTestHasSubscribeCompleteReport()) {
        test_assert(!mustExist);
        return UnitTestSubscribeCompleteReportPtr();
    }

    auto ptr = std::move(m_data.m_subscribeCompleteReports.front());
    m_data.m_subscribeCompleteReports.pop_front();
    return ptr;
}

CC_MqttsnErrorCode UnitTestCommonBase::unitTestSubscribeSend(CC_MqttsnSubscribeHandle subscribe)
{
    return m_funcs.m_subscribe_send(subscribe, &UnitTestCommonBase::unitTestSubscribeCompleteCb, this);
}

void UnitTestCommonBase::apiProcessData(CC_MqttsnClient* client, const unsigned char* buf, unsigned bufLen)
{
    m_funcs.m_process_data(client, buf, bufLen);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiSetDefaultRetryPeriod(CC_MqttsnClient* client, unsigned value)
{
    return m_funcs.m_set_default_retry_period(client, value);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiSetDefaultRetryCount(CC_MqttsnClient* client, unsigned value)
{
    return m_funcs.m_set_default_retry_count(client, value);
}

CC_MqttsnSearchHandle UnitTestCommonBase::apiSearchPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec)
{
    return m_funcs.m_search_prepare(client, ec);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiSearchSetRetryPeriod(CC_MqttsnSearchHandle search, unsigned value)
{
    return m_funcs.m_search_set_retry_period(search, value);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiSearchSetRetryCount(CC_MqttsnSearchHandle search, unsigned value)
{
    return m_funcs.m_search_set_retry_count(search, value);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiSearchSetBroadcastRadius(CC_MqttsnSearchHandle search, unsigned value)
{
    return m_funcs.m_search_set_broadcast_radius(search, value);
}

CC_MqttsnConnectHandle UnitTestCommonBase::apiConnectPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec)
{
    return m_funcs.m_connect_prepare(client, ec);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiConnectSetRetryCount(CC_MqttsnConnectHandle connect, unsigned count)
{
    return m_funcs.m_connect_set_retry_count(connect, count);
}

void UnitTestCommonBase::apiConnectInitConfig(CC_MqttsnConnectConfig* config)
{
    m_funcs.m_connect_init_config(config);
}

void UnitTestCommonBase::apiConnectInitConfigWill(CC_MqttsnWillConfig* config)
{
    m_funcs.m_connect_init_config_will(config);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiConnectConfig(CC_MqttsnConnectHandle connect, const CC_MqttsnConnectConfig* config)
{
    return m_funcs.m_connect_config(connect, config);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiConnectConfigWill(CC_MqttsnConnectHandle connect, const CC_MqttsnWillConfig* config)
{
    return m_funcs.m_connect_config_will(connect, config);
}

bool UnitTestCommonBase::apiIsConnected(CC_MqttsnClient* client)
{
    return m_funcs.m_is_connected(client);
}

CC_MqttsnDisconnectHandle UnitTestCommonBase::apiDisconnectPrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec)
{
    return m_funcs.m_disconnect_prepare(client, ec);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiDisconnectSetRetryCount(CC_MqttsnDisconnectHandle disconnect, unsigned count)
{
    return m_funcs.m_disconnect_set_retry_count(disconnect, count);
}

void UnitTestCommonBase::unitTestTickProgramCb(void* data, unsigned duration)
{
    auto* thisPtr = asThis(data);
    if (thisPtr->m_data.m_ticks.empty()) {
        asThis(data)->m_data.m_ticks.emplace_back(duration);
        return;
    }

    auto& info = thisPtr->m_data.m_ticks.front(); 
    test_assert(info.m_req == 0U);
    info.m_req = duration;
}

unsigned UnitTestCommonBase::unitTestCancelTickWaitCb(void* data)
{
    auto* thisPtr = asThis(data);
    test_assert(!thisPtr->m_data.m_ticks.empty());
    auto result = thisPtr->m_data.m_ticks.front().m_elapsed;
    thisPtr->m_data.m_ticks.pop_front();
    return result;
}

void UnitTestCommonBase::unitTestSendOutputDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius)
{
    auto* thisPtr = asThis(data);
    thisPtr->m_data.m_outData.emplace_back(buf, bufLen, broadcastRadius);
}

void UnitTestCommonBase::unitTestGwStatusReportCb(void* data, CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info)
{
    asThis(data)->m_data.m_gwInfoReports.push_back(std::make_unique<UnitTestGwInfoReport>(status, info));
}

void UnitTestCommonBase::unitTestGwDisconnectReportCb(void* data, CC_MqttsnGatewayDisconnectReason reason)
{
    asThis(data)->m_data.m_gwDisconnectReports.push_back(std::make_unique<UnitTestGwDisconnectReport>(reason));
}

CC_MqttsnSubscribeHandle UnitTestCommonBase::apiSubscribePrepare(CC_MqttsnClient* client, CC_MqttsnErrorCode* ec)
{
    return m_funcs.m_subscribe_prepare(client, ec);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiSubscribeSetRetryCount(CC_MqttsnSubscribeHandle subscribe, unsigned count)
{
    return m_funcs.m_subscribe_set_retry_count(subscribe, count);
}

void UnitTestCommonBase::apiSubscribeInitConfig(CC_MqttsnSubscribeConfig* config)
{
    m_funcs.m_subscribe_init_config(config);
}

CC_MqttsnErrorCode UnitTestCommonBase::apiSubscribeConfig(CC_MqttsnSubscribeHandle subscribe, const CC_MqttsnSubscribeConfig* config)
{
    return m_funcs.m_subscribe_config(subscribe, config);
}

void UnitTestCommonBase::unitTestMessageReportCb(void* data, const CC_MqttsnMessageInfo* msgInfo)
{
    // TODO:
    static_cast<void>(data);
    static_cast<void>(msgInfo);
    test_assert(false);    
}

unsigned UnitTestCommonBase::unitTestGwinfoDelayRequestCb(void* data)
{
    // TODO:
    static_cast<void>(data);
    test_assert(false);  
}

void UnitTestCommonBase::unitTestErrorLogCb([[maybe_unused]] void* data, const char* msg)
{
    std::cout << "ERROR: " << msg << std::endl;
}

void UnitTestCommonBase::unitTestSearchCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnGatewayInfo* info)
{
    test_assert((status != CC_MqttsnAsyncOpStatus_Complete) || (info != nullptr));

    auto* thisPtr = asThis(data);
    thisPtr->m_data.m_searchCompleteReports.push_back(std::make_unique<UnitTestSearchCompleteReport>(status, info));

    if (thisPtr->m_data.m_searchCompleteCallbacks.empty()) {
        return;
    }

    auto& func = thisPtr->m_data.m_searchCompleteCallbacks.front();
    test_assert(func);

    bool popReport = func(*thisPtr->m_data.m_searchCompleteReports.back());
    thisPtr->m_data.m_searchCompleteCallbacks.pop_front();

    if (popReport) {
        thisPtr->m_data.m_searchCompleteReports.pop_back();
    }
}

void UnitTestCommonBase::unitTestConnectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info)
{
    test_assert((status != CC_MqttsnAsyncOpStatus_Complete) || (info != nullptr));
    auto* thisPtr = asThis(data);
    thisPtr->m_data.m_connectCompleteReports.push_back(std::make_unique<UnitTestConnectCompleteReport>(status, info));
}

void UnitTestCommonBase::unitTestDisconnectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status)
{
    auto* thisPtr = asThis(data);
    thisPtr->m_data.m_disconnectCompleteReports.push_back(std::make_unique<UnitTestDisconnectCompleteReport>(status));
}

void UnitTestCommonBase::unitTestSubscribeCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info)
{
    test_assert((status != CC_MqttsnAsyncOpStatus_Complete) || (info != nullptr));
    auto* thisPtr = asThis(data);
    thisPtr->m_data.m_subscribeCompleteReports.push_back(std::make_unique<UnitTestSubscribeCompleteReport>(status, info));
}