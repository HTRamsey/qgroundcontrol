#include "NextVisionData.h"
#include "reports/AR_Markers_Report.h"
#include "reports/AuxCameraObjectDetection_Report.h"
#include "reports/Car_Count_Report.h"
#include "reports/Fire_Report.h"
#include "reports/GndCrs_Report.h"
#include "reports/IMU_Report.h"
#include "reports/Los_Report.h"
#include "reports/LosDir_Rate_Report.h"
#include "reports/LPR_Report.h"
#include "reports/ObjectDetection_Report.h"
#include "reports/OGLR_Report.h"
#include "reports/Parameter_Report.h"
#include "reports/PLR_Report.h"
#include "reports/RangeFinder_Report.h"
#include "reports/SDCard_Report.h"
#include "reports/Snapshot_Report.h"
#include "reports/Sys_Report.h"
#include "reports/Tracking_Report.h"
#include "reports/Video_Report.h"
#include "reports/VMD_Report.h"
#include <QtCore/QLoggingCategory>

static Q_LOGGING_CATEGORY(Log, "zat.comms.nextvisiondata")

NextVisionData::NextVisionData(QObject* parent): MavComponent(QHostAddress(GIMBAL_IP), GIMBAL_PORT, sysID(), compID(), parent),
    m_stream(QString("rtsp://%1:554/video").arg(getIPAddress().toString()))
{
    connect(this, &NextVisionData::ipChanged, this, [this]()
    {
        m_stream = QUrl(m_streamUrl.arg(getIPAddress().toString()));
        emit streamChanged();
    });

    (void) connect(MavLib::mavlib(), &MavLib::msgReceived, this, &NextVisionData::msgReceived, Qt::AutoConnection);

    qCDebug(Log) << Q_FUNC_INFO << this;
}

NextVisionData::~NextVisionData()
{
    qCDebug(Log) << Q_FUNC_INFO << this;
}

void NextVisionData::_handleMsg(const mavlink_message_t& msg)
{
    if(msg.msgid == MAVLINK_MSG_ID_HEARTBEAT)
    {

    }
    else if(msg.msgid == MAVLINK_MSG_ID_V2_EXTENSION)
    {
        const uint16_t report_type = _get_report_type(&msg);
        switch(report_type)
        {
            case MavExtReport_ARMarkers:
                _handleARMarkersMsg(&msg);
                break;

            case MavExtReport_RangeFinder:
                _handleRangeFinderMsg(&msg);
                break;

            case MavExtReport_CarCount:
                _handleCarCountMsg(&msg);
                break;

            case MavExtReport_Fire:
                _handleFireMsg(&msg);
                break;

            case MavExtReport_GndCrs:
                _handleGndCrsMsg(&msg);
                break;

            case MavExtReport_IMU:
                _handleIMUMsg(&msg);
                break;

            case MavExtReport_LOS:
                _handleLosMsg(&msg);
                break;

            case MavExtReport_LOSDirRate:
                _handleLosDirRateMsg(&msg);
                break;

            case MavExtReport_LPR:
                _handleLPRMsg(&msg);
                break;

            case MavExtReport_ObjDet:
                _handleObjDetMsg(&msg);
                break;

            case MavExtReport_OGLR:
                _handleOGLRMsg(&msg);
                break;

            case MavExtReport_Parameter:
                _handleParameterMsg(&msg);
                break;

            case MavExtReport_SDCard:
                _handleSDCardMsg(&msg);
                break;

            case MavExtReport_Snapshot:
                _handleSnapshotMsg(&msg);
                break;

            case MavExtReport_System:
                _handleSysMsg(&msg);
                break;

            case MavExtReport_Tracking:
                _handleTrackingMsg(&msg);
                break;

            case MavExtReport_Video:
                _handleVideoMsg(&msg);
                break;

            case MavExtReport_VMD:
                _handleVMDMsg(&msg);
                break;

            case MavExtReport_PLR:
                _handlePLRMsg(&msg);
                break;

            case MavExtReport_AuxCamObjDet:
                _handleAuxCamObjDetMsg(&msg);
                break;

            default:
                break;
        }
    }
}

void NextVisionData::_handleARMarkersMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_ar_markers_report_t report;
    mavlink_nvext_ar_markers_report_decode(_message, &report);

    const uint8_t detections = ((_message->len - 14) / 34);
}

void NextVisionData::_handleCarCountMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_car_count_report_t report;
    mavlink_nvext_car_count_report_decode(_message, &report);

    _carCount = report.car_count;
}

void NextVisionData::_handleFireMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_fire_report_t report;
    mavlink_nvext_fire_report_decode(_message, &report);

    _fireDetected = report.total_sat_pixels;
}

void NextVisionData::_handleGndCrsMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_gnd_crs_report_t report;
    mavlink_nvext_gnd_crs_report_decode(_message, &report);

    emit gndCrsReceived(report.gnd_crossing_lat, report.gnd_crossing_lon);
    _gndCrsAlt = report.gnd_crossing_alt;
}

void NextVisionData::_handleIMUMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_imu_report_t report;
    mavlink_nvext_imu_report_decode(_message, &report);
}

void NextVisionData::_handleLosMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_los_report_t report;
    mavlink_nvext_los_report_decode(_message, &report);

    float los_upper_left_corner_lat,    los_upper_left_corner_lon;
    float los_upper_right_corner_lat,   los_upper_right_corner_lon;
    float los_lower_right_corner_lat,   los_lower_right_corner_lon;
    float los_lower_left_corner_lat,    los_lower_left_corner_lon;
    uint8_t buf[64];
    memcpy(&buf, _MAV_PAYLOAD(_message),_message->len);
    memcpy(&los_upper_left_corner_lat,  &buf[14],4);
    memcpy(&los_upper_left_corner_lon,  &buf[18],4);
    memcpy(&los_upper_right_corner_lat, &buf[22],4);
    memcpy(&los_upper_right_corner_lon, &buf[26],4);
    memcpy(&los_lower_right_corner_lat, &buf[30],4);
    memcpy(&los_lower_right_corner_lon, &buf[34],4);
    memcpy(&los_lower_left_corner_lat,  &buf[38],4);
    memcpy(&los_lower_left_corner_lon,  &buf[42],4);

    QList<QGeoCoordinate> coords;
    if((double)los_upper_left_corner_lat < 360)
    {
        coords << QGeoCoordinate((double)los_upper_left_corner_lat,(double)los_upper_left_corner_lon);
    }
    if((double)los_upper_right_corner_lat < 360)
    {
        coords << QGeoCoordinate((double)los_upper_right_corner_lat,(double)los_upper_right_corner_lon);
    }
    if((double)los_lower_right_corner_lat != 400)
    {
        coords << QGeoCoordinate((double)los_lower_right_corner_lat,(double)los_lower_right_corner_lon);
    }
    if((double)los_lower_left_corner_lat != 400)
    {
        coords << QGeoCoordinate((double)los_lower_left_corner_lat,(double)los_lower_left_corner_lon);
    }
    if(coords.size() == 4)
    {
        // emit lineOfSightChanged(coords);
    }
    else
    {
        // QList<QGeoCoordinate> emptyCoords;
        // emit lineOfSightChanged(emptyCoords);
    }
}

void NextVisionData::_handleLosDirRateMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_los_dir_rate_report_t report;
    mavlink_nvext_los_dir_rate_report_decode(_message, &report);
}

void NextVisionData::_handleLPRMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_lpr_report_t report;
    mavlink_nvext_lpr_report_decode(_message, &report);

    const uint8_t detections = ((_message->len - 14) / 36);
}

void NextVisionData::_handleObjDetMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_object_detection_report_t report;
    mavlink_nvext_object_detection_report_decode(_message, &report);

    const uint8_t detections = ((_message->len - 14) / 32);
    const uint16_t classID = report.det_classID;
    const uint16_t uniqueID = report.det_uniqueID;
    const float lat = report.det_lat;
    const float lon = report.det_lon;
    const float alt = report.det_alt;
    switch((ObjDetHumanAndVehicleClassIDs)classID)
    {
        case ObjDetHumanAndVehicleClass_Human:
            break;

        case ObjDetHumanAndVehicleClass_Vehicle:
            break;

        case ObjDetHumanAndVehicleClass_Truck:
            break;

        default:
            break;
    }
    switch((ObjDetFireAndSmokeClassIDs)classID)
    {
        case ObjDetFireAndSmokeClass_Smoke:
            break;

        case ObjDetFireAndSmokeClass_Fire:
            break;

        default:
            break;
    }
    switch((ObjDetHumanOverboardClassIDs)classID)
    {
        case ObjDetHumanOverboardClass_Human:
            break;

        default:
            break;
    }
    switch((ObjDetMarineVesselClassIDs)classID)
    {
        case ObjDetMarineVesselClass_MarineVessel:
            break;

        default:
            break;
    }
    switch((ObjDetAircraftDetectorClassIDs)classID)
    {
        case ObjDetAircraftDetectorClass_Helicopter:
            break;

        case ObjDetAircraftDetectorClass_Airplane:
            break;

        default:
            break;
    }
    _objDetDetections = detections;
    _objDet0ClassID = classID;
    _objDet0UniqueID = uniqueID;
    _objDet0Lat = lat;
    _objDet0Lon = lon;
    _objDet0Alt = alt;
}

void NextVisionData::_handleOGLRMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_oglr_report_t report;
    mavlink_nvext_oglr_report_decode(_message, &report);
}

void NextVisionData::_handleParameterMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_parameter_report_t report;
    mavlink_nvext_parameter_report_decode(_message, &report);

    if(!report.status) return;
    const float value = report.param_value;
    // switch((CameraManagement::MavlinkExtConfigParams)report.param_id)
    // {
    //     case CameraManagement::MavExtConfigParam_VideoDestCh0:
    //         videoDestCh0 = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_VideoDestPort:
    //         videoDestPort = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EnableBandWidthLimit:
    //         enableBandWidthLimit = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_BandwithLimit:
    //         bandwithLimit = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_StreamBitrate:
    //         streamBitrate = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_RecordingBitrate:
    //         recordingBitrate = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EthIpAddr:
    //         ethIPAddr = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EthSubnetMask:
    //         ethSubnetMask = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EthGatewayIp:
    //         ethGatewayIP = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_MTU:
    //         MTU = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EnableRollDerot:
    //         enableRollDerot = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_AutoRecordEnable:
    //         autoRecordEnable = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_AutoRecordINSTimeout:
    //         autoRecordINSTimeout = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EnableAES:
    //         enableAES = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_AES128Key:
    //         AES128Key = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_RemoteIP:
    //         remoteIP = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_RemotePort:
    //         remotePort = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_LocalPort:
    //         localPort = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_RunFlightPlan:
    //         runFlightPlan = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ForceRollToHorizon:
    //         forceRollToHorizon = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_OSDMode:
    //         OSDMode = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ShowGndCrsInfo:
    //         showGndCrsInfo = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ShowGPSInfo:
    //         showGPSInfo = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ShowFlightMode:
    //         showFlightMode = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ShowAirSpeed:
    //         showAirSpeed = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_OSDMiniMap:
    //         OSDMiniMap = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_VGAResizingThreshold:
    //         VGAResizingThreshold = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_VGAOutputMode:
    //         VGAOutputMode = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ReportSystemID:
    //         reportSystemID = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ReportComponentID:
    //         reportComponentID = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_AckSystemID:
    //         ackSystemID = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_AckComponentID:
    //         ackComponentID = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EnableMavSequenceNum:
    //         enableMavSequenceNum = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_StreamGOPSize:
    //         streamGOPSize = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_StreamQuantizationVal:
    //         streamQuantizationVal = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EnableObjDet:
    //         enableObjDet = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ObjDetType:
    //         objDetType = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ObjDetAutoTrackEnable:
    //         objDetAutoTrackEnable = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ObjDetAutoZoomOnTarget:
    //         objDetAutoZoomOnTarget = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ObjDetLoiterAPOnDetect:
    //         objDetLoiterAPOnDetect = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_EnableALPR:
    //         enableALPR = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_ALPRCountryProfile:
    //         ALPRCountryProfile = value;
    //         break;

    //     case CameraManagement::MavExtConfigParam_AILicenseStatus:
    //         AILicenseStatus = value;
    //         break;

    //     default:
    //         break;
    // }
}

void NextVisionData::_handleSDCardMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_sd_card_report_t report;
    mavlink_nvext_sd_card_report_decode(_message, &report);

    const uint8_t detected = report.detected;
    const float total = report.total_capacity;
    const float available = report.available_capacity;
    _SDCardDetected = detected;
    _SDCardTotal = total;
    _SDCardAvailable = available;
}

void NextVisionData::_handleSnapshotMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_snapshot_report_t report;
    mavlink_nvext_snapshot_report_decode(_message, &report);
}

void NextVisionData::_handleSysMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_sys_report_t report;
    mavlink_nvext_sys_report_decode(_message, &report);

    const uint8_t tracking = report.tracker_status;
    const uint8_t recording = report.recording_status;
    const uint8_t mode = report.mode;
    const uint8_t snapshot_status = report.snapshot_busy;
    _systemTracking = tracking;
    _systemRecording = recording;
    _systemMode = mode;
    _systemSnapshotStatus = snapshot_status;
}

void NextVisionData::_handleTrackingMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_tracking_report_t report;
    mavlink_nvext_tracking_report_decode(_message, &report);

    const float lat = report.roi_lat;
    const float lon = report.roi_lon;
    const float alt = report.roi_alt;
    _trackingLat = lat;
    _trackingLon = lon;
    _trackingAlt = alt;
}

void NextVisionData::_handleVideoMsg(const mavlink_message_t* _message)
{
    mavlink_nvext_video_report_t report;
    mavlink_nvext_video_report_decode(_message, &report);

    const uint16_t bitrateStream0 = report.bitrateStream0;
    const uint16_t bitrateRecord0 = report.bitrateRecord0;
    _videoBitrateStream0 = bitrateStream0;
    _videoBitrateRecord0 = bitrateRecord0;
}

void NextVisionData::_handleVMDMsg(const mavlink_message_t* _message)
{
	mavlink_nvext_vmd_report_t report;
	mavlink_nvext_vmd_report_decode(_message, &report);

    const uint8_t detections = ((_message->len - 14) / 28);
    const float lat = report.objlat;
    const float lon = report.objlon;
    const float alt = report.objalt;
    _VMDLat = lat;
    _VMDLon = lon;
    _VMDAlt = alt;
}

void NextVisionData::_handlePLRMsg(const mavlink_message_t* _message)
{
	mavlink_nvext_plr_report_t report;
	mavlink_nvext_plr_report_decode(_message, &report);
}

void NextVisionData::_handleRangeFinderMsg(const mavlink_message_t* message)
{
    mavlink_nvext_range_finder_report_t report;
    mavlink_nvext_range_finder_report_decode(message, &report);
}

void NextVisionData::_handleAuxCamObjDetMsg(const mavlink_message_t* message)
{
    mavlink_nvext_aux_camera_object_detection_report_t report;
    mavlink_nvext_aux_camera_object_detection_report_decode(message, &report);
}

uint16_t NextVisionData::_get_report_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  0);
}
