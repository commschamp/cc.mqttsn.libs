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

#include "widget/PubSubWidget.h"

#include <iostream>

namespace cc = comms_champion;

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

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
                return new PubSubWidget(m_filter->defaultPubInfo(), false);

            });
}

Plugin::~Plugin() = default;

void Plugin::getCurrentConfigImpl(QVariantMap& config)
{
    static_cast<void>(config);
    std::cout << __FUNCTION__ << std::endl;
}

void Plugin::reconfigureImpl(const QVariantMap& config)
{
    static_cast<void>(config);
    std::cout << __FUNCTION__ << std::endl;
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

