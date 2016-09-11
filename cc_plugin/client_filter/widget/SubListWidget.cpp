//
// Copyright 2014 - 2016 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "SubListWidget.h"

#include <algorithm>
#include <cassert>
#include <iterator>

#include "PubSubWidget.h"

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

SubListElementWidget::SubListElementWidget(PubSubInfo& info, QWidget* parentObj)
  : Base(parentObj)
{
    m_ui.setupUi(this);
    m_ui.m_layout->addWidget(new PubSubWidget(info, true));
    connect(
        m_ui.m_removePushButton, SIGNAL(clicked()),
        this, SIGNAL(sigRemoveRequested()));

}

SubListElementWidget::~SubListElementWidget() = default;

SubListWidget::SubListWidget(QList<PubSubInfo>& subs, QWidget* parentObj)
  : Base(parentObj),
    m_subs(subs)
{
    m_ui.setupUi(this);

    for (auto& sub : m_subs) {
        addElemWidget(sub);
    }

    connect(
        m_ui.m_addPushButton, SIGNAL(clicked()),
        this, SLOT(addNew()));
}

SubListWidget::~SubListWidget() = default;

void SubListWidget::addNew()
{
    PubSubInfo info;
    info.m_qos = MqttsnQoS_ExactlyOnceDelivery;
    m_subs.append(info);
    addElemWidget(m_subs.back());
}

void SubListWidget::remove()
{
    auto* sigSender = qobject_cast<SubListElementWidget*>(sender());
    auto iter = std::find(m_elements.begin(), m_elements.end(), sigSender);
    if (iter == m_elements.end()) {
        assert(!"Something is not right");
        return;
    }

    auto idx = static_cast<int>(std::distance(m_elements.begin(), iter));
    m_subs.removeAt(idx);
    delete *iter;
    m_elements.erase(iter);
    assert(m_elements.size() == (unsigned)m_ui.m_layout->count() - 2);
}

void SubListWidget::addElemWidget(PubSubInfo& sub)
{
    auto* elemWidget = new SubListElementWidget(sub);
    m_ui.m_layout->insertWidget(m_ui.m_layout->count() - 2, elemWidget);
    m_elements.push_back(elemWidget);
    connect(
        elemWidget, SIGNAL(sigRemoveRequested()),
        this, SLOT(remove()));
}

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn



