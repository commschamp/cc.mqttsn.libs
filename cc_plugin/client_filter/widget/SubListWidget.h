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


#pragma once

#include <functional>
#include <vector>

#include "comms/CompileControl.h"

CC_DISABLE_WARNINGS()
#include <QtWidgets/QWidget>
#include "ui_SubListElementWidget.h"
#include "ui_SubListWidget.h"
CC_ENABLE_WARNINGS()

#include "Filter.h"

namespace mqttsn
{

namespace cc_plugin
{

namespace client_filter
{

class SubListElementWidget : public QWidget
{
    Q_OBJECT
    typedef QWidget Base;
public:

    typedef Filter::PubSubInfo PubSubInfo;

    SubListElementWidget(PubSubInfo& info, QWidget* parentObj = nullptr);
    virtual ~SubListElementWidget();

signals:
    void sigRemoveRequested();

private:
    Ui::SubListElementWidget m_ui;
};

class SubListWidget : public QWidget
{
    Q_OBJECT
    typedef QWidget Base;
public:
    typedef Filter::PubSubInfo PubSubInfo;

    explicit SubListWidget(
        QList<PubSubInfo>& subs,
        QWidget* parentObj = nullptr);

    virtual ~SubListWidget();

private slots:
    void addNew();
    void remove();

private:
    void addElemWidget(PubSubInfo& sub);

    QList<PubSubInfo>& m_subs;
    Ui::SubListWidget m_ui;
    std::vector<SubListElementWidget*> m_elements;
};

}  // namespace client_filter

}  // namespace cc_plugin

}  // namespace mqttsn



