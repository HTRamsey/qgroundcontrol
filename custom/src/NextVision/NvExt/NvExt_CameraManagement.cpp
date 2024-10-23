#include "NvExt_CameraManagement.h"
#include "QGCApplication.h"
#include "TerrainQuery.h"
#include "SettingsManager.h"
#include "JoystickManager.h"

Q_GLOBAL_STATIC(TerrainTileManager, _terrainTileManager)

CameraManagement::CameraManagement(QObject *parent, MultiVehicleManager *multiVehicleManager, JoystickManager *joystickManager)
    : QObject               (parent)
    , _multiVehicleManager  (nullptr)
    , activeVehicle         (nullptr)
    , _joystickManager      (nullptr)
{
    this->_multiVehicleManager  = multiVehicleManager;
    this->_joystickManager      = joystickManager;
    activeVehicle               = _multiVehicleManager->activeVehicle();
    connect(_multiVehicleManager,   &MultiVehicleManager::activeVehicleChanged, this, &CameraManagement::_activeVehicleChanged);
    connect(this->_joystickManager, &JoystickManager::activeCamJoystickChanged, this, &CameraManagement::_activeCamJoystickChanged);

    QGCMapEngine    *map_engine = getQGCMapEngine();
    QGCCacheWorker  *worker = map_engine->getWorker();
    connect(worker, &QGCCacheWorker::tileLoaded, this, &CameraManagement::addTileToCache);
}

void CameraManagement::sendMavCommandLong( MAV_CMD command,     float param1 = 0.0f,   float param2 = 0.0f,   float param3 = 0.0f,
                                           float param4 = 0.0f, float param5 = 0.0f,   float param6 = 0.0f,   float param7 = 0.0f)
{
    if(!activeVehicle) return;

    WeakLinkInterfacePtr weakLink = activeVehicle->vehicleLinkManager()->primaryLink();
    if (weakLink.expired()) return;

    activeVehicle->sendMavCommand( activeVehicle->defaultComponentId(),
                                   command,
                                   true,
                                   param1, param2, param3, param4, param5, param6, param7);
}

void CameraManagement::_activeVehicleChanged(Vehicle* activeVehicle)
{
    this->activeVehicle = activeVehicle;
    QGCLoadElevationTileSetsTask* taskSave;
    if(activeVehicle)
    {
        float time = QDateTime::currentSecsSinceEpoch();
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSystemTime,
                            time);

        taskSave = new QGCLoadElevationTileSetsTask();
        getQGCMapEngine()->addTask(taskSave);
    }
}

void CameraManagement::_activeCamJoystickChanged(Joystick* activeCamJoystick)
{
    if(activeCamJoystick)
    {
        connect(activeCamJoystick, &Joystick::manualControlCam, this, &CameraManagement::manualCamControl);
        this->_activeCamJoystick = activeCamJoystick;

        for ( int i = 0; i < 32;i++ )
        {
            _camButtonFuncState[i] = JoyBtnReleased;
            _camButtonFuncValue[i] = 0;
        }
    }
    else
    {
        if ( this->_activeCamJoystick ) disconnect(this->_activeCamJoystick, &Joystick::manualControlCam, this, &CameraManagement::manualCamControl);
        this->_activeCamJoystick = activeCamJoystick;
    }
}

void CameraManagement::manualCamControl(float cam_roll_yaw, float cam_pitch,unsigned char* buttons)
{
    static int prev_zoom_value = -1;
    QList<AssignedButtonAction*> button_actions;
    if (!activeVehicle) return;

    WeakLinkInterfacePtr weakLink = activeVehicle->vehicleLinkManager()->primaryLink();
    if (weakLink.expired()) return;

    if (!_activeCamJoystick) return;

    /* read the current joystick configuration */
    button_actions = _activeCamJoystick->_buttonCamActionArray;

    for (int buttonIndex=0; buttonIndex<_activeCamJoystick->totalButtonCount(); buttonIndex++)
    {
        // bool button_value = (buttons & (1 << buttonIndex)) ? true :false;
        AssignedButtonAction *button_action = button_actions.at(buttonIndex);
        if ( !button_action ) continue;
        doCamAction(button_action->action,buttons[buttonIndex],buttonIndex);
        // doCamAction(_camButtonActionsMap[buttonIndex],button_value,buttonIndex);
    }

    int zoomValue = getZoomValue(buttons,button_actions);
    if ( prev_zoom_value != zoomValue )
    {
        prev_zoom_value = zoomValue;

        switch ( zoomValue )
        {
            case MavExtCmdArg_ZoomIn:
            {
                setSysZoomInCommand();
                break;
            }
            case MavExtCmdArg_ZoomOut:
            {
                setSysZoomOutCommand();
                break;
            }
            case MavExtCmdArg_ZoomStop:
            {
                setSysZoomStopCommand();
                break;
            }
        }
    }

    if ( qgcApp()->toolbox()->settingsManager()->nextVisionSettings()->virtualCamJoystick()->rawValue().toBool() == false )
    {
        sendGimbalCommand(cam_roll_yaw/ ( -32768),cam_pitch/ ( -32768));
    }
}


bool CameraManagement::doBtnFuncToggle(bool pressed, int buttonIndex)
{
    switch ( _camButtonFuncState[buttonIndex] )
    {
        case JoyBtnReleased:
        {
            if ( pressed )
            {
                _camButtonFuncState[buttonIndex] = JoyBtnPressed;
            }
            break;
        }
        case JoyBtnPressed:
        {
            if ( !pressed )
            {
                _camButtonFuncValue[buttonIndex] ^= 1;
                _camButtonFuncState[buttonIndex] = JoyBtnReleased;
                return true;
            }
            break;
        }
    }
    return false;
}

void CameraManagement::doCamAction(QString buttonAction, bool pressed, int buttonIndex)
{
    if ( buttonAction.isEmpty() ) return;
    if (!doBtnFuncToggle(pressed,buttonIndex)) return;
    if (!activeVehicle) return;
    if (!activeVehicle->joystickCamEnabled()) return;

    if (buttonAction == "Day / IR")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSensor,
                            _camButtonFuncValue[buttonIndex]);
    }
    else if (buttonAction == "White Hot / Black Hot")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetIrPolarity,
                            _camButtonFuncValue[buttonIndex]);
    }
    else if (buttonAction == "Image Capture")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_TakeSnapShot);
    }
    else if (buttonAction == "Single Yaw")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSingleYawMode,
                            _camButtonFuncValue[buttonIndex]);
    }
    else if (buttonAction == "GRR")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSystemMode,
                            MavExtCmdArg_GRR);
    }
    else if (buttonAction == "NUC")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_DoNUC);
    }
    else if (buttonAction == "Stow")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSystemMode,
                            MavExtCmdArg_Stow);
    }
    else if (buttonAction == "Pilot")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSystemMode,
                            MavExtCmdArg_Pilot);
    }
    else if (buttonAction == "Retract")
    {
        sendMavCommandLong( MAV_CMD_DO_MOUNT_CONTROL,
                            _camButtonFuncValue[buttonIndex]);
    }
    else if (buttonAction == "Hold Coordinate")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSystemMode,
                            MavExtCmdArg_Hold);
    }
    else if (buttonAction == "Observation")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetSystemMode,
                            MavExtCmdArg_Observation);
    }
    else if (buttonAction == "Record")
    {
        sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                            MavExtCmd_SetRecordState,
                            _camButtonFuncValue[buttonIndex]);
    }
}

/* Returning the zoom value according to the buttons pressed */
CameraManagement::MavlinkExtSetGimbalArgs CameraManagement::getZoomValue(unsigned char* buttons,QList<AssignedButtonAction*> button_actions)
{
    if ( !_activeCamJoystick ) return MavExtCmdArg_ZoomStop;

    int zoomInVal = 0;
    int zoomOutVal = 0;

    /* call the button functions for each button */
    for (int buttonIndex=0; buttonIndex<_activeCamJoystick->totalButtonCount(); buttonIndex++)
    {
        // bool button_value = (buttons & (1 << buttonIndex)) ? true :false;
        AssignedButtonAction *button_action = button_actions.at(buttonIndex);
        if ( !button_action ) continue;
        if((button_action->action == "Zoom In") && (buttons[buttonIndex] != 0))
        {
            zoomInVal = 1;
        }
        else if((button_action->action == "Zoom Out") && (buttons[buttonIndex] != 0))
        {
            zoomOutVal = 1;
        }
    }

    if( (zoomInVal == 0 && zoomOutVal == 0) || (zoomInVal == 1 && zoomOutVal == 1) ) return MavExtCmdArg_ZoomStop;
    else if(zoomInVal == 1) return MavExtCmdArg_ZoomIn;
    else return MavExtCmdArg_ZoomOut;
        
}

/* Sending gimbal Command Messages */
void CameraManagement::sendGimbalCommand(float cam_roll_yaw,float cam_pitch)
{
    if(!activeVehicle) return;

    WeakLinkInterfacePtr weakLink = activeVehicle->vehicleLinkManager()->primaryLink();
    if (weakLink.expired()) return;
    SharedLinkInterfacePtr sharedLink = weakLink.lock();
    
    mavlink_message_t message;
    if ( activeVehicle->joystickCamEnabled() )
    {
        mavlink_msg_command_long_pack_chan(1,
                                           0,
                                           sharedLink->mavlinkChannel(),
                                           &message,
                                           1,
                                           0,
                                           MAV_CMD_DO_DIGICAM_CONTROL,
                                           0,
                                           MavExtCmd_SetGimbal,
                                           cam_roll_yaw,
                                           cam_pitch,
                                           MavExtCmdArg_ZoomNoChange,
                                           (float)this->gndCrsAltitude,
                                           0,
                                           0);
        activeVehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), message);
    }
}

void CameraManagement::sendGimbalVirtualCommand(float cam_roll_yaw,float cam_pitch)
{
    if(!activeVehicle) return;

    WeakLinkInterfacePtr weakLink = activeVehicle->vehicleLinkManager()->primaryLink();
    if (weakLink.expired()) return;
    SharedLinkInterfacePtr sharedLink = weakLink.lock();

    mavlink_message_t message;
    if ( qgcApp()->toolbox()->settingsManager()->nextVisionSettings()->virtualCamJoystick()->rawValue().toBool() == true )
    {
        mavlink_msg_command_long_pack_chan(1,
                                           0,
                                           sharedLink->mavlinkChannel(),
                                           &message,
                                           1,
                                           0,
                                           MAV_CMD_DO_DIGICAM_CONTROL,
                                           0,
                                           MavExtCmd_SetGimbal,
                                           cam_roll_yaw,
                                           cam_pitch,
                                           MavExtCmdArg_ZoomNoChange,
                                           (float)this->gndCrsAltitude,
                                           0,
                                           0);

        activeVehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), message);
    }
}

void CameraManagement::addTileToCache(QString tile_hash, QByteArray tile_data)
{
    _terrainTileManager->addTileToCache(tile_data,tile_hash);
}

void CameraManagement::getAltAtCoord(float lat,float lon)
{
    double terrainAltitude;
    QGeoCoordinate coord;

    coord.setLatitude(lat);
    coord.setLongitude(lon);

    if( _terrainTileManager->requestCachedData(coord,terrainAltitude) )
    {
        this->gndCrsAltitude = terrainAltitude;     /* save the value, will be transmitted to the TRIP2 in the next Gimbal or GndAlt message */
    }

    if ( !activeVehicle ) return;

    /* when the virtual joystick is disabled set gnd crs alt here */
    mavlink_message_t message;
    WeakLinkInterfacePtr weakLink;
    SharedLinkInterfacePtr sharedLink;
    if ( qgcApp()->toolbox()->settingsManager()->nextVisionSettings()->virtualCamJoystick()->rawValue().toBool() == false ||
         activeVehicle->joystickCamEnabled() )
    {
        if(!activeVehicle) return;

        weakLink = activeVehicle->vehicleLinkManager()->primaryLink();
        if (weakLink.expired()) return;
        sharedLink = weakLink.lock();

        /* when the virtual joystick is disabled send the ground altitude from here instead */
        mavlink_msg_command_long_pack_chan(1,
                                           0,
                                           sharedLink->mavlinkChannel(),
                                           &message,
                                           1,
                                           0,
                                           MAV_CMD_DO_DIGICAM_CONTROL,
                                           0,
                                           MavExtCmd_SetGroundCrossingAlt,
                                           (float)this->gndCrsAltitude,
                                           0,0,0,0,0);
        activeVehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), message);
    }
}

void CameraManagement::pointToCoordinate(float lat,float lon)
{
    double terrainAltitude;

    _coord.setLatitude(lat);
    _coord.setLongitude(lon);

    TerrainAtCoordinateQuery* terrain;
    QList<QGeoCoordinate> rgCoord;
    if ( _terrainTileManager->requestCachedData(_coord,terrainAltitude) == false )
    {
        terrain = new TerrainAtCoordinateQuery(true);
        connect(terrain, &TerrainAtCoordinateQuery::terrainDataReceived, this, &CameraManagement::_terrainDataReceived);
        rgCoord.append(_coord);
        terrain->requestData(rgCoord);        
    }
    else
    {
        sendMavCommandLong( MAV_CMD_DO_SET_ROI_LOCATION,
                            0,0,0,0,
                            _coord.latitude(),
                            _coord.longitude(),
                            terrainAltitude);
        //qDebug() << "00 PTC On lat= " << (int)(_coord.latitude() * 10000000.0) << " lon = " << (int)(_coord.longitude() * 10000000.0 )<< " alt = " << terrainAltitude;
    }
}

void CameraManagement::_terrainDataReceived(bool success, QList<double> heights)
{
    double _terrainAltitude = success ? heights[0] : 0;
    sendMavCommandLong( MAV_CMD_DO_SET_ROI_LOCATION,
                        0,0,0,0,
                        _coord.latitude(),
                        _coord.longitude(),
                        _terrainAltitude);
    //qDebug() << "11 PTC On lat= " << (int)(_coord.latitude() * 10000000.0) << " lon = " << (int)(_coord.longitude() * 10000000.0 )<< " alt = " << _terrainAltitude;
    //sender()->deleteLater();
}

void CameraManagement::trackOnPosition(float posX,float posY)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_TrackOnPosition,
                        posX,
                        posY);
}

void CameraManagement::setSysModeObsCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_Observation);
}

void CameraManagement::setSysModeGrrCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_GRR);
}

void CameraManagement::setSysModeEprCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_EPR);
}

void CameraManagement::setSysModeHoldCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_Hold);
}

void CameraManagement::setSysModePilotCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_Pilot);
}

void CameraManagement::setSysModeStowCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_Stow);
}

void CameraManagement::setSysModeRetractCommand()
{
    sendMavCommandLong(MAV_CMD_DO_MOUNT_CONTROL);
}

void CameraManagement::setSysModeRetractUnlockCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_ClearRetractLock);
}

void CameraManagement::setSysZoomStopCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetZoom);
}

void CameraManagement::setSysZoomInCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetZoom,
                        1);
}

void CameraManagement::setSysZoomOutCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetZoom,
                        2);
}

void CameraManagement::setSysSensorToggleCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSensor,
                        MavExtCmdArg_ToggleSensor);
}

void CameraManagement::setSysSensorDayCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSensor,
                        MavExtCmdArg_DaySensor);
}

void CameraManagement::setSysSensorIrCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSensor,
                        MavExtCmdArg_IrSensor);
}

void CameraManagement::setSysIrPolarityToggleCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetIrPolarity,
                        MavExtCmdArg_TogglePolarity);
}

void CameraManagement::setSysIrPolarityWHCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetIrPolarity,
                        MavExtCmdArg_WhiteHot);
}

void CameraManagement::setSysIrPolarityBHCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetIrPolarity,
                        MavExtCmdArg_BlackHot);
}

void CameraManagement::setSysIrColorPCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetIrColor,
                        MavExtCmdArg_Color_P);
}

void CameraManagement::setSysIrBWPCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetIrColor,
                        MavExtCmdArg_BW_P);
}

void CameraManagement::setSysIrNUCCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_DoNUC);
}

void CameraManagement::setSysRecToggleCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetRecordState,
                        MavExtCmdArg_Toggle);
}

void CameraManagement::setSysRecOnCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetRecordState,
                        MavExtCmdArg_Enable);
}

void CameraManagement::setSysRecOffCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetRecordState,
                        MavExtCmdArg_Disable);
}

void CameraManagement::setSysSnapshotCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_TakeSnapShot);
}

void CameraManagement::setSysAutoSnapshotCommand(int interval, int count, bool inf)
{
    /* inf override */
    if ( inf )
    {
        count = -1;
    }

    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SnapShotInterval,
                        (float)interval,
                        (float)count);
}

void CameraManagement::setSysFOVCommand(float fov_value)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetFOV,
                        fov_value);
}

void CameraManagement::setSysModeLocalPositionCommand(int pitch, int roll)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_LocalPosition,
                        pitch,
                        roll);
}

void CameraManagement::setSysModeGlobalPositionCommand( int elevation, int azimuth )
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_GlobalPosition,
                        elevation,
                        azimuth);
}

void CameraManagement::setSysSingleYawOnCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSingleYawMode,
                        1);
}

void CameraManagement::setSysSingleYawOffCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSingleYawMode);
}
void CameraManagement::setSysFlyAboveOnCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetFlyAbove,
                        1);
}

void CameraManagement::setSysFlyAboveOffCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetFlyAbove);
}

void CameraManagement::setSysFollowOnCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetFollowMode,
                        1);
}

void CameraManagement::setSysFollowOffCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetFollowMode);
}

void CameraManagement::setSysNadirCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_Nadir);
}

void CameraManagement::setSysNadirScanCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_NadirScan);
}

void CameraManagement::setSysObjDetOnCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_DetectionControl,
                        MavExtCmdArg_DetectorEnDis,
                        1);
}
void CameraManagement::setSysObjDetOffCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_DetectionControl,
                        MavExtCmdArg_DetectorEnDis);
}

void CameraManagement::setSysObjDetSetNetTypeCommand(int netType)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_DetectionControl,
                        MavExtCmdArg_DetectorSelect,
                        (float)netType);
}

void CameraManagement::setSysObjDetSetConfThresCommand(float confThres)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_DetectionControl,
                        MavExtCmdArg_DetectorConfThres,
                        confThres);
}

void CameraManagement::setSysObjDetSetFireThresCommand(float fireThres)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_DetectionControl,
                        MavExtCmdArg_DetectorFireThres,
                        fireThres);
}

void CameraManagement::setSysGeoAVGOnCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetGeoAvg,
                        1);
}

void CameraManagement::setSysGeoAVGOffCommand(void)
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetGeoAvg);
}

void CameraManagement::setSysMode2DScanCommand()
{
    sendMavCommandLong( MAV_CMD_DO_DIGICAM_CONTROL,
                        MavExtCmd_SetSystemMode,
                        MavExtCmdArg_2DScan);
}
