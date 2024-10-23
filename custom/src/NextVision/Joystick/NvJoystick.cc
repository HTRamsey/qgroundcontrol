#include "Joystick.h"
#include "QGCApplication.h"
#include "SettingsManager.h"

#define NV_HAT_INC 983.0f

void Joystick::initCamJoystick()
{
    for (int i = 0; i < _totalButtonCount; i++)
    {
        _buttonCamActionArray.append(nullptr);
    }
    _buildCamActionList();

    _camJoystickDZ      = qgcApp()->toolbox()->settingsManager()->nextVisionSettings()->camJoystickDZ()->rawValue().toInt();
    _camJoystickGain    = qgcApp()->toolbox()->settingsManager()->nextVisionSettings()->camJoystickGain()->rawValue().toInt();
}

void Joystick::loadCamButtons(QSettings *settings)
{
    for (int button = 0; button < _totalButtonCount; button++) 
    {
        QString a = settings->value(QString(_buttonCamActionNameKey).arg(button), QString()).toString();
        if(!a.isEmpty() && a != _buttonActionNone) 
        {
            if(_buttonCamActionArray[button]) _buttonCamActionArray[button]->deleteLater();
            AssignedButtonAction* ap = new AssignedButtonAction(this, a);
            ap->repeat = false;
            _buttonCamActionArray[button] = ap;
            _buttonCamActionArray[button]->buttonTime.start();
            qCDebug(JoystickLog) << "_loadSettings Cam button:action" << button << _buttonCamActionArray[button]->action << _buttonCamActionArray[button]->repeat;
        }
    }

    _camPitchRollAxle     = settings->value(_camPitchRollAxleKey, 0).toInt();
}

void Joystick::setCamButtons(QSettings *settings, int button)
{
    if(_buttonCamActionArray[button])
    {
        settings->setValue(QString(_buttonCamActionNameKey).arg(button),        _buttonCamActionArray[button]->action);
        settings->setValue(QString(_buttonCamActionRepeatKey).arg(button),      false);
    }
}

void Joystick::setButtonCamAction(int button, const QString& action)
{
    if (!_validButton(button)) return;
    qCWarning(JoystickLog) << "setButtonCamAction:" << button << action;
    QSettings settings;
    settings.beginGroup(_settingsGroup);
    settings.beginGroup(_name);
    if(action.isEmpty() || action == _buttonActionNone) 
    {
        if(_buttonCamActionArray[button]) 
        {
            _buttonCamActionArray[button]->deleteLater();
            _buttonCamActionArray[button] = nullptr;
            settings.remove(QString(_buttonCamActionNameKey).arg(button));
            settings.remove(QString(_buttonCamActionRepeatKey).arg(button));
        }
    } 
    else 
    {
        if(!_buttonCamActionArray[button]) 
        {
            _buttonCamActionArray[button] = new AssignedButtonAction(this, action);
        } 
        else 
        {
            _buttonCamActionArray[button]->action = action;
            _buttonCamActionArray[button]->repeat = false;
        }
        settings.setValue(QString(_buttonCamActionNameKey).arg(button),   _buttonCamActionArray[button]->action);
        settings.setValue(QString(_buttonCamActionRepeatKey).arg(button), false);
    }
    emit buttonCamActionsChanged();
}

QString Joystick::getButtonCamAction(int button)
{
    if (_validButton(button)) 
    {
        if(_buttonCamActionArray[button]) return _buttonCamActionArray[button]->action;
    }
    return QString(_buttonActionNone);
}

QStringList Joystick::buttonCamActions()
{
    QStringList list;
    for (int button = 0; button < _totalButtonCount; button++) 
    {
        list << getButtonCamAction(button);
    }
    return list;
}

void Joystick::setCamPitchRollAxle(int axle)
{
    _camPitchRollAxle = axle;
    _saveSettings();
    emit camPitchRollAxleChanged();
}

void Joystick::_buildCamActionList()
{
    if(_assignableCamButtonActions.count()) _assignableCamButtonActions.clearAndDeleteContents();
    _availableCamActionTitles.clear();

    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionNone));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionZoomIn));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionZoomOut));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionDayIR));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionWHBH));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionNUC));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionSnap));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionRec));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionSY));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionOBS));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionGRR));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionStow));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionPilot));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionRetract));
    _assignableCamButtonActions.append(new AssignableButtonAction(this, _buttonActionHoldCord));
    for(int i = 0; i < _assignableCamButtonActions.count(); i++) 
    {
        AssignableButtonAction* p = qobject_cast<AssignableButtonAction*>(_assignableCamButtonActions[i]);
        _availableCamActionTitles << p->action();
    }
    emit assignableCamActionsChanged();
}

void Joystick::setCamJoystickDZ(int DZ)
{
    _camJoystickDZ = DZ;
}

void Joystick::setCamJoystickGain(int gain)
{
    _camJoystickGain = gain;
}

void Joystick::camJoystickRun()
{
    bool            newButtonValue;
    int             rgButtonValueIndex;
    Calibration_t   roll_calib;
    Calibration_t   pitch_calib;
    
    float roll_yaw    = 0, pitch    = 0;
    float roll_yaw_DZ = 0, pitch_DZ = 0;
    static int camTimeDivider = 0;

    while (!_exitThread) 
    {
        _update();
        if ( _is_same_joystick || !_is_cam_joystick)
        {
            _handleButtons();
            _handleAxis();   
        }

        if ( (_is_same_joystick || _is_cam_joystick) && _activeVehicle->joystickCamEnabled() )
        {
            if (camTimeDivider++ >= 3)
            {
                if (_is_cam_joystick && !_is_same_joystick)
                {
                    for (int axisIndex = 0; axisIndex < _axisCount; axisIndex++) 
                    {
                        // Calibration code requires signal to be emitted even if value hasn't changed
                        _rgAxisValues[axisIndex] = _getAxis(axisIndex);
                    }

                    for (int buttonIndex = 0; buttonIndex < _buttonCount; buttonIndex++) 
                    {
                        newButtonValue = _getButton(buttonIndex);
                        if (newButtonValue && _rgButtonValues[buttonIndex] == BUTTON_UP)
                        {
                            _rgButtonValues[buttonIndex] = BUTTON_DOWN;
                        }
                        else if (!newButtonValue && _rgButtonValues[buttonIndex] != BUTTON_UP)
                        {
                            _rgButtonValues[buttonIndex] = BUTTON_UP;
                        }
                    }
                    //-- Update hat - append hat buttons to the end of the normal button list
                    for (int hatButtonIndex = 0; hatButtonIndex < 4; hatButtonIndex++) 
                    {
                        rgButtonValueIndex = hatButtonIndex + _buttonCount;
                        newButtonValue = _getHat(0, hatButtonIndex);
                        if (newButtonValue && _rgButtonValues[rgButtonValueIndex] == BUTTON_UP)
                        {
                            _rgButtonValues[rgButtonValueIndex] = BUTTON_DOWN;
                        }
                        else if (!newButtonValue && _rgButtonValues[rgButtonValueIndex] != BUTTON_UP)
                        {
                            _rgButtonValues[rgButtonValueIndex] = BUTTON_UP;
                        }
                    }
                }

                switch ( _camPitchRollAxle )
                {
                    case 0:
                    {
                        _handleCamHat(&roll_yaw,&pitch);
                        roll_calib.max  = 32768;
                        roll_calib.min  = -32768;
                        pitch_calib.max = 32768;
                        pitch_calib.min = -32768;
                        break;
                    }
                    case 1:
                    {
                        roll_yaw    = _getAxis(0);
                        pitch       = _getAxis(1);
                        roll_calib  = getCalibration(0);
                        pitch_calib = getCalibration(1);
                        break;
                    }
                    
                    case 2:
                    {
                        roll_yaw    = _getAxis(2);
                        pitch       = _getAxis(3);
                        roll_calib  = getCalibration(2);
                        pitch_calib = getCalibration(3);
                        break;
                    }
                    default:
                    {
                        QGC::SLEEP::msleep(20);
                        continue;
                    }
                }

                if ( _camPitchRollAxle != 0 )
                {
                    /* calculate Dead Zones */
                    roll_yaw_DZ     = (_camJoystickDZ * roll_calib.max   ) / 100;
                    pitch_DZ        = (_camJoystickDZ * pitch_calib.max  ) / 100;

                    /* check deadzones */
                    roll_yaw    =   ( roll_yaw  > -roll_yaw_DZ  && roll_yaw < roll_yaw_DZ )   ? 0 : roll_yaw;
                    pitch       =   ( pitch     > -pitch_DZ     && pitch    < pitch_DZ    )   ? 0 : pitch;
                }

                /* Apply Gain */
                roll_yaw    *= ((float)_camJoystickGain / 100.0f);
                pitch       *= ((float)_camJoystickGain / 100.0f);

                /* Apply limiting */
                roll_yaw    = (roll_yaw > roll_calib.max)   ? roll_calib.max    : roll_yaw;
                roll_yaw    = (roll_yaw < roll_calib.min)   ? roll_calib.min    : roll_yaw;
                pitch       = (pitch    > pitch_calib.max)  ? pitch_calib.max   : pitch;
                pitch       = (pitch    < pitch_calib.min)  ? pitch_calib.min   : pitch;

                emit manualControlCam   (roll_yaw,pitch,_rgButtonValues);
                emit manualControlCamQml(roll_yaw,pitch);

                camTimeDivider = 0;
            }
        }
    QGC::SLEEP::msleep(20);
    }
}

void Joystick::_handleCamHat(float *roll_yaw, float *pitch)
{
    bool up     = _getHat(0, 0);
    bool down   = _getHat(0, 1);
    bool left   = _getHat(0, 2);
    bool right  = _getHat(0, 3);

    /* calculate the Axle value */
    if ( up )
    {
        *pitch -= NV_HAT_INC;
    }
    else if ( down )
    {
        *pitch += NV_HAT_INC;
    }
    else
    {
        *pitch = 0;
    }

    if ( left )
    {
        *roll_yaw -= NV_HAT_INC;
    }
    else if ( right )
    {
        *roll_yaw += NV_HAT_INC;
    }
    else
    {
        *roll_yaw = 0;
    }

    /* rol_yaw / pitch limiting */
    *roll_yaw   = ( *roll_yaw   > 32768 ) ? 32768 : (( *roll_yaw    < -32768 ) ? -32768 : *roll_yaw);
    *pitch      = ( *pitch      > 32768 ) ? 32768 : (( *pitch       < -32768 ) ? -32768 : *pitch);
}
