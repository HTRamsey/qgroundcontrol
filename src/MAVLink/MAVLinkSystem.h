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
#include <QtCore/QStringList>

Q_DECLARE_LOGGING_CATEGORY(MAVLinkSystemLog)

class QGCMAVLinkMessage;

class QGCMAVLinkSystem : public QObject
{
    Q_OBJECT

public:
    explicit QGCMAVLinkSystem(quint8 id, QObject *parent = nullptr);
    ~QGCMAVLinkSystem();

    quint8 id() const { return _id; }
    QmlObjectListModel *messages() const { return _messages; }
    QList<int> compIDs() const { return _compIDs; }
    QStringList compIDsStr() const { return _compIDsStr; }
    int selected() const { return _selected; }

    QGCMAVLinkMessage *findMessage(uint32_t id, uint8_t compId);
    int findMessage(const QGCMAVLinkMessage *message);
    void append(QGCMAVLinkMessage *message);

signals:
    void compIDsChanged();

private:
    void _checkCompID(const QGCMAVLinkMessage *message);
    void _resetSelection();

private:
    quint8 _id = 0;
    QSet<QGCMAVLinkMessage> _messages;
    QSet<int> _compIDs;
    QStringList _compIDsStr;
    int _selected = 0;
};
