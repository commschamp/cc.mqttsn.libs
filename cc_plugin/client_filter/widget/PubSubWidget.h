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


#pragma once

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtWidgets/QWidget>
#include "ui_PubSubWidget.h"
CC_ENABLE_WARNINGS()

#include "Filter.h"

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

class PubSubWidget : public QWidget
{
    Q_OBJECT
    typedef QWidget Base;

public:
    typedef Filter::PubSubInfo PubSubInfo;

    PubSubWidget(PubSubInfo& info, bool sub);
    virtual ~PubSubWidget();

private slots:
    void topicChanged(const QString& val);
    void topicIdChanged(int val);
    void qosChanged(int val);
    void retainChanged(int val);

private:
    void refresh();

    PubSubInfo& m_info;
    Ui::PubSubWidget m_ui;
};

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn


