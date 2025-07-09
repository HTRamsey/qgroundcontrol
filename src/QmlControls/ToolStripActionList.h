/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtGui/QActionGroup>
#include <QtQml/QQmlListProperty>

#include "ToolStripAction.h"

Q_DECLARE_LOGGING_CATEGORY(ToolStripActionListLog)

class ToolStripActionList : public QActionGroup
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<ToolStripAction> model READ model NOTIFY modelChanged)

public:
    explicit ToolStripActionList(QObject *parent = nullptr);
    ~ToolStripActionList();

    QQmlListProperty<ToolStripAction> model();

signals:
    void modelChanged();

private:
    static qsizetype count(QQmlListProperty<ToolStripAction> *qmlListProperty);
    static ToolStripAction *at(QQmlListProperty<ToolStripAction> *qmlListProperty, qsizetype index);
    static void append(QQmlListProperty<ToolStripAction> *qmlListProperty, ToolStripAction *value);
    static void clear(QQmlListProperty<ToolStripAction> *qmlListProperty);
};
