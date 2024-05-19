//
// Copyright 2024 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "GatewaySession.h"

namespace cc_mqttsn_gateway_app
{

namespace 
{

const std::string WildcardStr("*");

} // namespace 
    

GatewaySession::GatewaySession(
    boost::asio::io_context& io, 
    GatewayLogger& logger, 
    const cc_mqttsn_gateway::Config& config,
    GatewayIoClientSocketPtr clientSocket) :
    m_io(io),
    m_logger(logger),
    m_config(config),
    m_timer(io),
    m_clientSocket(std::move(clientSocket)),
    m_sessionPtr(std::make_unique<cc_mqttsn_gateway::Session>()),
    m_session(m_sessionPtr.get())
{
}    

GatewaySession::GatewaySession(
    boost::asio::io_context& io, 
    GatewayLogger& logger, 
    const cc_mqttsn_gateway::Config& config, 
    cc_mqttsn_gateway::Session* session) : 
    m_io(io),
    m_logger(logger),
    m_config(config),
    m_timer(io),
    m_session(session)
{
}

bool GatewaySession::start()  
{
    assert(m_termReqCb);
    if (!startSession()) {
        return false;
    }

    if (m_clientSocket) {
        m_clientSocket->setDataReportCb(
            [this](const std::uint8_t* buf, std::size_t bufSize)
            {
                assert(m_sessionPtr.get() == m_session);
                [[maybe_unused]] auto consumed = m_session->dataFromClient(buf, bufSize);
            });    

        if (!m_clientSocket->start()) {
            m_logger.error() << "Failed to start client socket" << std::endl;
            return false;
        }      
    }  

    doBrokerConnect();
    return true;
}  

void GatewaySession::doTerminate()
{
    boost::asio::post(
        m_io,
        [this]()
        {
            assert(m_termReqCb);
            m_termReqCb();
        });
}

void GatewaySession::doBrokerConnect()
{
    if (m_brokerConnected) {
        m_session->setBrokerConnected(false);
    }

    m_brokerSocket = GatewayIoBrokerSocket::create(m_io, m_logger, m_config);
    if (!m_brokerSocket) {
        m_logger.error() << "Failed to allocate broker socket " << std::endl;
        doTerminate();
        return;
    }    

    m_brokerSocket->setDataReportCb(
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            auto actBuf = buf;
            auto actSize = bufSize;

            if (!m_brokerData.empty()) {
                m_brokerData.insert(m_brokerData.end(), buf, buf + bufSize);
                actBuf = m_brokerData.data();
                actSize = m_brokerData.size();
            }

            auto consumed = m_session->dataFromBroker(actBuf, actSize);
            if (actSize <= consumed) {
                m_brokerData.clear();
                return;
            }

            if (m_brokerData.empty()) {
                m_brokerData.assign(buf + consumed, buf + bufSize);
                return;
            }

            m_brokerData.erase(m_brokerData.begin(), m_brokerData.begin() + consumed);
        });

    m_brokerSocket->setConnectedReportCb(
        [this]()
        {
            m_session->setBrokerConnected(true);
        }
    );        

    m_brokerSocket->setErrorReportCb(
        [this]()
        {
            if (m_brokerConnected) {
                m_session->setBrokerConnected(false);
            }
            
            doBrokerReconnect();
        });    

    if (!m_brokerSocket->start()) {
        m_logger.error() << "Failed to start TCP/IP socket" << std::endl;
        doTerminate();
        return;
    }
}

void GatewaySession::doBrokerReconnect()
{
    boost::asio::post(
        m_io,
        [this]()
        {
            doBrokerConnect();
        });
}

bool GatewaySession::startSession()
{
    auto topicIdsAllocRange = m_config.topicIdAllocRange();

    m_session->setGatewayId(m_config.gatewayId());
    m_session->setRetryPeriod(m_config.retryPeriod());
    m_session->setRetryCount(m_config.retryCount());
    m_session->setSleepingClientMsgLimit(m_config.sleepingClientMsgLimit());
    m_session->setDefaultClientId(m_config.defaultClientId());
    m_session->setPubOnlyKeepAlive(m_config.pubOnlyKeepAlive());
    m_session->setTopicIdAllocationRange(topicIdsAllocRange.first, topicIdsAllocRange.second);

    m_session->setNextTickProgramReqCb(
        [this](unsigned ms)
        {
            m_tickReqTs = TimestampClock::now();
            m_timer.expires_after(std::chrono::milliseconds(ms));
            m_timer.async_wait(
                [this](const boost::system::error_code& ec)
                {
                    if (ec == boost::asio::error::operation_aborted) {
                        return;
                    }

                    m_session->tick();
                });
        });

    m_session->setCancelTickWaitReqCb(
        [this]() -> unsigned
        {
            boost::system::error_code ec;
            m_timer.cancel(ec);
            assert(m_tickReqTs != Timestamp());
            auto now = TimestampClock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_tickReqTs);
            return static_cast<unsigned>(elapsed.count());
        }
    );    


    m_session->setSendDataBrokerReqCb(
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            assert(m_brokerSocket);
            m_brokerSocket->sendData(buf, bufSize);
        }); 

    m_session->setBrokerReconnectReqCb(
        [this]()
        {
            doBrokerReconnect();
        }); 

    m_session->setClientConnectedReportCb(
        [this](const std::string& clientId)
        {
            auto& predefinedTopics = m_config.predefinedTopics();

            auto applyForClient = 
                [this, &predefinedTopics](const std::string& id)
                {
                    auto iter = 
                        std::lower_bound(
                            predefinedTopics.begin(), predefinedTopics.end(), id,
                            [](auto& info, const std::string& idParam)
                            {
                                return info.clientId < idParam;
                            });

                    if ((iter == predefinedTopics.end()) || (iter->clientId != id)) {
                        return;
                    }

                    while (iter != predefinedTopics.end()) {
                        if (iter->clientId != id) {
                            break;
                        }

                        m_session->addPredefinedTopic(iter->topic, iter->topicId);
                        ++iter;
                    }
                };
            
            applyForClient(clientId);
            applyForClient(WildcardStr);
        });
    
    m_session->setAuthInfoReqCb(
        [this](const std::string& clientId)
        {
            return getAuthInfoFor(clientId);
        }
    );     

    m_session->setFwdEncSessionCreatedReportCb(
        [this](cc_mqttsn_gateway::Session* fwdEncSession) -> bool
        {
            m_fwdEncSessions.push_back(std::make_unique<GatewaySession>(m_io, m_logger, m_config, fwdEncSession));
            auto& sessionPtr = m_fwdEncSessions.back();
            if (!sessionPtr->start()) {
                m_logger.error() << "Failed to start forwarder encapsulated session" << std::endl;
                return false;
            }

            return true;
        }); 

    m_session->setFwdEncSessionDeletedReportCb(
        [this](cc_mqttsn_gateway::Session* fwdEncSession)
        {
            boost::asio::post(
                m_io,
                [this, fwdEncSession]()
                {
                    auto iter = 
                        std::find_if(
                            m_fwdEncSessions.begin(), m_fwdEncSessions.end(),
                            [fwdEncSession](auto& sPtr)
                            {
                                return sPtr->m_session == fwdEncSession;
                            });

                    assert(iter != m_fwdEncSessions.end());
                    if (iter == m_fwdEncSessions.end()) {
                        return;
                    }            

                    m_fwdEncSessions.erase(iter);
                });
        }); 

    if (!m_sessionPtr) {
        // Forwarder encapsulated session
        return true;
    }   

    assert(m_sessionPtr.get() == m_session);
    m_session->setSendDataClientReqCb(
        [this](const std::uint8_t* buf, std::size_t bufSize, unsigned broadcastRadius)
        {
            assert(m_clientSocket);
            m_clientSocket->sendData(buf, bufSize, broadcastRadius);
        });  

    m_session->setTerminationReqCb(
        [this]()
        {
            doTerminate();
        });            


    if (!m_session->start()) {
        m_logger.error() << "Failed to start client session" << std::endl;
        return false;
    }

    return true;
}

GatewaySession::AuthInfo GatewaySession::getAuthInfoFor(const std::string& clientId)
{
    auto& authInfos = m_config.authInfos();

    auto findElemFunc =
        [&authInfos](const std::string& cId)
        {
            return std::lower_bound(
                authInfos.begin(), authInfos.end(), cId,
                [](auto& elem, const std::string& val) -> bool
                {
                    return elem.clientId < val;
                });
        };

    auto iter = findElemFunc(clientId);
    if ((iter == authInfos.end()) ||
        (iter->clientId != clientId)) {
        iter = findElemFunc(WildcardStr);
    }

    if ((iter == authInfos.end()) ||
        (iter->clientId != clientId)) {
        return AuthInfo();
    }

    using BinaryData = cc_mqttsn_gateway::Session::BinaryData;
    BinaryData data;
    data.reserve(iter->password.size());

    unsigned pos = 0U;
    while (pos < iter->password.size()) {
        auto remSize = iter->password.size() - pos;
        const char* remStr = &iter->password[pos];

        static const std::string BackSlashStr("\\\\");
        if ((BackSlashStr.size() <= remSize) &&
            (std::equal(BackSlashStr.begin(), BackSlashStr.end(), remStr))) {
            data.push_back(static_cast<std::uint8_t>('\\'));
            pos = static_cast<decltype(pos)>(pos + BackSlashStr.size());
            continue;
        }

        static const std::string HexNumStr("\\x");
        static const std::size_t HexNumSize = HexNumStr.size() + 2;
        if ((HexNumSize <= remSize) &&
            (std::equal(HexNumStr.begin(), HexNumStr.end(), remStr))) {
            try {
                auto* numStrBegin = &remStr[2];
                auto* numStrEnd = numStrBegin + 2;
                std::string numStr(numStrBegin, numStrEnd);
                auto byte = static_cast<std::uint8_t>(std::stoul(numStr));
                data.push_back(byte);
                pos = static_cast<decltype(pos)>(pos + HexNumSize);
                continue;
            }
            catch (...) {
                // do nothing, fall through
            }
        }

        data.push_back(static_cast<std::uint8_t>(iter->password[pos]));
        ++pos;
    }

    return std::make_pair(iter->username, std::move(data));
}

} // namespace cc_mqttsn_gateway_app
