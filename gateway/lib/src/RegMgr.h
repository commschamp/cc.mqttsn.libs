//
// Copyright 2016 - 2024 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <cstdint>
#include <list>
#include <map>
#include <tuple>

#include "cc_mqttsn/field/FlagsCommon.h"

namespace cc_mqttsn_gateway
{

class RegMgr
{
public:

    using TopicIdType = cc_mqttsn::field::TopicIdTypeVal;
    struct TopicInfo
    {
        std::uint16_t m_topicId = 0;
        TopicIdType m_topicIdType = TopicIdType::Normal;
        bool m_newInsersion = false;
    };

    bool setTopicIdAllocationRange(std::uint16_t minVal, std::uint16_t maxVal);
    bool regPredefined(const std::string& topic, std::uint16_t topicId);
    std::uint16_t mapTopicNoInfo(const std::string& topic);
    TopicInfo mapTopic(const std::string& topic);
    void discardRegistration(std::uint16_t topicId);
    const std::string& mapTopicId(std::uint16_t topicId);
    void clearRegistrations();

private:

    struct RegInfo
    {
        std::string m_topic;
        std::uint16_t m_topicId = 0U;
        TopicIdType m_topicIdType = TopicIdType::Normal;
    };
    typedef std::list<RegInfo> RegInfosList;

    void removeFromTopicMap(const std::string& topic, RegInfosList::iterator info);

    RegInfosList m_regInfos;
    std::map<std::string, RegInfosList::iterator> m_regInfosMap;
    std::map<std::uint16_t, RegInfosList::iterator> m_regInfosRevMap;

    std::uint16_t m_minTopicId = DefaultMinTopicId;
    std::uint16_t m_maxTopicId = DefaultMaxTopicId;

    static const std::uint16_t DefaultMinTopicId = 1;
    static const std::uint16_t DefaultMaxTopicId = 0xfffe;
};

}  // namespace cc_mqttsn_gateway
