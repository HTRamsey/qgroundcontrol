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
#include <QtQmlIntegration/QtQmlIntegration>

Q_DECLARE_LOGGING_CATEGORY(MAVLinkSystemLog)

class QGCMAVLinkMessage;
class QmlObjectListModel;

class QGCMAVLinkSystem : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_MOC_INCLUDE("QmlObjectListModel.h")
    Q_PROPERTY(quint8               sysId       READ sysId                          CONSTANT)
    Q_PROPERTY(QmlObjectListModel*  messages    READ messages                       CONSTANT)
    Q_PROPERTY(QList<quint8>        compIds     READ compIds                        NOTIFY compIdsChanged)
    Q_PROPERTY(QStringList          compIdsStr  READ compIdsStr                     NOTIFY compIdsChanged)
    Q_PROPERTY(quint32              selected    READ selected   WRITE setSelected   NOTIFY selectedChanged)

public:
    explicit QGCMAVLinkSystem(quint8 sysId, QObject *parent = nullptr);
    ~QGCMAVLinkSystem();

    quint8 sysId() const { return _sysId; }
    QmlObjectListModel *messages() const { return _messages; }
    QList<quint8> compIds() const { return _compIds; }
    QStringList compIdsStr() const { return _compIdsStr; }
    quint32 selected() const { return _selected; }

    void setSelected(quint32 msgId);
    QGCMAVLinkMessage *findMessage(uint8_t sysId, uint8_t compId);
    quint32 findMessage(const QGCMAVLinkMessage *message);
    void append(QGCMAVLinkMessage *message);
    QGCMAVLinkMessage *selectedMsg();

signals:
    void compIdsChanged();
    void selectedChanged();

private:
    void _checkCompID(const QGCMAVLinkMessage *message);
    void _resetSelection();

    quint8 _sysId = 0;
    QmlObjectListModel *_messages = nullptr; ///< List of QGCMAVLinkMessage
    QList<quint8> _compIds;
    QStringList _compIdsStr;
    quint32 _selected = 0;
};
