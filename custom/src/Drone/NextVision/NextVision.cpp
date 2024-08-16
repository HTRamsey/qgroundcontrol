#include "NextVision.h"
#include "NextVisionData.h"
#include <QtCore/QLoggingCategory>

static Q_LOGGING_CATEGORY(Log, "zat.comms.nextvision")

NextVision::NextVision(QObject *parent): QObject(parent)
{
    connect(this, &NextVision::sendTask, MavLib::mavlib(), &MavLib::addTask, Qt::AutoConnection);
    qCDebug(Log) << Q_FUNC_INFO << this;
}

NextVision::~NextVision()
{
    qCDebug(Log) << Q_FUNC_INFO << this;
}

void NextVision::_initCamera()
{
    /*setSystemTimeCmd();
    setReportFrequencySystem(1);
    setReportFrequencyLOS(1);
    setReportFrequencyGC(1);
    setReportFrequencySDCard(1);
    setReportFrequencyVideo(1);*/
//    connect(&_data, &NextVisionData::gndCrsReceived, this, &NextVision::_getAltAtCoord);
    // for (uint8_t i = 0; i < MavExtConfigParam_End; i++)
    // {
    //     getParam((MavlinkExtConfigParams)i);
    //     QThread::msleep(100);
    //     APP->processEvents(QEventLoop::ExcludeUserInputEvents);
    // }
    // updateVideoIP()
    // setVideoTransmissionCh0On();
    // setVideoTransmissionCh1Off();
}

void NextVision::_sendMavCmdLongMsg( MAV_CMD cmd,  float param1,   float param2,   float param3,
                                          float param4, float param5,   float param6,   float param7)
{
    emit sendTask(new CommandLongTask(255, 0, cmd, param1, param2, param3, param4, param5, param6, param7));
}

void NextVision::_getAltAtCoord(QGeoCoordinate coord)
{
//    if(!coord.isValid()) return;
//    coord.setAltitude(homePosition().altitude());
//    setGroundCrossingAlt(coord.altitude());
}

/* 0 */
void NextVision::pointToCoordinate(QGeoCoordinate coord)
{
    if(!coord.isValid()) return;
    _pointToCoord.setLatitude(coord.latitude());
    _pointToCoord.setLongitude(coord.longitude());
    if(coord.type() == QGeoCoordinate::CoordinateType::Coordinate3D)
    {
        _pointToCoord.setAltitude(coord.altitude());
    }
    setSysModePointToCoord(_pointToCoord);
}

/* 6 */
void NextVision::setGimbalCmd(float roll_rate, float pitch_rate, MavlinkExtSetGimbalArgs zoom_state, float alt)
{
    static float last_cam_roll_yaw = 0, last_cam_pitch = 0, last_alt = 0;
    if((roll_rate != last_cam_roll_yaw) || (pitch_rate != last_cam_pitch) || (last_alt != alt))
    {
        last_cam_roll_yaw = roll_rate;
        last_cam_pitch = pitch_rate;
        last_alt = alt;
    } else return;
    _sendMavCmdLongMsg(MAV_CMD_DO_DIGICAM_CONTROL, MavExtCmd_SetGimbal, roll_rate, pitch_rate, zoom_state, alt);
}

void NextVision::sendGimbalVirtualCmd(float cam_roll_yaw, float cam_pitch)
{
    setGimbalCmd(cam_roll_yaw, cam_pitch, MavExtCmdArg_GimbalZoomNoChange, 0.0); //TODO: use new calculated value from LOS message
}

/* 31 */
void NextVision::configCmd(MavlinkExtConfigArgs operation, MavlinkExtConfigParams param, float opt1, float opt2, float opt3, float opt4)
{
    if(operation == MavExtCmdArg_SaveParams || operation == MavExtCmdArg_Reboot)
    {
        _sendConfigCmd(operation);
    }
    if(operation == MavExtCmdArg_GetParam)
    {
        switch(param)
        {
            case MavExtConfigParam_EnableObjDet:
                _sendConfigCmd(operation, param);
                break;

            case MavExtConfigParam_ObjDetType:
                _sendConfigCmd(operation, param);
                break;

            case MavExtConfigParam_AILicenseStatus:
                _sendConfigCmd(operation, param);
                break;

            default:
                break;
        }
    }
    if(operation == MavExtCmdArg_GetParam || operation == MavExtCmdArg_UpdateParam)
    {
        switch(param)
        {
            case MavExtConfigParam_VideoDestCh0:
                _sendConfigCmd(operation, param, opt1, opt2, opt3, opt4);
                break;

            case MavExtConfigParam_VideoDestPort:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_EnableBandWidthLimit:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_BandwithLimit:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_StreamBitrate:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_RecordingBitrate:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_EthIpAddr:
                _sendConfigCmd(operation, param, opt1, opt2, opt3, opt4);
                break;

            case MavExtConfigParam_EthSubnetMask:
                _sendConfigCmd(operation, param, opt1, opt2, opt3, opt4);
                break;

            case MavExtConfigParam_EthGatewayIp:
                _sendConfigCmd(operation, param, opt1, opt2, opt3, opt4);
                break;

            case MavExtConfigParam_MTU:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_EnableRollDerot:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_AutoRecordEnable:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_AutoRecordINSTimeout:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_EnableAES:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_AES128Key:
                _sendConfigCmd(operation, param, opt1, opt2, opt3, opt4);
                break;

            case MavExtConfigParam_RemoteIP:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_RemotePort:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_LocalPort:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_RunFlightPlan:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ForceRollToHorizon:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_OSDMode:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ShowGndCrsInfo:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ShowGPSInfo:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ShowFlightMode:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ShowAirSpeed:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_OSDMiniMap:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_VGAResizingThreshold:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_VGAOutputMode:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_ReportSystemID:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ReportComponentID:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_AckSystemID:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_AckComponentID:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_EnableMavSequenceNum:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_StreamGOPSize:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_StreamQuantizationVal:
                _sendConfigCmd(operation, param, opt1, opt2);
                break;

            case MavExtConfigParam_ObjDetAutoTrackEnable:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ObjDetAutoZoomOnTarget:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ObjDetLoiterAPOnDetect:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_EnableALPR:
                _sendConfigCmd(operation, param, opt1);
                break;

            case MavExtConfigParam_ALPRCountryProfile:
                _sendConfigCmd(operation, param, opt1);
                break;

            default:
                break;
        }
    }
}

/* 54 */
void NextVision::detectionControlCmd(MavlinkExtDetectionControlArgs cmd, int param2)
{
    switch(cmd)
    {
        case MavExtCmdArg_DetectorEnDis:
            _sendDigicamCmd(MavExtCmd_DetectionControl, cmd, (MavlinkExtEnDisArgs) param2);
            break;

        case MavExtCmdArg_DetectorSelect:
            _sendDigicamCmd(MavExtCmd_DetectionControl, cmd, (MavlinkExtDetectionControlDetectorSelectArgs) param2);
            break;

        case MavExtCmdArg_DetectorThreshold:
            _sendDigicamCmd(MavExtCmd_DetectionControl, cmd, (uint8_t) param2);
            break;

        case MavExtCmdArg_DetectorResumeScan:
            _sendDigicamCmd(MavExtCmd_DetectionControl, cmd, (uint8_t) param2);
            break;

        case MavExtCmdArg_DetectorFireThres:
            _sendDigicamCmd(MavExtCmd_DetectionControl, cmd, (uint16_t) param2);
            break;

        case MavExtCmdArg_EnableCarCounting:
            _sendDigicamCmd(MavExtCmd_DetectionControl, cmd, (MavlinkExtEnDisArgs) param2);
            break;

        case MavExtCmdArg_ResetCarCounting:
            _sendDigicamCmd(MavExtCmd_DetectionControl, cmd);
            break;

        default:
            break;
    }
}

/* 56 */
void NextVision::geoMapControlCmd(MavlinkExtGeoMapControlArgs cmd, float opt1, float opt2, float opt3, float opt4)
{
    switch(cmd)
    {
        case MavExtCmdArg_RefineLocation:
            _sendDigicamCmd(MavExtCmd_GeoMapControl,cmd,opt1,opt2);
            break;
        case MavExtCmdArg_SetReferencePoint:
            _sendDigicamCmd(MavExtCmd_GeoMapControl,cmd,opt1,opt2,opt3,opt4);
            break;
        default:
            break;
    }
}

/* 57 */
void NextVision::streamControlCmd(MavlinkExtStreamControlArgs mode, int opt1, int opt2)
{
    switch(mode)
    {
        case MavExtCmdArg_StreamMode:
            _sendDigicamCmd(MavExtCmd_StreamControl, mode, (MavlinkExtStreamControlStreamModeArgs) opt1, (MavlinkExtStreamControlStreamModeArgs) opt2);
            break;

        case MavExtCmdArg_PipMode:
            _sendDigicamCmd(MavExtCmd_StreamControl, mode, (MavlinkExtStreamControlPIPModeArgs) opt1                                                 );
            break;

        case MavExtCmdArg_SBSMode:
            _sendDigicamCmd(MavExtCmd_StreamControl, mode, (MavlinkExtStreamControlSBSModeArgs) opt1                                                 );
            break;

        default:
            break;
    }
}

/* 58 */
void NextVision::vmdControlCmd(MavlinkExtVMDControlArgs cmd, int param2)
{
    switch(cmd)
    {
        case MavExtCmdArg_EnableVMD:
            _sendDigicamCmd(MavExtCmd_VMDControl, cmd, (MavlinkExtEnDisArgs) param2);
            break;

        case MavExtCmdArg_PolygonColorSelect:
            _sendDigicamCmd(MavExtCmd_VMDControl, cmd, (MavlinkExtVMDControlPolygonColorArgs) param2);
            break;

        case MavExtCmdArg_EnableReports:
            _sendDigicamCmd(MavExtCmd_VMDControl, cmd, (MavlinkExtEnDisArgs) param2);
            break;

        default:
            break;
    }
}

/* 59 */
void NextVision::multipleGCSControlCmd(MavlinkExtMultipleGCSControlArgs cmd, int param2)
{
    switch(cmd)
    {
        case MavExtCmdArg_EnableSecondaryControl:
            _sendDigicamCmd(MavExtCmd_MultipleGCSControl, cmd, (MavlinkExtEnDisArgs) param2);
            break;

        case MavExtCmdArg_EnableSecondaryReports:
            _sendDigicamCmd(MavExtCmd_MultipleGCSControl, cmd, (MavlinkExtEnDisArgs) param2);
            break;

        default:
            break;
    }
}

/* 61 */
void NextVision::trackerControlCmd(MavlinkExtTrackerControlArgs cmd, int param2)
{
    switch(cmd)
    {
        case MavExtCmdArg_SetPrimaryTracker:
            _sendDigicamCmd(MavExtCmd_TrackerControl, cmd, (MavlinkExtTrackerControlTrackerArgs) param2);
            break;

        case MavExtCmdArg_SetActiveTracker:
            _sendDigicamCmd(MavExtCmd_TrackerControl, cmd, (MavlinkExtTrackerControlTrackerArgs) param2);
            break;

        case MavExtCmdArg_SetTrackerROI:
            _sendDigicamCmd(MavExtCmd_TrackerControl, cmd, (MavlinkExtTrackerControlROIArgs) param2);
            break;

        default:
            break;
    }
}
