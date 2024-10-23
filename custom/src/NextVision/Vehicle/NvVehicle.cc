#include "Vehicle.h"
#include "JoystickManager.h"
#include "QGCApplication.h"

void Vehicle::updateLineOfSight(QList<QGeoCoordinate> coordsList)
{
    /* Removing the old points */
    _losCoords.clear();
    foreach( const auto &item, coordsList )
        _losCoords << QVariant::fromValue(item);
    emit losCoordsChanged();
}

void Vehicle::updateSnapShotStatus(bool status)
{
    _snapShotStatus = status;
    emit snapShotStatusChanged(_snapShotStatus);
}

void Vehicle::_saveCamSettings(void)
{
    QSettings settings;

    settings.beginGroup(QString(_settingsGroup).arg(_id));

    // The camera joystick enabled setting should only be changed if a joystick is present
    // since the checkbox can only be clicked if one is present
    if (_toolbox->joystickManager()->joysticks().count()) {
        settings.setValue(_joystickCamEnabledSettingsKey, _joystickCamEnabled);
    }
}

void Vehicle::setJoystickCamEnabled(bool enabled)
{
    _joystickCamEnabled = enabled;
    _startJoystickCam(_joystickCamEnabled);
    _saveCamSettings();
    emit joystickCamEnabledChanged(_joystickCamEnabled);
}

void Vehicle::_startJoystickCam(bool start)
{
    Joystick* joystick = _joystickManager->activeCamJoystick();
    if (joystick) {
        if (start) {
            joystick->startPolling(this);
        } else {
            if ( joystick->_is_same_joystick && !_joystickEnabled )
                joystick->stopPolling();
        }
    }
}

void Vehicle::virtualCamJoystickValue(double roll, double pitch)
{
    qgcApp()->toolbox()->joystickManager()->cameraManagement()->sendGimbalVirtualCommand(roll,pitch);
}
