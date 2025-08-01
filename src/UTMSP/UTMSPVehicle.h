/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtQmlIntegration/QtQmlIntegration>

#include "UTMSPServiceController.h"
#include "UTMSPOperator.h"
#include "UTMSPFlightDetails.h"

// UTM-Adapter per vehicle management class.
class Dispatcher;
class Vehicle;

class UTMSPVehicle : public UTMSPServiceController
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_PROPERTY(QString                 vehicleSerialNumber            READ vehicleSerialNumber       NOTIFY vehicleSerialNumberChanged)
    Q_PROPERTY(bool                    vehicleActivation              READ vehicleActivation         NOTIFY vehicleActivationChanged)

public:
    UTMSPVehicle(std::shared_ptr<Dispatcher> dispatcher, Vehicle *vehicle, QObject *parent = nullptr);
    ~UTMSPVehicle();

    Q_INVOKABLE void loadTelemetryFlag(bool value);

    QString vehicleSerialNumber(void) const {return _vehicleSerialNumber;};
    bool vehicleActivation(void) const {return _vehicleActivation;};

signals:
    void vehicleSerialNumberChanged (void);
    void vehicleActivationChanged   (void);

protected slots:
    void triggerFlightAuthorization             (void);
    void triggerNetworkRemoteID                 (const mavlink_message_t &message) ;
    void triggerActivationStatusBar             (bool flag);

private:
    UTMSPFlightPlanManager        _utmspFlightPlanManager;
    UTMSPAircraft                 _utmspAircraft;
    UTMSPOperator                 _utmspOperator;
    UTMSPFlightDetails            _utmspFlightDetails;  //TODO:Will be used later
    std::string                   _aircraftSerialNumber;
    std::string                   _aircraftModel;
    std::string                   _aircraftType;
    std::string                   _aircraftClass;
    std::string                   _operatorID;
    std::string                   _operatorClass;
    std::shared_ptr<Dispatcher>   _dispatcher;
    bool                          _remoteIDFlag = false;
    bool                          _stopFlag = false;
    std::string                   _flightID = "";
    QString                       _vehicleSerialNumber = "";
    bool                          _vehicleActivation = false;
};
