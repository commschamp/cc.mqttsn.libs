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


#include "FilterConfigWidget.h"

#include "PubSubWidget.h"
#include "SubListWidget.h"


namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

FilterConfigWidget::FilterConfigWidget(Filter& filter)
  : m_filter(filter)
{
    m_ui.setupUi(this);

    m_ui.m_layout->insertWidget(m_ui.m_layout->count() - 1, new PubSubWidget(m_filter.defaultPubInfo(), false));
    m_ui.m_layout->insertWidget(m_ui.m_layout->count() - 1, new SubListWidget(m_filter.subInfos()));

    static const int MaxDuration = std::numeric_limits<std::uint16_t>::max();
    m_ui.m_keepAliveSpinBox->setRange(0, MaxDuration);
    m_ui.m_retryDurationSpinBox->setRange(0, MaxDuration);

    m_ui.m_clientIdLineEdit->setText(m_filter.clientId().c_str());
    m_ui.m_keepAliveSpinBox->setValue(m_filter.keepAlivePeriod());
    m_ui.m_retryDurationSpinBox->setValue(m_filter.retryPeriod());
    m_ui.m_retryCountSpinBox->setValue(m_filter.retryCount());

    connect(
        m_ui.m_clientIdLineEdit, SIGNAL(textChanged(const QString&)),
        this, SLOT(clientIdChanged(const QString&)));
    connect(
        m_ui.m_keepAliveSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(keepAliveChanged(int)));
    connect(
        m_ui.m_retryDurationSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(retryDurationChanged(int)));
    connect(
        m_ui.m_retryCountSpinBox, SIGNAL(valueChanged(int)),
        this, SLOT(retryCountChanged(int)));
}

FilterConfigWidget::~FilterConfigWidget() = default;

void FilterConfigWidget::clientIdChanged(const QString& val)
{
    m_filter.clientId() = val.toStdString();
}

void FilterConfigWidget::keepAliveChanged(int val)
{
    m_filter.keepAlivePeriod() = val;
}

void FilterConfigWidget::retryDurationChanged(int val)
{
    m_filter.retryPeriod() = val;
}

void FilterConfigWidget::retryCountChanged(int val)
{
    m_filter.retryCount() = val;
}

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn


