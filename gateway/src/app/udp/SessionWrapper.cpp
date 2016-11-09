//
// Copyright 2016 (C). Alex Robenko. All rights reserved.
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

#include "SessionWrapper.h"

#include <iostream>
#include <cassert>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

namespace app
{

namespace udp
{

namespace
{

const std::string WildcardStr("*");

}  // namespace

SessionWrapper::SessionWrapper(
    const Config& config,
    QObject* parent)
  : Base(parent),
    m_config(config)
{
    m_session.setNextTickProgramReqCb(
        [this](unsigned ms)
        {
            programNextTick(ms);
        });

    m_session.setCancelTickWaitReqCb(
        [this]() -> unsigned
        {
            return cancelTick();
        });

    m_session.setSendDataBrokerReqCb(
        [this](const std::uint8_t* buf, std::size_t bufSize)
        {
            sendDataToBroker(buf, bufSize);
        });

    m_session.setTerminationReqCb(
        [this]()
        {
            termSession();
        });

    m_session.setBrokerReconnectReqCb(
        [this]()
        {
            reconnectBroker();
        });

    m_session.setClientConnectedReportCb(
        [this](const std::string& clientId)
        {
            addPredefinedTopicsFor(clientId);
        });

    m_session.setAuthInfoReqCb(
        [this](const std::string& clientId) -> AuthInfo
        {
            return getAuthInfoFor(clientId);
        });

    m_session.setGatewayId(m_config.gatewayId());
    m_session.setRetryPeriod(m_config.retryPeriod());
    m_session.setRetryCount(m_config.retryCount());
    m_session.setDefaultClientId(m_config.defaultClientId());
    m_session.setPubOnlyKeepAlive(m_config.pubOnlyKeepAlive());
    m_session.setSleepingClientMsgLimit(m_config.sleepingClientMsgLimit());

    auto topicIdAllocRange = m_config.topicIdAllocRange();
    m_session.setTopicIdAllocationRange(topicIdAllocRange.first, topicIdAllocRange.second);

    addPredefinedTopicsFor(WildcardStr);

    connect(
        &m_timer, SIGNAL(timeout()),
        this, SLOT(tickTimeout()));

    connect(
        &m_brokerSocket, SIGNAL(connected()),
        this, SLOT(brokerConnected()));
    connect(
        &m_brokerSocket, SIGNAL(disconnected()),
        this, SLOT(brokerDisconnected()));
    connect(
        &m_brokerSocket, SIGNAL(readyRead()),
        this, SLOT(readFromBrokerSocket()));
    connect(
        &m_brokerSocket, SIGNAL(error(QAbstractSocket::SocketError)),
        this, SLOT(brokerSocketErrorOccurred(QAbstractSocket::SocketError)));

}

SessionWrapper::~SessionWrapper() = default;

bool SessionWrapper::start()
{
    if (!m_session.start()) {
        std::cerr << "Failed to start new session" << std::endl;
        return false;
    }

    connectToBroker();
    return true;
}

void SessionWrapper::tickTimeout()
{
    m_session.tick(m_reqTicks);
}

void SessionWrapper::brokerConnected()
{
    m_session.setBrokerConnected(true);
    m_reconnectRequested = false;
}

void SessionWrapper::brokerDisconnected()
{
    m_session.setBrokerConnected(false);
    if (m_reconnectRequested) {
        connectToBroker();
    }
}

void SessionWrapper::readFromBrokerSocket()
{
    auto data = m_brokerSocket.readAll();

    if (m_terminating) {
        return;
    }

    auto* buf = reinterpret_cast<const std::uint8_t*>(data.constData());
    std::size_t bufSize = data.size();

    if (!m_brokerData.empty()) {
        m_brokerData.insert(m_brokerData.end(), buf, buf + bufSize);
        buf = &m_brokerData[0];
        bufSize = m_brokerData.size();
    }

    std::size_t consumed = m_session.dataFromBroker(buf, bufSize);
    if (bufSize <= consumed) {
        m_brokerData.clear();
        return;
    }

    if (!m_brokerData.empty()) {
        m_brokerData.erase(m_brokerData.begin(), m_brokerData.begin() + consumed);
        return;
    }

    m_brokerData.assign(buf + consumed, buf + bufSize);
}

void SessionWrapper::brokerSocketErrorOccurred(QAbstractSocket::SocketError err)
{
    static_cast<void>(err);
    std::cerr << "ERROR: TCP Socket: " << m_brokerSocket.errorString().toStdString() << std::endl;
}

void SessionWrapper::programNextTick(unsigned ms)
{
    assert(!m_terminating);
    m_reqTicks = ms;
    m_timer.setSingleShot(true);
    m_timer.start(ms);
}

unsigned SessionWrapper::cancelTick()
{
    auto rem = m_timer.remainingTime();
    unsigned result = 0U;
    do {
        if (static_cast<decltype(rem)>(m_reqTicks) <= rem) {
            break;
        }

        result = m_reqTicks - rem;
    } while (false);

    m_timer.stop();
    return result;
}

void SessionWrapper::sendDataToBroker(const std::uint8_t* buf, std::size_t bufSize)
{
    std::size_t writtenCount = 0;
    while (writtenCount < bufSize) {
        auto remSize = bufSize - writtenCount;
        auto count =
            m_brokerSocket.write(
                reinterpret_cast<const char*>(&buf[writtenCount]),
                remSize);
        if (count < 0) {
            std::cerr << "Failed to write to TCP socket" << std::endl;
            return;
        }

        writtenCount += count;
    }
}

void SessionWrapper::termSession()
{
    if (m_terminating) {
        return;
    }

    m_terminating = true;
    m_timer.stop();
    m_brokerSocket.blockSignals(true);
    m_brokerSocket.flush();
    m_brokerSocket.disconnectFromHost();
    assert(m_termNotifyCb);
    m_termNotifyCb(*this);
    deleteLater();
}

void SessionWrapper::reconnectBroker()
{
    m_reconnectRequested = true;
    assert(m_brokerSocket.state() == QTcpSocket::ConnectedState);
    m_brokerSocket.disconnectFromHost();
}

void SessionWrapper::connectToBroker()
{
    auto host = QString::fromStdString(m_config.brokerTcpHostAddress());
    auto port = m_config.brokerTcpHostPort();
    m_brokerSocket.connectToHost(host, port);
}

void SessionWrapper::addPredefinedTopicsFor(const std::string& clientId)
{
    auto& predefinedTopics = m_config.predefinedTopics();
    auto iter = std::lower_bound(
        predefinedTopics.begin(), predefinedTopics.end(), clientId,
        [](const Config::PredefinedTopicsList::const_reference elem, const std::string& cId) -> bool
        {
            return elem.clientId < cId;
        });

    while (iter != predefinedTopics.end()) {
        if (iter->clientId != clientId) {
            return;
        }

        m_session.addPredefinedTopic(iter->topic, iter->topicId);
        ++iter;
    }
}

SessionWrapper::AuthInfo SessionWrapper::getAuthInfoFor(const std::string& clientId)
{
    auto& authInfos = m_config.authInfos();

    auto findElemFunc =
        [&authInfos](const std::string& cId) -> Config::AuthInfosList::const_iterator
        {
            return std::lower_bound(
                authInfos.begin(), authInfos.end(), cId,
                [](Config::AuthInfosList::const_reference elem, const std::string& val) -> bool
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

    typedef decltype(m_session)::BinaryData BinaryData;
    BinaryData data;
    data.reserve(iter->password.size());

    unsigned pos;
    while (pos < iter->password.size()) {
        auto remSize = iter->password.size() - pos;
        const char* remStr = &iter->password[pos];

        static const std::string BackSlashStr("\\");
        if ((BackSlashStr.size() <= remSize) &&
            (std::equal(BackSlashStr.begin(), BackSlashStr.end(), remStr))) {
            data.push_back(static_cast<std::uint8_t>('\\'));
            pos += BackSlashStr.size();
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
                pos += HexNumSize;
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

}  // namespace udp

}  // namespace app

}  // namespace gateway

}  // namespace mqttsn


