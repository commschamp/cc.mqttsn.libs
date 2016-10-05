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

    m_minTopicId = minVal;
    m_maxTopicId = maxVal;
    return true;
}

bool RegMgr::regPredefined(const std::string& topic, std::uint16_t topicId)
{
    if (topicId == 0U) {
        return false;
    }

    auto revIter = m_regInfosRevMap.find(topicId);
    if (revIter != m_regInfosRevMap.end()) {
        return false;
    }

    auto iters = m_regInfosMap.equal_range(topic);
    for (auto it = iters.first; it != iters.second; ++it) {
        auto regInfoIter = it->second;
        if (regInfoIter->m_type == Type::Predefined) {
            return false;
        }
    }

    RegInfo info;
    info.m_topic = topic;
    info.m_topicId = topicId;
    info.m_type = Type::Predefined;
    m_regInfos.push_back(std::move(info));
    auto lastIter = m_regInfos.end();
    --lastIter;
    assert(lastIter != m_regInfos.end());
    assert(lastIter->m_topic == topic);
    assert(lastIter->m_topicId == topicId);
    assert(lastIter->m_type == Type::Predefined);

    m_regInfosMap.insert(std::make_pair(topic, lastIter));
    m_regInfosRevMap.insert(std::make_pair(topicId, lastIter));
    return true;

}

std::tuple<std::uint16_t, bool> RegMgr::mapTopic(const std::string& topic, Type type)
{
    assert(type != Type::Predefined);
    assert(type != Type::Invalid);

    auto iters = m_regInfosMap.equal_range(topic);
    do {
        if (iters.first == iters.second) {
            break;
        }

        for (auto it = iters.first; it != iters.second; ++it) {
            auto regInfoIter = it->second;
            assert(regInfoIter != m_regInfos.end());
            assert(regInfoIter->m_topic == topic);
            if (regInfoIter->m_type != Type::Predefined) {
                assert(regInfoIter->m_topicId != 0);
                return std::make_tuple(regInfoIter->m_topicId, false);
            }
        }

    } while (false);

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
            topicId = lastIt->first + 1;
            break;
        }

        auto firstIt = m_regInfosRevMap.begin();
        if (m_minTopicId < firstIt->first) {
            topicId = firstIt->first - 1;
            break;
        }

        if (std::numeric_limits<decltype(topicId)>::max() <= m_regInfosRevMap.size()) {
            auto it =
                std::find_if(
                    m_regInfosRevMap.begin(), m_regInfosRevMap.end(),
                    [this](decltype(m_regInfosRevMap)::const_reference elem) -> bool
                    {
                        auto regInfoIter = elem.second;
                        return (regInfoIter->m_type != Type::Predefined) &&
                               (m_minTopicId <= regInfoIter->m_topicId) &&
                               (regInfoIter->m_topicId <= m_maxTopicId);
                    });
            topicId = it->first;
            auto infoIter = it->second;
            removeFromTopicMap(topic, infoIter);
            m_regInfosRevMap.erase(it);
            m_regInfos.erase(infoIter);
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

    } while (false);
    assert(topicId != 0);

    RegInfo info;
    info.m_topic = topic;
    info.m_topicId = topicId;
    info.m_type = type;
    m_regInfos.push_back(std::move(info));
    auto infoIter = m_regInfos.end();
    --infoIter;
    m_regInfosMap.insert(std::make_pair(topic, infoIter));
    m_regInfosRevMap.insert(std::make_pair(topicId, infoIter));
    return std::make_tuple(topicId, true);
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


