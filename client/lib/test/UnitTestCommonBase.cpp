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

    test_assert(m_funcs.m_set_next_tick_program_callback != nullptr); 
    test_assert(m_funcs.m_set_cancel_next_tick_wait_callback != nullptr); 
    test_assert(m_funcs.m_set_send_output_data_callback != nullptr); 
    test_assert(m_funcs.m_set_gw_status_report_callback != nullptr); 
    test_assert(m_funcs.m_set_gw_disconnect_report_callback != nullptr); 
    test_assert(m_funcs.m_set_message_report_callback != nullptr); 
    test_assert(m_funcs.m_set_error_log_callback != nullptr); 
    test_assert(m_funcs.m_set_gwinfo_delay_request_callback != nullptr); 
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

void UnitTestCommonBase::unitTestSetUp()
{
}

void UnitTestCommonBase::unitTestTearDown()
{
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
    return !m_ticks.empty();
}

const UnitTestCommonBase::UnitTestTickInfo* UnitTestCommonBase::unitTestTickInfo(bool mustExist) const
{
    if (!unitTestHasTickReq()) {
        test_assert(!mustExist);
        return nullptr;
    }

    return &m_ticks.front();
}

void UnitTestCommonBase::unitTestTick(CC_MqttsnClient* client, unsigned ms)
{
    test_assert(!m_ticks.empty());
    auto& info = m_ticks.front();
    if (ms == 0U) {
        ms = info.m_req;
    }

    if (ms < info.m_req) {
        info.m_elapsed = ms;
        return;
    }

    auto msToReport = info.m_req;
    m_ticks.pop_front();
    m_funcs.m_tick(client, msToReport);
}

bool UnitTestCommonBase::unitTestHasGwInfoReport() const
{
    return !m_gwInfoReports.empty();
}

const UnitTestCommonBase::UnitTestGwInfoReport* UnitTestCommonBase::unitTestGetGwInfoReport(bool mustExist) const
{
    if (!unitTestHasGwInfoReport()) {
        test_assert(!mustExist);
        return nullptr;
    }

    return &m_gwInfoReports.front();
}

void UnitTestCommonBase::unitTestPopGwInfoReport()
{
    test_assert(!m_gwInfoReports.empty());
    m_gwInfoReports.pop_front();
}

void UnitTestCommonBase::apiProcessData(CC_MqttsnClient* client, const unsigned char* buf, unsigned bufLen)
{
    m_funcs.m_process_data(client, buf, bufLen);
}

void UnitTestCommonBase::unitTestTickProgramCb(void* data, unsigned duration)
{
    auto* thisPtr = asThis(data);
    if (thisPtr->m_ticks.empty()) {
        asThis(data)->m_ticks.emplace_back(duration);
        return;
    }

    auto& info = thisPtr->m_ticks.front(); 
    test_assert(info.m_req == 0U);
    info.m_req = duration;
}

unsigned UnitTestCommonBase::unitTestCancelTickWaitCb(void* data)
{
    auto* thisPtr = asThis(data);
    test_assert(!thisPtr->m_ticks.empty());
    auto result = thisPtr->m_ticks.front().m_elapsed;
    thisPtr->m_ticks.pop_front();
    return result;
}

void UnitTestCommonBase::unitTestSendOutputDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius)
{
    // TODO:
    static_cast<void>(data);
    static_cast<void>(buf);
    static_cast<void>(bufLen);
    static_cast<void>(broadcastRadius);
    test_assert(false);
}

void UnitTestCommonBase::unitTestGwStatusReportCb(void* data, CC_MqttsnGwStatus status, const CC_MqttsnGatewayInfo* info)
{
    asThis(data)->m_gwInfoReports.emplace_back(status, info);
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