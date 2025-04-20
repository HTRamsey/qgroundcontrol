/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtQml/QQmlListProperty>

class ToolStripActionList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> model READ model NOTIFY modelChanged)

public:
    ToolStripActionList(QObject *parent = nullptr);

    QQmlListProperty<QObject> model();

signals:
    void modelChanged();

private:
    static void _append(QQmlListProperty<QObject> *qmlListProperty, QObject *value);
    static qsizetype _count(QQmlListProperty<QObject> *qmlListProperty);
    static QObject *_at(QQmlListProperty<QObject> *qmlListProperty, qsizetype index);
    static void _clear(QQmlListProperty<QObject> *qmlListProperty);

    QList<QObject*> _objectList;
};
