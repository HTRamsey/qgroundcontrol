#pragma once

#include "MAVLinkLib.h"
#include <QtGui/QVector3D>
#include <QtCore/QList>
#include <QtPositioning/QGeoCoordinate>
#include <QtQmlIntegration/QtQmlIntegration>

class NextVision : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    typedef enum
    {
        MavExtCmdArg_Disable = 0,
        MavExtCmdArg_Enable,
        MavExtCmdArg_Toggle
    }MavlinkExtEnDisArgs;

    typedef enum
    {
        MavExtCmdArg_VideoCh0 = 0,
        MavExtCmdArg_VideoCh1,
    }MavlinkExtVideoChannelArgs;

    typedef enum
    {
        MavExtCmdArg_Stow = 0,
        MavExtCmdArg_Pilot,
        MavExtCmdArg_Hold,
        MavExtCmdArg_Observation,
        MavExtCmdArg_LocalPosition,
        MavExtCmdArg_GlobalPosition,
        MavExtCmdArg_GRR,
        MavExtCmdArg_Tracking,
        MavExtCmdArg_EPR,
        MavExtCmdArg_Nadir,
        MavExtCmdArg_NadirScan,
        MavExtCmdArg_2DScan,
        MavExtCmdArg_PTC,
        MavExtCmdArg_UnstabilizedPosition
    }MavlinkExtSetSystemModeArgs;

    typedef enum
    {
        MavExtCmdArg_TrackOnPosition = 0,
        MavExtCmdArg_TrackEnable,
        MavExtCmdArg_Track,
        MavExtCmdArg_Retrack,
        MavExtCmdArg_TrackDisable,
    }MavlinkExtSetSystemModeTrackingArgs;

    typedef enum
    {
        MavExtCmdArg_DaySensor = 0,
        MavExtCmdArg_IRSensor,
        MavExtCmdArg_ToggleSensor
    }MavlinkExtSetSensorArgs;

    typedef enum
    {
        MavExtCmdArg_NoSharpnessBoost = 0,
        MavExtCmdArg_LowSharpnessBoost,
        MavExtCmdArg_HighSharpnessBoost
    }MavlinkExtSetSharpnessArgs;

    typedef enum
    {
        MavExtCmdArg_GimbalZoomStop = 0,
        MavExtCmdArg_GimbalZoomIn,
        MavExtCmdArg_GimbalZoomOut,
        MavExtCmdArg_GimbalZoomNoChange
    }MavlinkExtSetGimbalArgs;

    typedef enum
    {
        MavExtCmdArg_WhiteHot = 0,
        MavExtCmdArg_BlackHot,
        MavExtCmdArg_TogglePolarity,
    }MavlinkExtSetPolarityArgs;

    typedef enum
    {
        MavExtCmdArg_System_Report = 0,
        MavExtCmdArg_LOS_Report,
        MavExtCmdArg_GroundCrossingReport,
        MavExtCmdArg_SDCardReport = 6,
        MavExtCmdArg_VideoReport
    }MavlinkExtSetReportFrequencyArgs;

    typedef enum
    {
        MavExtCmdArg_Grey = 0,
        MavExtCmdArg_Color
    }MavlinkExtSetIRColorArgs;

    typedef enum
    {
        MavExtCmdArg_Normal = 0,
        MavExtCmdArg_DeRotated,
    }MavlinkExtSetJoystickModeArgs;

    typedef enum
    {
        MavExtCmdArg_ZoomStop = 0,
        MavExtCmdArg_ZoomIn,
        MavExtCmdArg_ZoomOut,
    }MavlinkExtSetZoomArgs;

    typedef enum
    {
        MavExtCmdArg_UpdateParam = 0,
        MavExtCmdArg_SaveParams,
        MavExtCmdArg_Reboot,
        MavExtCmdArg_GetParam
    }MavlinkExtConfigArgs;

    typedef enum
    {
        MavExtConfigParam_VideoDestCh0          = 0,
        MavExtConfigParam_VideoDestPort         = 1,
        MavExtConfigParam_EnableBandWidthLimit  = 2,
        MavExtConfigParam_BandwithLimit         = 3,
        MavExtConfigParam_StreamBitrate         = 4,
        MavExtConfigParam_RecordingBitrate      = 5,
        MavExtConfigParam_EthIpAddr             = 6,
        MavExtConfigParam_EthSubnetMask         = 7,
        MavExtConfigParam_EthGatewayIp          = 8,
        MavExtConfigParam_MTU                   = 9,
        MavExtConfigParam_EnableRollDerot       = 10,
        MavExtConfigParam_AutoRecordEnable      = 11,
        MavExtConfigParam_AutoRecordINSTimeout  = 12,
        MavExtConfigParam_EnableAES             = 13,
        MavExtConfigParam_AES128Key             = 14,
        MavExtConfigParam_RemoteIP              = 15,
        MavExtConfigParam_RemotePort            = 16,
        MavExtConfigParam_LocalPort             = 17,
        MavExtConfigParam_RunFlightPlan         = 18,
        MavExtConfigParam_ForceRollToHorizon    = 19,
        MavExtConfigParam_OSDMode               = 20,
        MavExtConfigParam_ShowGndCrsInfo        = 21,
        MavExtConfigParam_ShowGPSInfo           = 22,
        MavExtConfigParam_ShowFlightMode        = 23,
        MavExtConfigParam_ShowAirSpeed          = 24,
        MavExtConfigParam_OSDMiniMap            = 25,
        MavExtConfigParam_VGAResizingThreshold  = 26,
        MavExtConfigParam_VGAOutputMode         = 27,
        MavExtConfigParam_ReportSystemID        = 28,
        MavExtConfigParam_ReportComponentID     = 29,
        MavExtConfigParam_AckSystemID           = 30,
        MavExtConfigParam_AckComponentID        = 31,
        MavExtConfigParam_EnableMavSequenceNum  = 32,
        MavExtConfigParam_StreamGOPSize         = 33,
        MavExtConfigParam_StreamQuantizationVal = 34,
        MavExtConfigParam_EnableObjDet          = 35,
        MavExtConfigParam_ObjDetType            = 36,
        MavExtConfigParam_ObjDetAutoTrackEnable = 37,
        MavExtConfigParam_ObjDetAutoZoomOnTarget= 38,
        MavExtConfigParam_ObjDetLoiterAPOnDetect= 39,
        MavExtConfigParam_EnableALPR            = 40,
        MavExtConfigParam_ALPRCountryProfile    = 41,
        MavExtConfigParam_AILicenseStatus       = 42,
        MavExtConfigParam_End
    }MavlinkExtConfigParams;

    static constexpr int clearSDCard_validation = 12345678;

    typedef enum
    {
        MavExtCmdArg_x1 = 0,
        MavExtCmdArg_x2,
        MavExtCmdArg_x4,
        MavExtCmdArg_x8
    }MavlinkExtSetRateMultiplierArgs;

    typedef enum
    {
        MavExtCmdArg_LevelDecrement = 0,
        MavExtCmdArg_LevelIncrement,
        MavExtCmdArg_GainDecrement,
        MavExtCmdArg_GainIncrement,
        MavExtCmdArg_GainLevelReset
    }MavlinkExtSetIRGainLevelArgs;

    typedef enum
    {
        MavExtCmdArg_Auto = 0,
        MavExtCmdArg_Manual,
    }MavlinkExtSetDayWhiteBalanceArgs;

    typedef enum
    {
        MavExtCmdArg_AlwaysOn = 0,
        MavExtCmdArg_2Hz,
        MavExtCmdArg_6Hz,
        MavExtCmdArg_30Hz
    }MavlinkExtSetLaserModeArgs;

    typedef enum
    {
        MavExtCmdArg_Update = 0,
        MavExtCmdArg_Starting,
    }MavlinkExtSetHoldCoordinateModeArgs;

    typedef enum
    {
        MavExtCmdArg_DetectorEnDis = 0,
        MavExtCmdArg_DetectorSelect,
        MavExtCmdArg_DetectorThreshold,
        MavExtCmdArg_DetectorResumeScan,
        MavExtCmdArg_DetectorFireThres,
        MavExtCmdArg_EnableCarCounting,
        MavExtCmdArg_ResetCarCounting
    }MavlinkExtDetectionControlArgs;

    typedef enum
    {
        MavExtCmdArg_HumanAndVehicle = 0,
        MavExtCmdArg_FireAndSmoke,
        MavExtCmdArg_HumanOverboard,
        MavExtCmdArg_MarineVessel,
    }MavlinkExtDetectionControlDetectorSelectArgs;

    typedef enum
    {
        MavExtCmdArg_RefineLocation = 0,
        MavExtCmdArg_SetReferencePoint,
    }MavlinkExtGeoMapControlArgs;

    typedef enum
    {
        MavExtCmdArg_StreamMode = 0,
        MavExtCmdArg_PipMode,
        MavExtCmdArg_SBSMode,
    }MavlinkExtStreamControlArgs;

    typedef enum
    {
        MavExtCmdArg_Disabled = 0,
        MavExtCmdArg_Day,
        MavExtCmdArg_IR,
        MavExtCmdArg_Fusion,
        MavExtCmdArg_PIP,
        MavExtCmdArg_SideBySide
    }MavlinkExtStreamControlStreamModeArgs;

    typedef enum
    {
        MavExtCmdArg_DayLarge = 0,
        MavExtCmdArg_IRLarge,
    }MavlinkExtStreamControlPIPModeArgs;

    typedef enum
    {
        MavExtCmdArg_DayLeftIRRight = 0,
        MavExtCmdArg_DayRightIRLeft,
    }MavlinkExtStreamControlSBSModeArgs;

    typedef enum
    {
        MavExtCmdArg_EnableVMD = 0,
        MavExtCmdArg_PolygonColorSelect,
        MavExtCmdArg_EnableReports
    }MavlinkExtVMDControlArgs;

    typedef enum
    {
        MavExtCmdArg_PolygonColorWhite = 0,
        MavExtCmdArg_PolygonColorBlack,
        MavExtCmdArg_PolygonColorRed,
        MavExtCmdArg_PolygonColorGreen,
        MavExtCmdArg_PolygonColorBlue,
        MavExtCmdArg_PolygonColorYellow,
        MavExtCmdArg_PolygonColorOrange,
        MavExtCmdArg_PolygonColorAzure,
        MavExtCmdArg_PolygonColorMagenta,
        MavExtCmdArg_PolygonColorCyan
    }MavlinkExtVMDControlPolygonColorArgs;

    typedef enum
    {
        MavExtCmdArg_EnableSecondaryControl = 0,
        MavExtCmdArg_EnableSecondaryReports,
    }MavlinkExtMultipleGCSControlArgs;

    typedef enum
    {
        MavExtCmdArg_SetPrimaryTracker = 0,
        MavExtCmdArg_SetActiveTracker,
        MavExtCmdArg_SetTrackerROI
    }MavlinkExtTrackerControlArgs;

    typedef enum
    {
        MavExtCmdArg_Tracker1 = 0,
        MavExtCmdArg_Tracker2,
        MavExtCmdArg_Tracker3,
        MavExtCmdArg_Tracker4
    }MavlinkExtTrackerControlTrackerArgs;

    typedef enum
    {
        MavExtCmdArg_ROI64 = 0,
        MavExtCmdArg_ROI96,
        MavExtCmdArg_ROI128,
        MavExtCmdArg_ROI192,
        MavExtCmdArg_ROI256
    }MavlinkExtTrackerControlROIArgs;

    typedef enum
    {
        MavExtCmdArg_Low = 0,
        MavExtCmdArg_Med,
        MavExtCmdArg_High
    }MavlinkExtIRNoiseReductionArgs;

    typedef enum
    {
        MavExtCmd_SetSystemMode                     = 0,
        MavExtCmd_TakeSnapShot                      = 1,
        MavExtCmd_SetRecordState                    = 2,
        MavExtCmd_SetSensor                         = 3,
        MavExtCmd_SetFOV                            = 4,
        MavExtCmd_SetSharpness                      = 5,
        MavExtCmd_SetGimbal                         = 6,
        MavExtCmd_DoBIT                             = 7,
        MavExtCmd_SetIRPolarity                     = 8,
        MavExtCmd_SetSingleYawMode                  = 9,
        MavExtCmd_SetFollowMode                     = 10,
        MavExtCmd_DoNUC                             = 11,
        MavExtCmd_SetReportFrequency                = 12,
        MavExtCmd_ClearRetractLock                  = 13,
        MavExtCmd_SetSystemTime                     = 14,
        MavExtCmd_SetIRColor                        = 15,
        MavExtCmd_SetJoystickMode                   = 16,
        MavExtCmd_SetGroundCrossingAlt              = 17,
        MavExtCmd_SetRollDerot                      = 18,
        MavExtCmd_SetLaser                          = 19,
        MavExtCmd_Reboot                            = 20,
        MavExtCmd_ReconfVideo                       = 21,
        MavExtCmd_SetTrackingIcon                   = 22,
        MavExtCmd_SetZoom                           = 23,
        MavExtCmd_FreezeVideo                       = 24,
        MavExtCmd_PilotView                         = 26,
        MavExtCmd_RvtPostion                        = 27,
        MavExtCmd_SnapShotInterval                  = 28,
        MavExtCmd_UpdateRemoteIP                    = 29,
        MavExtCmd_UpdateVideoIP                     = 30,
        MavExtCmd_ConfigCmd                         = 31,
        MavExtCmd_ClearSDCard                       = 32,
        MavExtCmd_SetRateMultiplier                 = 33,
        MavExtCmd_SetIRGainLevel                    = 34,
        MavExtCmd_SetVideoStreamTransmissionState   = 35,
        MavExtCmd_SetDayWhiteBalance                = 36,
        MavExtCmd_SetLaserMode                      = 37,
        MavExtCmd_SetHoldCoordinateMode             = 38,
        MavExtCmd_UpdateEnableCrosshair             = 39,
        MavExtCmd_SetFlyAbove                       = 43,
        MavExtCmd_SetCameraStabilization            = 50,
        MavExtCmd_UpdateStreamBitrate               = 51,
        MavExtCmd_DoIperfTest                       = 52,
        MavExtCmd_SetGeoAvg                         = 53,
        MavExtCmd_DetectionControl                  = 54,
        MavExtCmd_ARMarkerControl                   = 55,
        MavExtCmd_GeoMapControl                     = 56,
        MavExtCmd_StreamControl                     = 57,
        MavExtCmd_VMDControl                        = 58,
        MavExtCmd_MultipleGCSControl                = 59,
        MavExtCmd_UpdateINSCalibrationSet           = 60,
        MavExtCmd_TrackerControl                    = 61,
        MavExtCmd_PLRControl                        = 62,
        MavExtCmd_SetIRNoiseReductionLevel          = 63,
    }MavlinkExtCmd;

public:
    explicit NextVision(QObject *parent = nullptr);
    ~NextVision();

    void sendGimbalCmd(float cam_roll_yaw, float cam_pitch)
    {
//        setGimbalCmd(cam_roll_yaw, cam_pitch, MavExtCmdArg_GimbalZoomNoChange, _data.gndCrsAlt()->rawValue().toFloat());
    }

    Q_INVOKABLE void sendGimbalVirtualCmd(float cam_roll_yaw, float cam_pitch);

    void setStartCameraParked(bool enabled)
    {
        _sendMavCmdLongMsg( MAV_CMD_DO_MOUNT_CONFIGURE, enabled ? MAV_MOUNT_MODE_RETRACT : MAV_MOUNT_MODE_NEUTRAL );
    }

    void setStartCameraParkedOn()
    {
        setStartCameraParked(true);
    }

    void setStartCameraParkedOff()
    {
        setStartCameraParked(false);
    }

    /* 0 */
    void setSysModeCmd(MavlinkExtSetSystemModeArgs mode, float param3=0.0f, float param4=0.0f, float param5=0.0f, float param6=0.0f, float param7=0.0f)
    {
        _sendDigicamCmd( MavExtCmd_SetSystemMode, mode, param3, param4, param5, param6, param7);
    }

    Q_INVOKABLE void setSysModeStow()
    {
        setSysModeCmd(MavExtCmdArg_Stow);
    }

    Q_INVOKABLE void setSysModePilot()
    {
        setSysModeCmd(MavExtCmdArg_Pilot);
    }

    Q_INVOKABLE void setSysModeHold()
    {
        setSysModeCmd(MavExtCmdArg_Hold);
    }

    Q_INVOKABLE void setSysModeObs()
    {
        setSysModeCmd(MavExtCmdArg_Observation);
    }

    Q_INVOKABLE void setSysModeLocalPos(float pitch, float roll)
    {
        setSysModeCmd(MavExtCmdArg_LocalPosition, pitch, roll);
    }

    Q_INVOKABLE void setSysModeGlobalPos(float pitch, float roll)
    {
        setSysModeCmd(MavExtCmdArg_GlobalPosition, pitch, roll);
    }

    Q_INVOKABLE void setSysModeGrr()
    {
        setSysModeCmd(MavExtCmdArg_GRR);
    }

    void setSysModeGrr2()
    {
        _sendMavCmdLongMsg( MAV_CMD_DO_MOUNT_CONTROL, MAV_MOUNT_MODE_NEUTRAL);
    }

    Q_INVOKABLE void setSysModePark()
    {
        _sendMavCmdLongMsg( MAV_CMD_DO_MOUNT_CONTROL, MAV_MOUNT_MODE_RETRACT);
    }

    void setSysLastModeBeforePTC()
    {
        _sendMavCmdLongMsg( MAV_CMD_DO_SET_ROI_NONE );
    }

    void setSysModeTracking(float posX, float posY, MavlinkExtSetSystemModeTrackingArgs mode, MavlinkExtVideoChannelArgs ch)
    {
        setSysModeCmd(MavExtCmdArg_Tracking, posX, posY, mode, ch);
    }

    Q_INVOKABLE void trackOnPosition(float posX, float posY)
    {
        trackOnPositionCh0(posX, posY);
    }

    void trackOnPositionCh0(float posX, float posY)
    {
        setSysModeTracking(posX, posY, MavExtCmdArg_TrackOnPosition, MavExtCmdArg_VideoCh0);
    }

    void trackOnPositionCh1(float posX, float posY)
    {
        setSysModeTracking(posX, posY, MavExtCmdArg_TrackOnPosition, MavExtCmdArg_VideoCh1);
    }

    Q_INVOKABLE void setSysModeEpr()
    {
        setSysModeCmd(MavExtCmdArg_EPR);
    }

    Q_INVOKABLE void setSysModeNadir()
    {
        setSysModeCmd(MavExtCmdArg_Nadir);
    }

    Q_INVOKABLE void setSysModeNadirScan()
    {
        setSysModeCmd(MavExtCmdArg_NadirScan);
    }

    Q_INVOKABLE void setSysMode2DScan()
    {
        setSysModeCmd(MavExtCmdArg_2DScan);
    }

    void setSysModePointToCoord(QGeoCoordinate coord)
    {
        setSysModeCmd(MavExtCmdArg_PTC, coord.latitude(), coord.longitude(), coord.altitude());
    }

    void setSysModePointToCoord2(QGeoCoordinate coord)
    {
        _sendMavCmdLongMsg( MAV_CMD_DO_SET_ROI_LOCATION, 0,0,0,0, coord.latitude(), coord.longitude(), coord.altitude());
    }

    void setSysModePointToCoord3(QGeoCoordinate coord)
    {
        _sendMavCmdLongMsg( MAV_CMD_DO_SET_ROI, 0,0,0,0, coord.latitude(), coord.longitude(), coord.altitude());
    }

    Q_INVOKABLE void pointToCoordinate(QGeoCoordinate coord);

    Q_INVOKABLE void setSysModeUnstabilized(float pitch, float roll)
    {
        setSysModeCmd(MavExtCmdArg_UnstabilizedPosition, pitch, roll);
    }

    /* 1 */
    void takeSnapshotCmd(MavlinkExtVideoChannelArgs ch)
    {
        _sendDigicamCmd(MavExtCmd_TakeSnapShot, ch);
    }

    Q_INVOKABLE void takeSnapshotCh0()
    {
        takeSnapshotCmd(MavExtCmdArg_VideoCh0);
    }

    Q_INVOKABLE void takeSnapshotCh1()
    {
        takeSnapshotCmd(MavExtCmdArg_VideoCh1);
    }

    /* 2 */
    void setRecStateCmd(MavlinkExtEnDisArgs enabled, MavlinkExtVideoChannelArgs ch)
    {
        _sendDigicamCmd(MavExtCmd_SetRecordState, enabled, ch);
    }

    Q_INVOKABLE void setRecStateOnCh0()
    {
        setRecStateCmd(MavExtCmdArg_Enable,MavExtCmdArg_VideoCh0);
    }

    Q_INVOKABLE void setRecStateOffCh0()
    {
        setRecStateCmd(MavExtCmdArg_Disable,MavExtCmdArg_VideoCh0);
    }

    Q_INVOKABLE void setRecStateOnCh1()
    {
        setRecStateCmd(MavExtCmdArg_Enable,MavExtCmdArg_VideoCh1);
    }

    Q_INVOKABLE void setRecStateOffCh1()
    {
        setRecStateCmd(MavExtCmdArg_Disable,MavExtCmdArg_VideoCh1);
    }

    /* 3 */
    void setSensorCmd(MavlinkExtSetSensorArgs sensor)
    {
        _sendDigicamCmd(MavExtCmd_SetSensor, sensor);
    }

    Q_INVOKABLE void setSensorDay()
    {
        setSensorCmd(MavExtCmdArg_DaySensor);
    }

    Q_INVOKABLE void setSensorIR()
    {
        setSensorCmd(MavExtCmdArg_IRSensor);
    }

    /* 4 */
    Q_INVOKABLE void setFOVCmd(float degrees)
    {
        _sendDigicamCmd(MavExtCmd_SetFOV,degrees);
    }

    /* 5 */
    Q_INVOKABLE void setSharpnessCmd(MavlinkExtSetSensorArgs sensor, MavlinkExtSetSharpnessArgs mode)
    {
        _sendDigicamCmd(MavExtCmd_SetSharpness, sensor, mode);
    }

    /* 6 */
    void setGimbalCmd(float roll_rate, float pitch_rate, MavlinkExtSetGimbalArgs zoom_state, float alt);

    /* 7 */
    Q_INVOKABLE void doBitCmd()
    {
        _sendDigicamCmd(MavExtCmd_DoBIT);
    }

    /* 8 */
    void setIRPolarityCmd(MavlinkExtSetPolarityArgs polarity)
    {
        _sendDigicamCmd(MavExtCmd_SetIRPolarity, polarity);
    }

    Q_INVOKABLE void setIRPolarityBlack()
    {
        setIRPolarityCmd(MavExtCmdArg_BlackHot);
    }

    Q_INVOKABLE void setIRPolarityWhite()
    {
        setIRPolarityCmd(MavExtCmdArg_WhiteHot);
    }

    /* 9 */
    void setSingleYawCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_SetSingleYawMode, enabled);
    }

    Q_INVOKABLE void setSingleYawOn()
    {
        setSingleYawCmd(MavExtCmdArg_Enable);
    }

    Q_INVOKABLE void setSingleYawOff()
    {
        setSingleYawCmd(MavExtCmdArg_Disable);
    }

    /* 10 */
    void setFollowCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_SetFollowMode, enabled);
    }

    Q_INVOKABLE void setFollowOn()
    {
        setFollowCmd(MavExtCmdArg_Enable);
    }

    Q_INVOKABLE void setFollowOff()
    {
        setFollowCmd(MavExtCmdArg_Disable);
    }

    /* 11 */
    Q_INVOKABLE void doNUCCmd()
    {
        _sendDigicamCmd(MavExtCmd_DoNUC);
    }

    /* 12 */
    void setReportFrequencyCmd(MavlinkExtSetReportFrequencyArgs report, uint8_t frequency)
    {
        if(frequency <= 25)
            _sendDigicamCmd(MavExtCmd_SetReportFrequency, report, frequency);
    }

    void setReportFrequencySystem(uint8_t freq)
    {
        setReportFrequencyCmd(MavExtCmdArg_System_Report, freq);
    }

    void setReportFrequencyLOS(uint8_t freq)
    {
        setReportFrequencyCmd(MavExtCmdArg_LOS_Report, freq);
    }

    void setReportFrequencyGC(uint8_t freq)
    {
        setReportFrequencyCmd(MavExtCmdArg_GroundCrossingReport, freq);
    }

    void setReportFrequencySDCard(uint8_t freq)
    {
        setReportFrequencyCmd(MavExtCmdArg_SDCardReport, freq);
    }

    void setReportFrequencyVideo(uint8_t freq)
    {
        setReportFrequencyCmd(MavExtCmdArg_VideoReport, freq);
    }

    /* 13 */
    Q_INVOKABLE void clearRetractLockCmd()
    {
        _sendDigicamCmd(MavExtCmd_ClearRetractLock);
    }

    /* 14 */
    void setSystemTimeCmd()
    {
        _sendDigicamCmd(MavExtCmd_SetSystemTime, static_cast<float>(QDateTime::currentSecsSinceEpoch()));
    }

    /* 15 */
    void setIRColorCmd(MavlinkExtSetIRColorArgs color)
    {
        _sendDigicamCmd(MavExtCmd_SetIRColor,color);
    }

    Q_INVOKABLE void setIRColorGrey()
    {
        setIRColorCmd(MavExtCmdArg_Grey);
    }

    Q_INVOKABLE void setIRColor1()
    {
        setIRColorCmd(MavExtCmdArg_Color);
    }

    /* 16 */
    void setJoystickMode(MavlinkExtSetJoystickModeArgs mode)
    {
        _sendDigicamCmd(MavExtCmd_SetJoystickMode, mode);
    }

    Q_INVOKABLE void setJoystickModeNormal()
    {
        setJoystickMode(MavExtCmdArg_Normal);
    }

    Q_INVOKABLE void setJoystickModeDeRotated()
    {
        setJoystickMode(MavExtCmdArg_DeRotated);
    }

    /* 17 */
    void setGroundCrossingAlt(float alt)
    {
        _sendDigicamCmd(MavExtCmd_SetGroundCrossingAlt, alt);
    }

    /* 18 */
    void setRollDerotationCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_SetRollDerot, enabled);
    }

    Q_INVOKABLE void setRollDerotationOn()
    {
        setRollDerotationCmd(MavExtCmdArg_Enable);
    }

    Q_INVOKABLE void setRollDerotationOff()
    {
        setRollDerotationCmd(MavExtCmdArg_Disable);
    }

    /* 19 */
    void setLaserCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_SetRollDerot, enabled);
    }

    Q_INVOKABLE void setLaserOn()
    {
        setLaserCmd(MavExtCmdArg_Enable);
    }

    Q_INVOKABLE void setLaserOff()
    {
        setLaserCmd(MavExtCmdArg_Disable);
    }

    /* 20 */
    Q_INVOKABLE void rebootSystemCmd()
    {
        _sendDigicamCmd(MavExtCmd_Reboot);
    }

    /* 21 */
    Q_INVOKABLE void reconfigureBitrateCmd(float bitrate, MavlinkExtVideoChannelArgs ch)
    {
        if(bitrate >= 250 && bitrate <= 12000)
            _sendDigicamCmd(MavExtCmd_ReconfVideo,bitrate,ch);
    }

    /* 22 */
    Q_INVOKABLE void setTrackingIconCmd(uint8_t index)
    {
        if(index <= 6)
            _sendDigicamCmd(MavExtCmd_SetTrackingIcon, index);
    }

    /* 23 */
    void setZoomCmd(MavlinkExtSetZoomArgs zoom)
    {
        _sendDigicamCmd(MavExtCmd_SetZoom, zoom);
    }

    Q_INVOKABLE void zoomIn()
    {
        setZoomCmd(MavExtCmdArg_ZoomIn);
    }

    Q_INVOKABLE void zoomOut()
    {
        setZoomCmd(MavExtCmdArg_ZoomOut);
    }

    Q_INVOKABLE void zoomStop()
    {
        setZoomCmd(MavExtCmdArg_ZoomStop);
    }

    /* 24 */
    void freezeVideoCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_FreezeVideo,enabled);
    }

    Q_INVOKABLE void freezeVideoOff()
    {
        freezeVideoCmd(MavExtCmdArg_Disable);
    }

    Q_INVOKABLE void freezeVideoOn()
    {
        freezeVideoCmd(MavExtCmdArg_Enable);
    }

    /* 26 */
    Q_INVOKABLE void pilotViewCmd(uint8_t degrees)
    {
        if(degrees <= 90)
            _sendDigicamCmd(MavExtCmd_PilotView,degrees);
    }

    /* 27 */
    Q_INVOKABLE void rvtPositionCmd(float lat, float lon)
    {
        _sendDigicamCmd(MavExtCmd_RvtPostion, lat, lon);
    }

    /* 28 */
    Q_INVOKABLE void snapshotIntervalCmd(uint32_t interval, int count, MavlinkExtVideoChannelArgs ch)
    {
        if(((interval >= 500 && interval <= 10000) || interval == 0) && (count >= -1 && count <= 10000))
            _sendDigicamCmd(MavExtCmd_SnapShotInterval,interval,count,ch);
    }

    /* 29 */
    Q_INVOKABLE void updateRemoteIP(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4)
    {
        _sendDigicamCmd(MavExtCmd_UpdateRemoteIP,octet1,octet2,octet3,octet4);
    }

    /* 30 */
    Q_INVOKABLE void updateVideoIP(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4, MavlinkExtVideoChannelArgs ch)
    {
        _sendDigicamCmd(MavExtCmd_UpdateVideoIP,octet1,octet2,octet3,octet4,ch);
    }

    /* 31 */
    void configCmd(MavlinkExtConfigArgs operation, MavlinkExtConfigParams param=MavExtConfigParam_End, float opt1=0.0f, float opt2=0.0f, float opt3=0.0f, float opt4=0.0f);

    void getParam(MavlinkExtConfigParams param)
    {
        configCmd(MavExtCmdArg_GetParam, param);
    }

    /* 32 */
    Q_INVOKABLE void clearSDCardCmd()
    {
        _sendDigicamCmd(MavExtCmd_ClearSDCard,clearSDCard_validation);
    }

    /* 33 */
    Q_INVOKABLE void setRateMultiplierCmd(MavlinkExtSetRateMultiplierArgs multiplier)
    {
        _sendDigicamCmd(MavExtCmd_SetRateMultiplier, multiplier);
    }

    /* 34 */
    Q_INVOKABLE void setIRGainLevelCmd(MavlinkExtSetIRGainLevelArgs cmd)
    {
        _sendDigicamCmd(MavExtCmd_SetIRGainLevel, cmd);
    }

    /* 35 */
    void setVideoTransmissionState(MavlinkExtEnDisArgs enabled, MavlinkExtVideoChannelArgs ch)
    {
        _sendDigicamCmd(MavExtCmd_SetVideoStreamTransmissionState,enabled,ch);
    }

    Q_INVOKABLE void setVideoTransmissionCh0On()
    {
        _sendDigicamCmd(MavExtCmdArg_Enable,MavExtCmdArg_VideoCh0);
    }

    Q_INVOKABLE void setVideoTransmissionCh0Off()
    {
        _sendDigicamCmd(MavExtCmdArg_Disable,MavExtCmdArg_VideoCh0);
    }

    Q_INVOKABLE void setVideoTransmissionCh1On()
    {
        _sendDigicamCmd(MavExtCmdArg_Enable,MavExtCmdArg_VideoCh1);
    }

    Q_INVOKABLE void setVideoTransmissionCh1Off()
    {
        _sendDigicamCmd(MavExtCmdArg_Disable,MavExtCmdArg_VideoCh1);
    }

    /* 36 */
    Q_INVOKABLE void setDayWhiteBalanceCmd(MavlinkExtSetDayWhiteBalanceArgs mode, uint16_t temperature)
    {
        _sendDigicamCmd(MavExtCmd_SetDayWhiteBalance,mode,temperature);
    }

    /* 37 */
    Q_INVOKABLE void setLaserModeCmd(MavlinkExtSetLaserModeArgs mode)
    {
        _sendDigicamCmd(MavExtCmd_SetLaserMode, mode);
    }

    /* 38 */
    Q_INVOKABLE void setHoldCoordinateModeCmd(MavlinkExtSetHoldCoordinateModeArgs mode)
    {
        _sendDigicamCmd(MavExtCmd_SetHoldCoordinateMode, mode);
    }

    /* 39 */
    Q_INVOKABLE void updateEnableCrosshairCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_UpdateEnableCrosshair, enabled);
    }

    /* 43 */
    void setFlyAboveCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_SetFlyAbove, enabled);
    }

    Q_INVOKABLE void setFlyAboveOn()
    {
        setFlyAboveCmd(MavExtCmdArg_Enable);
    }

    Q_INVOKABLE void setFlyAboveOff()
    {
        setFlyAboveCmd(MavExtCmdArg_Disable);
    }

    /* 50 */
    void setCameraStabilizationCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_SetCameraStabilization, enabled);
    }

    Q_INVOKABLE void setCameraStabilizationOn()
    {
        setCameraStabilizationCmd(MavExtCmdArg_Enable);
    }

    Q_INVOKABLE void setCameraStabilizationOff()
    {
        setCameraStabilizationCmd(MavExtCmdArg_Disable);
    }

    /* 51 */
    Q_INVOKABLE void updateStreamBitrateCmd(float bitrate, MavlinkExtVideoChannelArgs ch)
    {
        if(bitrate >= 250 && bitrate <= 12000)
            _sendDigicamCmd(MavExtCmd_SnapShotInterval,bitrate,ch);
    }

    /* 52 */
    Q_INVOKABLE void doIperfTestCmd()
    {
        _sendDigicamCmd(MavExtCmd_DoIperfTest);
    }

    /* 53 */
    void setGeoAvgCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_SetGeoAvg, enabled);
    }

    Q_INVOKABLE void setGeoAvgOn()
    {
        setGeoAvgCmd(MavExtCmdArg_Enable);
    }

    Q_INVOKABLE void setGeoAvgOff()
    {
        setGeoAvgCmd(MavExtCmdArg_Disable);
    }

    /* 54 */
    void detectionControlCmd(MavlinkExtDetectionControlArgs cmd, int param2);

    /* 55 */
    Q_INVOKABLE void ARMarkerControlCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_ARMarkerControl, 0, enabled);
    }

    /* 56 */
    void geoMapControlCmd(MavlinkExtGeoMapControlArgs cmd, float param2, float param3, float param4, float param5);

    /* 57 */
    Q_INVOKABLE void streamControlCmd(MavlinkExtStreamControlArgs mode, int param2, int param3);

    /* 58 */
    void vmdControlCmd(MavlinkExtVMDControlArgs cmd, int param2);

    /* 59 */
    void multipleGCSControlCmd(MavlinkExtMultipleGCSControlArgs cmd, int param2);

    /* 60 */
    Q_INVOKABLE void updateINSCalibrationSetCmd(QVector3D values)
    {
        _sendDigicamCmd(MavExtCmd_UpdateINSCalibrationSet, values.x(), values.y(), values.z());
    }

    /* 61 */
    void trackerControlCmd(MavlinkExtTrackerControlArgs cmd, int param2);

    /* 62 */
    Q_INVOKABLE void plrControlCmd(MavlinkExtEnDisArgs enabled)
    {
        _sendDigicamCmd(MavExtCmd_PLRControl, enabled);
    }

    /* 63 */
    void irNoiseReductionCmd(MavlinkExtIRNoiseReductionArgs level)
    {
        _sendDigicamCmd(MavExtCmd_SetIRNoiseReductionLevel, level);
    }

    Q_INVOKABLE void setIRNoiseReductionHigh()
    {
        irNoiseReductionCmd(MavExtCmdArg_High);
    }

    Q_INVOKABLE void setIRNoiseReducitonMed()
    {
        irNoiseReductionCmd(MavExtCmdArg_Med);
    }

    Q_INVOKABLE void setIRNoiseReducitonLow()
    {
        irNoiseReductionCmd(MavExtCmdArg_Low);
    }

signals:
    void sendTask(MavlinkTask*);

private slots:
    void _initCamera();
    void _terrainDataReceived(bool success, QList<double> heights)
    {
        QGeoCoordinate coord = _pointToCoord;
        coord.setAltitude(success ? heights[0] : 0);
        setSysModePointToCoord(coord);
    }
    void _getAltAtCoord(QGeoCoordinate coord);

private:
    QGeoCoordinate _pointToCoord;

    void _sendMavCmdLongMsg(MAV_CMD cmd,         float param1=0.0f, float param2=0.0f, float param3=0.0f,
                           float param4=0.0f,   float param5=0.0f, float param6=0.0f, float param7=0.0f);

    void _sendDigicamCmd(float param1=0.0f, float param2=0.0f, float param3=0.0f,
                        float param4=0.0f, float param5=0.0f, float param6=0.0f, float param7=0.0f)
    {
        _sendMavCmdLongMsg( MAV_CMD_DO_DIGICAM_CONTROL, param1, param2, param3, param4, param5, param6, param7);
    }

    void _sendConfigCmd(MavlinkExtConfigArgs cmd, MavlinkExtConfigParams param2=MavExtConfigParam_End, float param3=0.0f,
                       float param4=0.0f,  float param5=0.0f, float param6=0.0f)
    {
        _sendDigicamCmd(MavExtCmd_ConfigCmd, cmd, param2, param3, param4, param5, param6);
    }
};
