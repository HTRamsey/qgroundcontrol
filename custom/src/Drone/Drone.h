#pragma once

#include "DroneData.h"
#include "MavlinkTasks.h"
#include <QtPositioning/QGeoCoordinate>
#include <QtQmlIntegration/QtQmlIntegration>

class Drone : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Drone(QObject* parent = nullptr);
    ~Drone();

    bool readyForRtl();
    Q_INVOKABLE void rtl();
    bool readyForLand();
    Q_INVOKABLE void land();
    bool readyForBrake();
    Q_INVOKABLE void brake();
    bool readyForTakeoff();
    Q_INVOKABLE void takeoff(float altitude);
    bool readyForChangeAltitude();
    Q_INVOKABLE void changeAltitude(float altitude);
    bool readyForReposition();
    Q_INVOKABLE void reposition(const QGeoCoordinate& gotoCoord);
    bool readyForChangeHeading();
    Q_INVOKABLE void changeHeading(const QGeoCoordinate& headingCoord);
    bool readyForAdjustLights();
    Q_INVOKABLE void adjustLights(uint8_t intensity);
    bool readyForToggleBeacon();
    Q_INVOKABLE void toggleBeacon();
    bool readyForToggleRemoteID();
    Q_INVOKABLE void toggleRemoteID();
    bool readyForToggleNavigationLights();
    Q_INVOKABLE void toggleNavigationLights();
    bool readyForSetAntiCollisionLightState();
    Q_INVOKABLE void setAntiCollisionLightState(DroneData::AntiCollisionLightState);
    bool readyForReboot();
    Q_INVOKABLE void reboot();
    bool readyForFollow();
    Q_INVOKABLE void follow();
    bool readyForSetFollowOffsets();
    Q_INVOKABLE void setFollowOffsets(const QGeoCoordinate& targetCoord);
    bool readyForSetFollowLandOffsets();
    Q_INVOKABLE void setFollowLandOffsets(const QGeoCoordinate& target);
    bool readyForSetFollowAltitude();
    Q_INVOKABLE void setFollowAltitude(float altitude);
    bool readyForSetFollowTarget();
    Q_INVOKABLE void setFollowTarget(uint8_t target);
    bool readyForFollowLand();
    Q_INVOKABLE void followLand();
    void setParam(const QString &param_id, uint8_t param_type, QVariant value);
    void getParam(const QString &param_id, uint8_t param_type, int16_t param_index = -1);

signals:
    void sendTask(MavlinkTask*);

private:
    QList<int8_t> m_lightTargets{};
    uint32_t m_count = 0;

    bool _readyForArm();
    void _arm(bool arm);
    bool _readyForGuided();
    void _guided();
    bool _isValidAltitude(float altitude);
};
