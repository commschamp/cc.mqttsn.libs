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

const UnitTestCommonBase::UnitTestGwInfoReport* UnitTestCommonBase::unitTestGetGwInfoReport(bool mustExist) const
{
    if (!unitTestHasGwInfoReport()) {
        test_assert(!mustExist);
        return nullptr;
    }

    return &m_data.m_gwInfoReports.front();
}

void UnitTestCommonBase::unitTestPopGwInfoReport()
{
    test_assert(!m_data.m_gwInfoReports.empty());
    m_data.m_gwInfoReports.pop_front();
}

bool UnitTestCommonBase::unitTestHasSearchCompleteReport() const
{
    return !m_data.m_searchCompleteReports.empty();
}

const UnitTestCommonBase::UnitTestSearchCompleteReport* UnitTestCommonBase::unitTestSearchCompleteReport(bool mustExist) const
{
    if (!unitTestHasSearchCompleteReport()) {
        test_assert(!mustExist);
        return nullptr;
    }

    return &m_data.m_searchCompleteReports.front();
}

void UnitTestCommonBase::unitTestPopSearchCompleteReport()
{
    test_assert(unitTestHasSearchCompleteReport());
    m_data.m_searchCompleteReports.pop_front();
}

void UnitTestCommonBase::unitTestSearchSend(CC_MqttsnSearchHandle search, UnitTestSearchCompleteCb&& cb)
{
    if (cb) {
        m_data.m_searchCompleteCallbacks.push_back(std::move(cb));
    }

    m_funcs.m_search_send(search, &UnitTestCommonBase::unitTestSearchCompleteCb, this);
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
    asThis(data)->m_data.m_gwInfoReports.emplace_back(status, info);
}

void UnitTestCommonBase::unitTestGwDisconnectReportCb(void* data, CC_MqttsnGatewayDisconnectReason reason)
{
    // TODO:
    static_cast<void>(data);
    static_cast<void>(reason);
    test_assert(false);
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
    auto* thisPtr = asThis(data);
    thisPtr->m_data.m_searchCompleteReports.emplace_back(status, info);

    if (thisPtr->m_data.m_searchCompleteCallbacks.empty()) {
        return;
    }

    auto& func = thisPtr->m_data.m_searchCompleteCallbacks.front();
    test_assert(func);

    bool popReport = func(thisPtr->m_data.m_searchCompleteReports.back());
    thisPtr->m_data.m_searchCompleteCallbacks.pop_front();

    if (popReport) {
        thisPtr->m_data.m_searchCompleteReports.pop_back();
    }
}