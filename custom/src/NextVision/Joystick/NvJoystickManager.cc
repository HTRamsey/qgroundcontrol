#include "JoystickManager.h"

void JoystickManager::setCameraManagement(CameraManagement *camManagement)
{
    _cameraManagement = camManagement;
    emit activeCameraManagementChanged();
}

void JoystickManager::setCamJoystickActive(QSettings *settings, QString name)
{
    QString cam_joy_name = settings->value(_settingsKeyActiveCamJoystick).toString();
    cam_joy_name = cam_joy_name.isEmpty() ? _name2JoystickMap.first()->name() : cam_joy_name;

    if ( name == cam_joy_name )
    {
        Joystick* joystick = _name2JoystickMap.value(name, _name2JoystickMap.first());
        joystick->_is_same_joystick = true;
    }

    /* Must Be First !!!!! */
    setActiveCamJoystick(_name2JoystickMap.value(cam_joy_name, _name2JoystickMap.first()));
    settings->setValue(_settingsKeyActiveCamJoystick, _activeCamJoystick->name());
}

QString JoystickManager::activeCamJoystickName(void)
{
    return _activeCamJoystick ? _activeCamJoystick->name() : QString();
}

void JoystickManager::setActiveCamJoystickName(const QString& name)
{
    if (!_name2JoystickMap.contains(name)) 
    {
        qCWarning(JoystickManagerLog) << "Set cam active not in map" << name;
        return;
    }

    setActiveCamJoystick(_name2JoystickMap[name]);
}

void JoystickManager::setActiveCamJoystick(Joystick* joystick)
{
    QSettings settings;

    if (joystick != nullptr && !_name2JoystickMap.contains(joystick->name())) 
    {
        qCWarning(JoystickManagerLog) << "Set cam active not in map" << joystick->name();
        return;
    }

    if (_activeCamJoystick == joystick) return;

    if (_activeCamJoystick) _activeCamJoystick->stopPolling();

    _activeCamJoystick = joystick;

    if (_activeCamJoystick != nullptr) 
    {
        _activeCamJoystick->_is_cam_joystick = true;
        qCDebug(JoystickManagerLog) << "Set cam active:" << _activeCamJoystick->name();

        settings.beginGroup(_settingsGroup);
        settings.setValue(_settingsKeyActiveCamJoystick, _activeCamJoystick->name());
    }

    emit activeCamJoystickChanged(_activeCamJoystick);
    emit activeCamJoystickNameChanged(_activeCamJoystick?_activeCamJoystick->name():"");
}