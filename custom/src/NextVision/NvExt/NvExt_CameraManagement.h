#ifndef CAMERAMANAGEMENT_H
#define CAMERAMANAGEMENT_H

#include <QObject>
#include <QWidget>
#include <QVector>

#include "Joystick.h"
#include "QGCMapEngineManager.h"
#include "QGCMapEngine.h"
#include "QGeoMapReplyQGC.h"
#include "QGeoTileFetcherQGC.h"

class CameraManagement : public QObject
{
    Q_OBJECT

    /* enums */
    typedef enum
    {
        JoyBtnReleased,
        JoyBtnPressed
    }JoyBtnState;

    /* Mavlink Extension Arguments for Set System Mode Command */
    typedef enum
    {
        MavExtCmdArg_Stow = 0,
        MavExtCmdArg_Pilot,
        MavExtCmdArg_Hold,
        MavExtCmdArg_Observation,
        MavExtCmdArg_LocalPosition,
        MavExtCmdArg_GlobalPosition,
        MavExtCmdArg_GRR,
        MavExtCmdArg_TrackOnPosition,
        MavExtCmdArg_EPR,
        MavExtCmdArg_Nadir,
        MavExtCmdArg_NadirScan,
        MavExtCmdArg_2DScan,
    }MavlinkExtSetSystemModeArgs;

    /* Mavlink Extension Arguments for Enable/Disable */
    typedef enum
    {
        MavExtCmdArg_Disable = 0,
        MavExtCmdArg_Enable,
        MavExtCmdArg_Toggle
    }MavlinkExtEnDisArgs;

    /* Mavlink Extension Arguments for Set Sensor Command */
    typedef enum
    {
        MavExtCmdArg_DaySensor = 0,
        MavExtCmdArg_IrSensor,
        MavExtCmdArg_ToggleSensor
    }MavlinkExtSetSensorArgs;

    /* Mavlink Extension Arguments for Set Sharpness Command */
    typedef enum
    {
        MavExtCmdArg_NoSharpnessBoost = 0,
        MavExtCmdArg_LowSharpnessBoost,
        MavExtCmdArg_HighSharpnessBoost
    }MavlinkExtSetSharpnessArgs;

    /* Mavlink Extension Arguments for Set Gimbal Command */
    typedef enum
    {
        MavExtCmdArg_ZoomStop = 0,
        MavExtCmdArg_ZoomIn,
        MavExtCmdArg_ZoomOut,
        MavExtCmdArg_ZoomNoChange
    }MavlinkExtSetGimbalArgs;

    /* Mavlink Extension Arguments for Set Polarity Command */
    typedef enum
    {
        MavExtCmdArg_WhiteHot = 0,
        MavExtCmdArg_BlackHot,
        MavExtCmdArg_TogglePolarity,
    }MavlinkExtSetPolarityArgs;

    /* Mavlink Extension Arguments for Set IR Color Pallete Command */
    typedef enum
    {
        MavExtCmdArg_BW_P = 0,
        MavExtCmdArg_Color_P,
    }MavlinkExtSetIRPArgs;

    /* Mavlink Extension Arguments for Set IR Color Pallete Command */
    typedef enum
    {
        MavExtCmdArg_DetectorEnDis = 0,
        MavExtCmdArg_DetectorSelect,
        MavExtCmdArg_DetectorConfThres,
        MavExtCmdArg_DetectorFireThres,
    }MavlinkExtDetectionControlArgs;

private:

    void _terrainDataReceived(bool success, QList<double> heights);
    QGeoCoordinate _coord;
    double gndCrsAltitude = 0.0;

public:
    /* Mavlink Extension Commands */
    typedef enum
    {
        MavExtCmd_SetSystemMode = 0,
        MavExtCmd_TakeSnapShot,
        MavExtCmd_SetRecordState,
        MavExtCmd_SetSensor,
        MavExtCmd_SetFOV,
        MavExtCmd_SetSharpness,
        MavExtCmd_SetGimbal,
        MavExtCmd_DoBIT,
        MavExtCmd_SetIrPolarity,
        MavExtCmd_SetSingleYawMode,
        MavExtCmd_SetFollowMode,
        MavExtCmd_DoNUC,
        MavExtCmd_SetReportInterval,
        MavExtCmd_ClearRetractLock,
        MavExtCmd_SetSystemTime,
        MavExtCmd_SetIrColor,
        MavExtCmd_SetJoystickMode,
        MavExtCmd_SetGroundCrossingAlt,
        MavExtCmd_SetRollDerot,
        MavExtCmd_SetLaser,
        MavExtCmd_Reboot,
        MavExtCmd_ReconfVideo,
        MavExtCmd_SetTrackerIcon,
        MavExtCmd_SetZoom,
        MavExtCmd_FreezeVideo,
        MavExtCmd_SetColibExtMode,
        MavExtCmd_PilotView,
        MavExtCmd_RvtPostion,
        MavExtCmd_SnapShotInterval,        
        MavExtCmd_UpadteRemoteIP,
        MavExtCmd_UpdateVideoIP,
        MavExtCmd_ConfigurationCommand,
        MavExtCmd_ClearSDCard,
        MavExtCmd_SetRateMultiplier,
        MavExtCmd_SetIRGainLevel,
        MavExtCmd_SetVideoStreamTransmissionState,
        MavExtCmd_SetDayWhiteBalance,
        MavExtCmd_SetLaserMode,
        MavExtCmd_SetHoldCoordinateMode,
        MavExtCmd_NA,
        MavExtCmd_CameraTest,
        MavExtCmd_SetAIDetection,
        MavExtCmd_ResumeAIScan,
        MavExtCmd_SetFlyAbove,
        MavExtCmd_SetCameraStabilization = 50,
        MavExtCmd_UpdateStreamBitrate,
        MavExtCmd_DoIperfTest,
        MavExtCmd_SetGeoAvg,
        MavExtCmd_DetectionControl,
    }MavlinkExtCmd;

    explicit CameraManagement(QObject *parent = nullptr, MultiVehicleManager *multiVehicleManager = nullptr, JoystickManager *joystickManager = nullptr);
    void sendGimbalCommand(float cam_roll_yaw,float cam_pitch);
    void sendGimbalVirtualCommand(float cam_roll_yaw,float cam_pitch);

    JoyBtnState _camButtonFuncState[32];
    int         _camButtonFuncValue[32];

    void addTileToCache(QString tile_hash, QByteArray tile_data);
    void getAltAtCoord(float lat,float lon);

    Q_INVOKABLE void pointToCoordinate(float lat,float lon);
    Q_INVOKABLE void trackOnPosition(float posX,float posY);
    Q_INVOKABLE void setSysModeObsCommand(void);
    Q_INVOKABLE void setSysModeGrrCommand(void);
    Q_INVOKABLE void setSysModeEprCommand(void);
    Q_INVOKABLE void setSysModeHoldCommand(void);
    Q_INVOKABLE void setSysModePilotCommand(void);
    Q_INVOKABLE void setSysModeStowCommand(void);
    Q_INVOKABLE void setSysModeRetractCommand(void);
    Q_INVOKABLE void setSysModeRetractUnlockCommand(void);    
    Q_INVOKABLE void setSysZoomStopCommand(void);
    Q_INVOKABLE void setSysZoomInCommand(void);
    Q_INVOKABLE void setSysZoomOutCommand(void);
    Q_INVOKABLE void setSysSensorToggleCommand(void);
    Q_INVOKABLE void setSysSensorDayCommand(void);
    Q_INVOKABLE void setSysSensorIrCommand(void);
    Q_INVOKABLE void setSysIrPolarityToggleCommand(void);
    Q_INVOKABLE void setSysIrPolarityWHCommand(void);
    Q_INVOKABLE void setSysIrPolarityBHCommand(void);
    Q_INVOKABLE void setSysIrColorPCommand(void);
    Q_INVOKABLE void setSysIrBWPCommand(void);
    Q_INVOKABLE void setSysIrNUCCommand(void);
    Q_INVOKABLE void setSysRecToggleCommand(void);
    Q_INVOKABLE void setSysRecOnCommand(void);
    Q_INVOKABLE void setSysRecOffCommand(void);
    Q_INVOKABLE void setSysSnapshotCommand(void);
    Q_INVOKABLE void setSysAutoSnapshotCommand(int interval, int count, bool inf);
    Q_INVOKABLE void setSysFOVCommand(float fov_value);
    Q_INVOKABLE void setSysModeLocalPositionCommand(int pitch, int roll);
    Q_INVOKABLE void setSysModeGlobalPositionCommand(int elevation, int azimuth);
    Q_INVOKABLE void setSysSingleYawOnCommand(void);
    Q_INVOKABLE void setSysSingleYawOffCommand(void);
    Q_INVOKABLE void setSysFlyAboveOnCommand(void);
    Q_INVOKABLE void setSysFlyAboveOffCommand(void);
    Q_INVOKABLE void setSysFollowOnCommand(void);
    Q_INVOKABLE void setSysFollowOffCommand(void);
    Q_INVOKABLE void setSysNadirCommand(void);
    Q_INVOKABLE void setSysNadirScanCommand(void);    
    Q_INVOKABLE void setSysObjDetOnCommand(void);
    Q_INVOKABLE void setSysObjDetOffCommand(void);
    Q_INVOKABLE void setSysObjDetSetNetTypeCommand(int netType);
    Q_INVOKABLE void setSysObjDetSetConfThresCommand(float confThres);
    Q_INVOKABLE void setSysObjDetSetFireThresCommand(float fireThres);
    Q_INVOKABLE void setSysGeoAVGOnCommand(void);
    Q_INVOKABLE void setSysGeoAVGOffCommand(void);
    Q_INVOKABLE void setSysMode2DScanCommand(void);

protected:
    MultiVehicleManager*    _multiVehicleManager;
    Vehicle*                activeVehicle;
    JoystickManager*        _joystickManager;
    Joystick*               _activeCamJoystick;

private:
    void doCamAction(QString buttonAction, bool pressed, int buttonIndex);
    bool doBtnFuncToggle(bool pressed, int buttonIndex);
    MavlinkExtSetGimbalArgs  getZoomValue(unsigned char* buttons, QList<AssignedButtonAction*> button_actions);
    void sendMavCommandLong(MAV_CMD command, float param1, float param2, float param3,
                            float param4, float param5, float param6, float param7);
public slots:
    void _activeVehicleChanged(Vehicle* activeVehicle);
    void _activeCamJoystickChanged(Joystick* activeCamJoystick);
    void manualCamControl( float cam_roll_yaw, float cam_pitch, unsigned char* buttons);
};

#endif // CAMERAMANAGEMENT_H
