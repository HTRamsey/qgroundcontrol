/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtPositioning/QGeoPositionInfo>

#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(RemoteIDManagerLog)

class RemoteIDSettings;
class Vehicle;

// Supporting Open Drone ID protocol
class RemoteIDManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool    available            READ available          NOTIFY availableChanged)             ///< true: the vehicle supports Mavlink Open Drone ID messages
    Q_PROPERTY(bool    armStatusGood        READ armStatusGood      NOTIFY armStatusGoodChanged)
    Q_PROPERTY(QString armStatusError       READ armStatusError     NOTIFY armStatusErrorChanged)
    Q_PROPERTY(bool    commsGood            READ commsGood          NOTIFY commsGoodChanged)
    Q_PROPERTY(bool    gcsGPSGood           READ gcsGPSGood         NOTIFY gcsGPSGoodChanged)
    Q_PROPERTY(bool    basicIDGood          READ basicIDGood        NOTIFY basicIDGoodChanged)
    Q_PROPERTY(bool    emergencyDeclared    READ emergencyDeclared  NOTIFY emergencyDeclaredChanged)
    Q_PROPERTY(bool    operatorIDGood       READ operatorIDGood     NOTIFY operatorIDGoodChanged)

public:
    RemoteIDManager(Vehicle *vehicle);

    Q_INVOKABLE void checkOperatorID(const QString &operatorID);
    Q_INVOKABLE void setOperatorID();
    Q_INVOKABLE void setEmergency(bool declare);

    bool available() const { return _available; }
    bool armStatusGood() const { return _armStatusGood; }
    QString armStatusError() const { return _armStatusError; }
    bool commsGood() const { return _commsGood; }
    bool gcsGPSGood() const { return _gcsGPSGood; }
    bool basicIDGood() const { return _basicIDGood; }
    bool emergencyDeclared() const { return _emergencyDeclared;}
    bool operatorIDGood() const { return _operatorIDGood; }

    void mavlinkMessageReceived(const mavlink_message_t &message);

    enum LocationTypes {
        TAKEOFF = 0,
        LiveGNSS = 1,
        FIXED = 2
    };

    enum Region {
        FAA,
        EU
    };

signals:
    void availableChanged();
    void armStatusGoodChanged();
    void armStatusErrorChanged();
    void commsGoodChanged();
    void gcsGPSGoodChanged();
    void basicIDGoodChanged();
    void emergencyDeclaredChanged();
    void operatorIDGoodChanged();

private slots:
    /// This slot will be called if we stop receiving heartbeats for more than RID_TIMEOUT seconds
    void _odidTimeout();
    /// Function that sends messages periodically
    void _sendMessages();
    void _updateLastGCSPositionInfo(const QGeoPositionInfo &update);
    void _checkGCSBasicID();

private:
    /// Parsing of the ARM_STATUS message comming from the RID device
    void _handleArmStatus(const mavlink_message_t &message);

    void _sendSelfIDMsg();
    /// We need to return the correct description for the self ID type we have selected
    const char *_getSelfIDDescription();

    void _sendOperatorID();

    void _sendSystem();
    // Returns seconds elapsed since 00:00:00 1/1/2019
    static uint32_t _timestamp2019();

    void _sendBasicID();

    static bool _isEUOperatorIDValid(const QString &operatorID);
    static QChar _calculateLuhnMod36(const QString &input);

    Vehicle *_vehicle = nullptr;
    RemoteIDSettings *_settings = nullptr;

    bool _available = false;
    bool _armStatusGood = false;
    QString _armStatusError;
    bool _commsGood = false;
    bool _gcsGPSGood = false;
    bool _basicIDGood = true;
    bool _GCSBasicIDValid = false;
    bool _operatorIDGood = false;

    bool _emergencyDeclared = false;
    QDateTime _lastGeoPositionTimeStamp;
    int _targetSystem = 0;
    int _targetComponent = 0;

    // After emergency cleared, this makes sure the non emergency selfID message makes it to the vehicle
    bool _enforceSendingSelfID = false;

    QTimer _odidTimeoutTimer;
    QTimer _sendMessagesTimer;

    QByteArray _descriptionBuffer;

    static const uint8_t *_id_or_mac_unknown;
};
