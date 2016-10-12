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

#include "RegMgr.h"

#include <cassert>
#include <limits>
#include <algorithm>

namespace mqttsn
{

namespace gateway
{

bool RegMgr::setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal)
{
    if (maxVal < minVal) {
        return false;
    }

    m_minTopicId = std::max(minVal, std::uint16_t(DefaultMinTopicId));
    m_maxTopicId = std::min(maxVal, std::uint16_t(DefaultMaxTopicId));
    return true;
}

bool RegMgr::regPredefined(const std::string& topic, std::uint16_t topicId)
{
    if (topicId == 0U) {
        return false;
    }

    auto iter = m_regInfosMap.find(topic);
    auto revIter = m_regInfosRevMap.find(topicId);
    if ((iter == m_regInfosMap.end()) &&
        (revIter == m_regInfosRevMap.end())) {
        RegInfo info;
        info.m_topic = topic;
        info.m_topicId = topicId;
        info.m_predefined = true;
        m_regInfos.push_back(std::move(info));
        auto lastIter = m_regInfos.end();
        --lastIter;
        assert(lastIter != m_regInfos.end());
        assert(lastIter->m_topic == topic);
        assert(lastIter->m_topicId == topicId);
        assert(lastIter->m_predefined);

        m_regInfosMap.insert(std::make_pair(topic, lastIter));
        m_regInfosRevMap.insert(std::make_pair(topicId, lastIter));
        return true;
    }

    if (iter->second != revIter->second) {
        return false;
    }

    auto infoIter = iter->second;
    if ((infoIter->m_topic != topic) || (infoIter->m_topicId != topicId)) {
        return false;
    }

    infoIter->m_predefined = true;
    return true;
}

std::uint16_t RegMgr::mapTopicNoInfo(const std::string& topic)
{
    return mapTopic(topic).m_topicId;
}

RegMgr::TopicInfo RegMgr::mapTopic(const std::string& topic)
{
    TopicInfo retInfo;

    auto iter = m_regInfosMap.find(topic);
    if (iter != m_regInfosMap.end()) {
        auto infoIter = iter->second;
        assert(infoIter->m_topic == topic);
        retInfo.m_topicId = infoIter->m_topicId;
        retInfo.m_predefined = infoIter->m_predefined;
        retInfo.m_newInsersion = false;
        return retInfo;
    }

    std::uint16_t topicId = 0;
    do {
        if (m_regInfos.empty()) {
            topicId = m_minTopicId;
            break;
        }

        assert(!m_regInfosMap.empty());
        assert(!m_regInfosRevMap.empty());
        auto lastIt = m_regInfosRevMap.rbegin();
        if (lastIt->first < m_maxTopicId) {
            topicId = std::max(std::uint16_t(lastIt->first + 1), m_minTopicId);
            break;
        }

        auto firstIt = m_regInfosRevMap.begin();
        if (m_minTopicId < firstIt->first) {
            topicId = std::min(std::uint16_t(firstIt->first - 1), m_maxTopicId);
            break;
        }

        std::uint16_t prevTopicId = 0;
        for (auto it = m_regInfosRevMap.begin(); it != m_regInfosRevMap.end(); ++it) {
            auto expId = prevTopicId + 1;
            if ((it->first != expId) &&
                (m_minTopicId <= expId) &&
                (expId <= m_maxTopicId)) {
                topicId = expId;
                break;
            }
            prevTopicId = expId;
        }

        if (topicId != 0) {
            break;
        }

        auto it =
            std::find_if(
                m_regInfosRevMap.begin(), m_regInfosRevMap.end(),
                [this](decltype(m_regInfosRevMap)::const_reference elem) -> bool
                {
                    auto regInfoIter = elem.second;
                    return (!regInfoIter->m_predefined) &&
                           (m_minTopicId <= regInfoIter->m_topicId) &&
                           (regInfoIter->m_topicId <= m_maxTopicId);
                });
        topicId = it->first;
        auto infoIter = it->second;
        removeFromTopicMap(topic, infoIter);
        m_regInfosRevMap.erase(it);
        m_regInfos.erase(infoIter);

    } while (false);
    assert(topicId != 0);

    RegInfo info;
    info.m_topic = topic;
    info.m_topicId = topicId;
    info.m_predefined = false;
    m_regInfos.push_back(std::move(info));
    auto infoIter = m_regInfos.end();
    --infoIter;
    m_regInfosMap.insert(std::make_pair(topic, infoIter));
    m_regInfosRevMap.insert(std::make_pair(topicId, infoIter));

    retInfo.m_topicId = topicId;
    retInfo.m_predefined = false;
    retInfo.m_newInsersion = true;
    return retInfo;
}

void RegMgr::discardRegistration(std::uint16_t topicId)
{
    auto idIter = m_regInfosRevMap.find(topicId);
    if (idIter == m_regInfosRevMap.end()) {
        return;
    }

    auto infoIter = idIter->second;
    assert(infoIter != m_regInfos.end());

    auto topicIter = m_regInfosMap.find(infoIter->m_topic);
    assert(topicIter != m_regInfosMap.end());
    assert(topicIter->second == infoIter);

    m_regInfosMap.erase(topicIter);
    m_regInfosRevMap.erase(idIter);
    m_regInfos.erase(infoIter);
}

const std::string& RegMgr::mapTopicId(std::uint16_t topicId)
{
    auto idIter = m_regInfosRevMap.find(topicId);
    if (idIter == m_regInfosRevMap.end()) {
        static const std::string EmptyString;
        return EmptyString;
    }

    auto infoIter = idIter->second;
    assert(infoIter != m_regInfos.end());

    return infoIter->m_topic;
}

void RegMgr::removeFromTopicMap(const std::string& topic, RegInfosList::iterator info)
{
    auto topicMapIters = m_regInfosMap.equal_range(topic);
    for (auto iter = topicMapIters.first; iter != topicMapIters.second; ++iter) {
        if (iter->second == info) {
            m_regInfosMap.erase(iter);
            return;
        }
    }
}

}  // namespace gateway

}  // namespace mqttsn


