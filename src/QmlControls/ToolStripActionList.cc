/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "ToolStripActionList.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(ToolStripActionListLog, "qgc.qmlcontrols.toolstripactionlist")

ToolStripActionList::ToolStripActionList(QObject *parent)
    : QActionGroup(parent)
{
    qCDebug(ToolStripActionListLog) << this;
}

ToolStripActionList::~ToolStripActionList()
{
    qCDebug(ToolStripActionListLog) << this;
}

QQmlListProperty<ToolStripAction> ToolStripActionList::model()
{
    return QQmlListProperty<ToolStripAction>(this, this,
             &ToolStripActionList::append,
             &ToolStripActionList::count,
             &ToolStripActionList::at,
             &ToolStripActionList::clear);
}

ToolStripAction *ToolStripActionList::at(QQmlListProperty<ToolStripAction> *qmlListProperty, qsizetype index)
{
    return reinterpret_cast<ToolStripAction*>(reinterpret_cast<ToolStripActionList*>(qmlListProperty->data)->actions()[index]);
}

qsizetype ToolStripActionList::count(QQmlListProperty<ToolStripAction> *qmlListProperty)
{
    return reinterpret_cast<ToolStripActionList*>(qmlListProperty->data)->actions().count();
}

void ToolStripActionList::append(QQmlListProperty<ToolStripAction> *qmlListProperty, ToolStripAction *value)
{
    reinterpret_cast<ToolStripActionList*>(qmlListProperty->data)->addAction(value);
}

void ToolStripActionList::clear(QQmlListProperty<ToolStripAction> *qmlListProperty)
{
    for (int i = 0; i < ToolStripActionList::count(qmlListProperty); i++) {
        reinterpret_cast<ToolStripActionList*>(qmlListProperty->data)->removeAction(ToolStripActionList::at(qmlListProperty, i));
    }
}
