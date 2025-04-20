/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "Vehicle.h"
#include "MAVLinkLib.h"

#include <QtCore/QTimer>

class Autotune : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool     autotuneInProgress  READ autotuneInProgress NOTIFY autotuneChanged)
    Q_PROPERTY(float    autotuneProgress    READ autotuneProgress   NOTIFY autotuneChanged)
    Q_PROPERTY(QString  autotuneStatus      READ autotuneStatus     NOTIFY autotuneChanged)

public:
    Autotune(Vehicle *vehicle);

    Q_INVOKABLE void autotuneRequest();

    bool autotuneInProgress() const { return _autotuneInProgress; }
    float autotuneProgress() const { return _autotuneProgress; }
    QString autotuneStatus() const { return _autotuneStatus; }

    static void ackHandler(void *resultHandlerData, int compId, const mavlink_command_ack_t &ack, Vehicle::MavCmdResultFailureCode_t failureCode);
    static void progressHandler(void *progressHandlerData, int compId, const mavlink_command_ack_t &ack);

signals:
    void autotuneChanged();

public slots:
    void sendMavlinkRequest();

private:
    void handleAckStatus(uint8_t ackProgress);
    void handleAckFailure();
    void handleAckError(uint8_t ackError);
    void startTimers();
    void stopTimers();

    Vehicle *_vehicle = nullptr;
    bool _autotuneInProgress = false;
    float _autotuneProgress = 0.0;
    QString _autotuneStatus = tr("Autotune: Not performed");
    bool _disarmMessageDisplayed = false;
    QTimer _pollTimer; // the frequency at which the polling should be performed
};
