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


#include "PubSubWidget.h"


namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

PubSubWidget::PubSubWidget(PubSubInfo& info, bool sub)
  : m_info(info)
{
    m_ui.setupUi(this);

    if (sub) {
        m_ui.m_qosLabel->setText(QApplication::translate("PubSubWidget", "Max. QoS", 0));
        m_ui.m_groupBox->setTitle(QApplication::translate("PubSubWidget", "Subscribe", 0));
        m_ui.m_retainWidget->hide();
    }

    m_ui.m_topicLineEdit->setText(m_info.m_topic);
    m_ui.m_topicIdSpinBox->setValue(m_info.m_topicId);
    m_ui.m_qosComboBox->setCurrentIndex(m_info.m_qos);
    m_ui.m_reatainCheckBox->setChecked(m_info.m_retain);

    refresh();

    connect(
        m_ui.m_topicLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(topicChanged(const QString&)));

    connect(
        m_ui.m_topicIdSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(topicIdChanged(int)));
    connect(
        m_ui.m_qosComboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(qosChanged(int)));
    connect(
        m_ui.m_reatainCheckBox, SIGNAL(stateChanged(int)),
        this, SLOT(retainChanged(int)));
}

PubSubWidget::~PubSubWidget() = default;

void PubSubWidget::topicChanged(const QString& val)
{
    m_info.m_topic = val;
    refresh();
}

void PubSubWidget::topicIdChanged(int val)
{
    m_info.m_topicId = static_cast<decltype(m_info.m_topicId)>(val);
}

void PubSubWidget::qosChanged(int val)
{
    m_info.m_qos = static_cast<decltype(m_info.m_qos)>(val);
}

void PubSubWidget::retainChanged(int val)
{
    m_info.m_retain = (val != Qt::Unchecked);
}

void PubSubWidget::refresh()
{
    if (!m_info.m_topic.isEmpty()) {
        m_ui.m_topicIdWidget->hide();
        if (m_info.m_topicId != 0) {
            m_oldTopicId = m_info.m_topicId;
            m_info.m_topicId = 0;
        }
    }
    else {
        m_ui.m_topicIdWidget->show();
        if (m_oldTopicId != 0) {
            m_info.m_topicId = m_oldTopicId;
            m_oldTopicId = 0;
            m_ui.m_topicIdSpinBox->setValue(m_info.m_topicId);
        }
    }
}

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn


