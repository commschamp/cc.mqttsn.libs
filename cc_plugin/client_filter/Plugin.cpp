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


#include "Plugin.h"

#include "widget/FilterConfigWidget.h"

#include <iostream>

namespace cc = comms_champion;

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

namespace
{

const QString MainConfigKey("mqttsn_client_filter");
const QString ClientIdKey("client_id");
const QString KeepAliveKey("keep_alive");
const QString RetryDurationKey("retry_duration");
const QString RetryCountKey("retry_count");
const QString PubKey("pub");
const QString SubsKey("subs");
const QString TopicKey("topic");
const QString TopicIdKey("topic_id");
const QString QosKey("qos");
const QString RetainKey("retain");

}  // namespace

Plugin::Plugin()
{
    pluginProperties()
        .setFiltersCreateFunc(
            [this]() -> ListOfFilters
            {
                createFilterIfNeeded();
                ListOfFilters list;
                assert(m_filter);
                list.append(m_filter);
                return list;
            })
        .setConfigWidgetCreateFunc(
            [this]() -> QWidget*
            {
                createFilterIfNeeded();
                return new FilterConfigWidget(*m_filter);

            });
}

Plugin::~Plugin() = default;

void Plugin::getCurrentConfigImpl(QVariantMap& config)
{
    createFilterIfNeeded();

    QVariantMap subConfig;
    subConfig.insert(ClientIdKey, QString::fromStdString(m_filter->clientId()));
    subConfig.insert(KeepAliveKey, m_filter->keepAlivePeriod());
    subConfig.insert(RetryDurationKey, m_filter->retryPeriod());
    subConfig.insert(RetryCountKey, m_filter->retryCount());

    auto& pubInfo = m_filter->defaultPubInfo();
    if ((!pubInfo.m_topic.isEmpty()) || (pubInfo.m_topicId != 0)) {
        QVariantMap publishConfig;
        if (!pubInfo.m_topic.isEmpty()) {
            publishConfig.insert(TopicKey, pubInfo.m_topic);
        }
        else {
            publishConfig.insert(TopicIdKey, pubInfo.m_topicId);
        }
        publishConfig.insert(QosKey, pubInfo.m_qos);
        publishConfig.insert(RetainKey, pubInfo.m_retain);
        subConfig.insert(PubKey, std::move(publishConfig));
    }

    QVariantList subsList;
    auto& subInfos = m_filter->subInfos();
    for (auto& sub : subInfos) {
        if (sub.m_topic.isEmpty() && (sub.m_topicId == 0U)) {
            continue;
        }

        QVariantMap subscribeConfig;
        if (!sub.m_topic.isEmpty()) {
            subscribeConfig.insert(TopicKey, sub.m_topic);
        }
        else {
            subscribeConfig.insert(TopicIdKey, sub.m_topicId);
        }

        subscribeConfig.insert(QosKey, sub.m_qos);
        subsList.append(std::move(subscribeConfig));
    }

    if (!subsList.isEmpty()) {
        subConfig.insert(SubsKey, std::move(subsList));
    }

    config.insert(MainConfigKey, QVariant::fromValue(subConfig));
}

void Plugin::reconfigureImpl(const QVariantMap& config)
{
    auto subConfigVar = config.value(MainConfigKey);
    if ((!subConfigVar.isValid()) || (!subConfigVar.canConvert<QVariantMap>())) {
        return;
    }

    createFilterIfNeeded();
    assert(m_filter);

    auto subConfig = subConfigVar.value<QVariantMap>();
    auto clientIdVar = subConfig.value(ClientIdKey);
    if (clientIdVar.isValid() && clientIdVar.canConvert<QString>()) {
        auto clientId = clientIdVar.value<QString>();
        m_filter->clientId() = clientId.toStdString();
    }

    auto keepAliveVar = subConfig.value(KeepAliveKey);
    if (keepAliveVar.isValid() && keepAliveVar.canConvert<Filter::KeepAliveType>()) {
        m_filter->keepAlivePeriod() = keepAliveVar.value<Filter::KeepAliveType>();
    }

    auto retryDurationVar = subConfig.value(RetryDurationKey);
    if (retryDurationVar.isValid() && retryDurationVar.canConvert<int>()) {
        m_filter->retryPeriod() = retryDurationVar.value<int>();
    }

    auto retryCountVar = subConfig.value(RetryCountKey);
    if (retryCountVar.isValid() && retryCountVar.canConvert<int>()) {
        m_filter->retryCount() = retryCountVar.value<int>();
    }

    auto pubVar = subConfig.value(PubKey);
    if (pubVar.isValid() && pubVar.canConvert<QVariantMap>()) {
        auto pubInfoMap = pubVar.value<QVariantMap>();
        Filter::PubSubInfo pubInfo;
        pubInfo.m_qos = static_cast<decltype(pubInfo.m_qos)>(pubInfoMap.value(QosKey).toInt());
        pubInfo.m_retain = pubInfoMap.value(RetainKey).toBool();

        auto topic = pubInfoMap.value(TopicKey).toString();
        auto topicId = pubInfoMap.value(TopicIdKey).toInt();

        if (!topic.isEmpty()) {
            pubInfo.m_topic = topic;
            m_filter->defaultPubInfo() = std::move(pubInfo);
        }
        else if (topicId != 0) {
            pubInfo.m_topicId = topicId;
            m_filter->defaultPubInfo() = std::move(pubInfo);
        }
    }

    auto subsVar = subConfig.value(SubsKey);
    do {
        if ((!subsVar.isValid()) || (!subsVar.canConvert<QVariantList>())) {
            break;
        }

        QVariantList subsList = subsVar.value<QVariantList>();
        if (subsList.isEmpty()) {
            break;
        }

        for (auto& subVar : subsList) {
            if ((!subVar.isValid()) || (!subVar.canConvert<QVariantMap>())) {
                continue;
            }

            QVariantMap subInfoMap = subVar.value<QVariantMap>();
            Filter::PubSubInfo subInfo;
            subInfo.m_qos = static_cast<decltype(subInfo.m_qos)>(subInfoMap.value(QosKey).toInt());

            auto topic = subInfoMap.value(TopicKey).toString();
            auto topicId = subInfoMap.value(TopicIdKey).toInt();

            if (!topic.isEmpty()) {
                subInfo.m_topic = topic;
                m_filter->subInfos().append(std::move(subInfo));
            }
            else if (topicId != 0) {
                subInfo.m_topicId = topicId;
                m_filter->subInfos().append(std::move(subInfo));
            }
        }
    } while (false);
}

void Plugin::createFilterIfNeeded()
{
    if (!m_filter) {
        m_filter.reset(new Filter());
    }
}


}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn

