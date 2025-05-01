/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCMAVLink.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(QGCMAVLinkLog, "qgc.mavlink.qgcmavlink")

const QHash<int, QString> QGCMAVLink::mavlinkCompIdHash {
    { MAV_COMP_ID_CAMERA,   "Camera1" },
    { MAV_COMP_ID_CAMERA2,  "Camera2" },
    { MAV_COMP_ID_CAMERA3,  "Camera3" },
    { MAV_COMP_ID_CAMERA4,  "Camera4" },
    { MAV_COMP_ID_CAMERA5,  "Camera5" },
    { MAV_COMP_ID_CAMERA6,  "Camera6" },
    { MAV_COMP_ID_SERVO1,   "Servo1" },
    { MAV_COMP_ID_SERVO2,   "Servo2" },
    { MAV_COMP_ID_SERVO3,   "Servo3" },
    { MAV_COMP_ID_SERVO4,   "Servo4" },
    { MAV_COMP_ID_SERVO5,   "Servo5" },
    { MAV_COMP_ID_SERVO6,   "Servo6" },
    { MAV_COMP_ID_SERVO7,   "Servo7" },
    { MAV_COMP_ID_SERVO8,   "Servo8" },
    { MAV_COMP_ID_SERVO9,   "Servo9" },
    { MAV_COMP_ID_SERVO10,  "Servo10" },
    { MAV_COMP_ID_SERVO11,  "Servo11" },
    { MAV_COMP_ID_SERVO12,  "Servo12" },
    { MAV_COMP_ID_SERVO13,  "Servo13" },
    { MAV_COMP_ID_SERVO14,  "Servo14" },
    { MAV_COMP_ID_GIMBAL,   "Gimbal1" },
    { MAV_COMP_ID_ADSB,     "ADSB" },
    { MAV_COMP_ID_OSD,      "OSD" },
    { MAV_COMP_ID_FLARM,    "FLARM" },
    { MAV_COMP_ID_GIMBAL2,  "Gimbal2" },
    { MAV_COMP_ID_GIMBAL3,  "Gimbal3" },
    { MAV_COMP_ID_GIMBAL4,  "Gimbal4" },
    { MAV_COMP_ID_GIMBAL5,  "Gimbal5" },
    { MAV_COMP_ID_GIMBAL6,  "Gimbal6" },
    { MAV_COMP_ID_IMU,      "IMU1" },
    { MAV_COMP_ID_IMU_2,    "IMU2" },
    { MAV_COMP_ID_IMU_3,    "IMU3" },
    { MAV_COMP_ID_GPS,      "GPS1" },
    { MAV_COMP_ID_GPS2,     "GPS2" }
};

#ifdef MAVLINK_EXTERNAL_RX_STATUS
    mavlink_status_t m_mavlink_status[MAVLINK_COMM_NUM_BUFFERS];
#endif

#ifdef MAVLINK_GET_CHANNEL_STATUS
mavlink_status_t* mavlink_get_channel_status(uint8_t channel)
{
#ifndef MAVLINK_EXTERNAL_RX_STATUS
    static QList<mavlink_status_t> m_mavlink_status(MAVLINK_COMM_NUM_BUFFERS);
#endif
    if (!QGCMAVLink::isValidChannel(channel)) {
        qCWarning(QGCMAVLinkLog) << Q_FUNC_INFO << "Invalid Channel Number:" << channel;
        return nullptr;
    }

    return &m_mavlink_status[channel];
}
#endif

QGCMAVLink::QGCMAVLink(QObject *parent)
    : QObject(parent)
{
    // qCDebug(StatusTextHandlerLog) << Q_FUNC_INFO << this;

   (void) qRegisterMetaType<mavlink_message_t>("mavlink_message_t");
   (void) qRegisterMetaType<MAV_TYPE>("MAV_TYPE");
   (void) qRegisterMetaType<MAV_AUTOPILOT>("MAV_AUTOPILOT");
   (void) qRegisterMetaType<GRIPPER_ACTIONS>("GRIPPER_ACTIONS");
}

QGCMAVLink::~QGCMAVLink()
{
    // qCDebug(StatusTextHandlerLog) << Q_FUNC_INFO << this;
}

QList<QGCMAVLink::FirmwareClass_t> QGCMAVLink::allFirmwareClasses(void)
{
    static const QList<QGCMAVLink::FirmwareClass_t> classes = {
        FirmwareClassPX4,
        FirmwareClassArduPilot,
        FirmwareClassGeneric
    };

    return classes;
}

QList<QGCMAVLink::VehicleClass_t> QGCMAVLink::allVehicleClasses(void)
{
    static const QList<QGCMAVLink::VehicleClass_t> classes = {
        VehicleClassFixedWing,
        VehicleClassRoverBoat,
        VehicleClassSub,
        VehicleClassMultiRotor,
        VehicleClassVTOL,
        VehicleClassGeneric,
    };

    return classes;
}

QGCMAVLink::FirmwareClass_t QGCMAVLink::firmwareClass(MAV_AUTOPILOT autopilot)
{
    if (isPX4FirmwareClass(autopilot)) {
        return FirmwareClassPX4;
    } else if (isArduPilotFirmwareClass(autopilot)) {
        return FirmwareClassArduPilot;
    } else {
        return FirmwareClassGeneric;
    }
}

QString QGCMAVLink::firmwareClassToString(FirmwareClass_t firmwareClass)
{
    switch (firmwareClass) {
    case FirmwareClassPX4:
        return QT_TRANSLATE_NOOP("Firmware Class", "PX4 Pro");
    case FirmwareClassArduPilot:
        return QT_TRANSLATE_NOOP("Firmware Class", "ArduPilot");
    case FirmwareClassGeneric:
        return QT_TRANSLATE_NOOP("Firmware Class", "Generic");
    default:
        return QT_TRANSLATE_NOOP("Firmware Class", "Unknown");
    }
}

bool QGCMAVLink::isAirship(MAV_TYPE mavType)
{
    return vehicleClass(mavType) == VehicleClassAirship;
}

bool QGCMAVLink::isFixedWing(MAV_TYPE mavType)
{
    return vehicleClass(mavType) == VehicleClassFixedWing;
}

bool QGCMAVLink::isRoverBoat(MAV_TYPE mavType)
{
    return vehicleClass(mavType) == VehicleClassRoverBoat;
}

bool QGCMAVLink::isSub(MAV_TYPE mavType)
{
    return vehicleClass(mavType) == VehicleClassSub;
}

bool QGCMAVLink::isMultiRotor(MAV_TYPE mavType)
{
    return vehicleClass(mavType) == VehicleClassMultiRotor;
}

bool QGCMAVLink::isVTOL(MAV_TYPE mavType)
{
    return vehicleClass(mavType) == VehicleClassVTOL;
}

QGCMAVLink::VehicleClass_t QGCMAVLink::vehicleClass(MAV_TYPE mavType)
{
    switch (mavType) {
    case MAV_TYPE_GROUND_ROVER:
    case MAV_TYPE_SURFACE_BOAT:
        return VehicleClassRoverBoat;
    case MAV_TYPE_SUBMARINE:
        return VehicleClassSub;
    case MAV_TYPE_QUADROTOR:
    case MAV_TYPE_COAXIAL:
    case MAV_TYPE_HELICOPTER:
    case MAV_TYPE_HEXAROTOR:
    case MAV_TYPE_OCTOROTOR:
    case MAV_TYPE_TRICOPTER:
        return VehicleClassMultiRotor;
    case MAV_TYPE_VTOL_TAILSITTER_DUOROTOR:
    case MAV_TYPE_VTOL_TAILSITTER_QUADROTOR:
    case MAV_TYPE_VTOL_TILTROTOR:
    case MAV_TYPE_VTOL_FIXEDROTOR:
    case MAV_TYPE_VTOL_TAILSITTER:
    case MAV_TYPE_VTOL_TILTWING:
    case MAV_TYPE_VTOL_RESERVED5:
        return VehicleClassVTOL;
    case MAV_TYPE_FIXED_WING:
        return VehicleClassFixedWing;
    case MAV_TYPE_AIRSHIP:
        return VehicleClassAirship;
    default:
        return VehicleClassGeneric;
    }
}

QString QGCMAVLink::vehicleClassToUserVisibleString(VehicleClass_t vehicleClass)
{
    switch (vehicleClass) {
    case VehicleClassAirship:
        return QT_TRANSLATE_NOOP("Vehicle Class", "Airship");
    case VehicleClassFixedWing:
        return QT_TRANSLATE_NOOP("Vehicle Class", "Fixed Wing");
    case VehicleClassRoverBoat:
        return QT_TRANSLATE_NOOP("Vehicle Class", "Rover-Boat");
    case VehicleClassSub:
        return QT_TRANSLATE_NOOP("Vehicle Class", "Sub");
    case VehicleClassMultiRotor:
        return QT_TRANSLATE_NOOP("Vehicle Class", "Multi-Rotor");
    case VehicleClassVTOL:
        return QT_TRANSLATE_NOOP("Vehicle Class", "VTOL");
    case VehicleClassGeneric:
        return QT_TRANSLATE_NOOP("Vehicle Class", "Generic");
    default:
        return QT_TRANSLATE_NOOP("Vehicle Class", "Unknown");
    }
}

QString QGCMAVLink::vehicleClassToInternalString(VehicleClass_t vehicleClass)
{
    switch (vehicleClass) {
    case VehicleClassAirship:
        return QStringLiteral("Airship");
    case VehicleClassFixedWing:
        return QStringLiteral("FixedWing");
    case VehicleClassRoverBoat:
        return QStringLiteral("RoverBoat");
    case VehicleClassSub:
        return QStringLiteral("Sub");
    case VehicleClassMultiRotor:
        return QStringLiteral("MultiRotor");
    case VehicleClassVTOL:
        return QStringLiteral("VTOL");
    case VehicleClassGeneric:
        return QStringLiteral("Generic");
    default:
        return QStringLiteral("Unknown");
    }
}

QString QGCMAVLink::mavResultToString(MAV_RESULT result)
{
    switch (result) {
    case MAV_RESULT_ACCEPTED:
        return QStringLiteral("MAV_RESULT_ACCEPTED");
    case MAV_RESULT_TEMPORARILY_REJECTED:
        return QStringLiteral("MAV_RESULT_TEMPORARILY_REJECTED");
    case MAV_RESULT_DENIED:
        return QStringLiteral("MAV_RESULT_DENIED");
    case MAV_RESULT_UNSUPPORTED:
        return QStringLiteral("MAV_RESULT_UNSUPPORTED");
    case MAV_RESULT_FAILED:
        return QStringLiteral("MAV_RESULT_FAILED");
    case MAV_RESULT_IN_PROGRESS:
        return QStringLiteral("MAV_RESULT_IN_PROGRESS");
    default:
        return QStringLiteral("MAV_RESULT unknown %1").arg(result);
    }
}

QString QGCMAVLink::mavSysStatusSensorToString(MAV_SYS_STATUS_SENSOR sysStatusSensor)
{
    struct sensorInfo_s {
        uint32_t    bit;
        QString     sensorName;
    };

    static const sensorInfo_s rgSensorInfo[] = {
        { MAV_SYS_STATUS_SENSOR_3D_GYRO,                QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Gyro") },
        { MAV_SYS_STATUS_SENSOR_3D_ACCEL,               QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Accelerometer") },
        { MAV_SYS_STATUS_SENSOR_3D_MAG,                 QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Magnetometer") },
        { MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE,      QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Absolute pressure") },
        { MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE,  QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Differential pressure") },
        { MAV_SYS_STATUS_SENSOR_GPS,                    QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "GPS") },
        { MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW,           QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Optical flow") },
        { MAV_SYS_STATUS_SENSOR_VISION_POSITION,        QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Computer vision position") },
        { MAV_SYS_STATUS_SENSOR_LASER_POSITION,         QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Laser based position") },
        { MAV_SYS_STATUS_SENSOR_EXTERNAL_GROUND_TRUTH,  QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "External ground truth") },
        { MAV_SYS_STATUS_SENSOR_ANGULAR_RATE_CONTROL,   QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Angular rate control") },
        { MAV_SYS_STATUS_SENSOR_ATTITUDE_STABILIZATION, QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Attitude stabilization") },
        { MAV_SYS_STATUS_SENSOR_YAW_POSITION,           QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Yaw position") },
        { MAV_SYS_STATUS_SENSOR_Z_ALTITUDE_CONTROL,     QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Z/altitude control") },
        { MAV_SYS_STATUS_SENSOR_XY_POSITION_CONTROL,    QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "X/Y position control") },
        { MAV_SYS_STATUS_SENSOR_MOTOR_OUTPUTS,          QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Motor outputs / control") },
        { MAV_SYS_STATUS_SENSOR_RC_RECEIVER,            QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "RC receiver") },
        { MAV_SYS_STATUS_SENSOR_3D_GYRO2,               QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Gyro 2") },
        { MAV_SYS_STATUS_SENSOR_3D_ACCEL2,              QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Accelerometer 2") },
        { MAV_SYS_STATUS_SENSOR_3D_MAG2,                QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Magnetometer 2") },
        { MAV_SYS_STATUS_GEOFENCE,                      QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "GeoFence") },
        { MAV_SYS_STATUS_AHRS,                          QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "AHRS") },
        { MAV_SYS_STATUS_TERRAIN,                       QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Terrain") },
        { MAV_SYS_STATUS_REVERSE_MOTOR,                 QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Motors reversed") },
        { MAV_SYS_STATUS_LOGGING,                       QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Logging") },
        { MAV_SYS_STATUS_SENSOR_BATTERY,                QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Battery") },
        { MAV_SYS_STATUS_SENSOR_PROXIMITY,              QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Proximity") },
        { MAV_SYS_STATUS_SENSOR_SATCOM,                 QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Satellite Communication") },
        { MAV_SYS_STATUS_PREARM_CHECK,                  QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Pre-Arm Check") },
        { MAV_SYS_STATUS_OBSTACLE_AVOIDANCE,            QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Avoidance/collision prevention") },
        { MAV_SYS_STATUS_SENSOR_PROPULSION,             QT_TRANSLATE_NOOP("MAVLink SYS_STATUS_SENSOR value", "Propulsion") }
    };

    for (size_t i=0; i<sizeof(rgSensorInfo)/sizeof(sensorInfo_s); i++) {
        const sensorInfo_s* pSensorInfo = &rgSensorInfo[i];
        if (sysStatusSensor == pSensorInfo->bit) {
            return pSensorInfo->sensorName;
        }
    }

    qWarning() << "QGCMAVLink::mavSysStatusSensorToString: Unknown sensor" << sysStatusSensor;

    return QT_TRANSLATE_NOOP("MAVLink unknown SYS_STATUS_SENSOR value", "Unknown sensor");
}

QString QGCMAVLink::mavTypeToString(MAV_TYPE mavType) {
    static const QMap<int, QString> typeNames = {
        { MAV_TYPE_GENERIC,         tr("Generic micro air vehicle" )},
        { MAV_TYPE_FIXED_WING,      tr("Fixed wing aircraft")},
        { MAV_TYPE_QUADROTOR,       tr("Quadrotor")},
        { MAV_TYPE_COAXIAL,         tr("Coaxial helicopter")},
        { MAV_TYPE_HELICOPTER,      tr("Normal helicopter with tail rotor.")},
        { MAV_TYPE_ANTENNA_TRACKER, tr("Ground installation")},
        { MAV_TYPE_GCS,             tr("Operator control unit / ground control station")},
        { MAV_TYPE_AIRSHIP,         tr("Airship, controlled")},
        { MAV_TYPE_FREE_BALLOON,    tr("Free balloon, uncontrolled")},
        { MAV_TYPE_ROCKET,          tr("Rocket")},
        { MAV_TYPE_GROUND_ROVER,    tr("Ground rover")},
        { MAV_TYPE_SURFACE_BOAT,    tr("Surface vessel, boat, ship")},
        { MAV_TYPE_SUBMARINE,       tr("Submarine")},
        { MAV_TYPE_HEXAROTOR,       tr("Hexarotor")},
        { MAV_TYPE_OCTOROTOR,       tr("Octorotor")},
        { MAV_TYPE_TRICOPTER,       tr("trirotor")},
        { MAV_TYPE_FLAPPING_WING,   tr("Flapping wing")},
        { MAV_TYPE_KITE,            tr("Kite")},
        { MAV_TYPE_ONBOARD_CONTROLLER, tr("Onboard companion controller")},
        { MAV_TYPE_VTOL_TAILSITTER_DUOROTOR,   tr("Two-rotor VTOL using control surfaces in vertical operation in addition. Tailsitter")},
        { MAV_TYPE_VTOL_TAILSITTER_QUADROTOR,  tr("Quad-rotor VTOL using a V-shaped quad config in vertical operation. Tailsitter")},
        { MAV_TYPE_VTOL_TILTROTOR,  tr("Tiltrotor VTOL")},
        { MAV_TYPE_VTOL_FIXEDROTOR,  tr("VTOL Fixedrotor")},
        { MAV_TYPE_VTOL_TAILSITTER,  tr("VTOL Tailsitter")},
        { MAV_TYPE_VTOL_TILTWING,   tr("VTOL Tiltwing")},
        { MAV_TYPE_VTOL_RESERVED5,  tr("VTOL reserved 5")},
        { MAV_TYPE_GIMBAL,          tr("Onboard gimbal")},
        { MAV_TYPE_ADSB,            tr("Onboard ADSB peripheral")},
    };

    return typeNames.value(mavType, "MAV_TYPE_UNKNOWN");
}

QString QGCMAVLink::firmwareVersionTypeToString(FIRMWARE_VERSION_TYPE firmwareVersionType)
{
    switch (firmwareVersionType) {
    case FIRMWARE_VERSION_TYPE_DEV:
        return QStringLiteral("dev");
    case FIRMWARE_VERSION_TYPE_ALPHA:
        return QStringLiteral("alpha");
    case FIRMWARE_VERSION_TYPE_BETA:
        return QStringLiteral("beta");
    case FIRMWARE_VERSION_TYPE_RC:
        return QStringLiteral("rc");
    case FIRMWARE_VERSION_TYPE_OFFICIAL:
    default:
        return QString();
    }
}

int QGCMAVLink::motorCount(MAV_TYPE mavType, uint8_t frameType)
{
    switch (mavType) {
    case MAV_TYPE_HELICOPTER:
        return 1;
    case MAV_TYPE_VTOL_TAILSITTER_DUOROTOR:
        return 2;
    case MAV_TYPE_TRICOPTER:
        return 3;
    case MAV_TYPE_QUADROTOR:
    case MAV_TYPE_VTOL_TAILSITTER_QUADROTOR:
        return 4;
    case MAV_TYPE_HEXAROTOR:
        return 6;
    case MAV_TYPE_OCTOROTOR:
        return 8;
    case MAV_TYPE_SUBMARINE:
    {
        // Supported frame types
        enum {
            SUB_FRAME_BLUEROV1,
            SUB_FRAME_VECTORED,
            SUB_FRAME_VECTORED_6DOF,
            SUB_FRAME_VECTORED_6DOF_90DEG,
            SUB_FRAME_SIMPLEROV_3,
            SUB_FRAME_SIMPLEROV_4,
            SUB_FRAME_SIMPLEROV_5,
            SUB_FRAME_CUSTOM
        };

        switch (frameType) {  // ardupilot/libraries/AP_Motors/AP_Motors6DOF.h sub_frame_t

        case SUB_FRAME_BLUEROV1:
        case SUB_FRAME_VECTORED:
            return 6;

        case SUB_FRAME_SIMPLEROV_3:
            return 3;

        case SUB_FRAME_SIMPLEROV_4:
            return 4;

        case SUB_FRAME_SIMPLEROV_5:
            return 5;

        case SUB_FRAME_VECTORED_6DOF:
        case SUB_FRAME_VECTORED_6DOF_90DEG:
        case SUB_FRAME_CUSTOM:
            return 8;

        default:
            return -1;
        }
    }

    default:
        return -1;
    }
}

uint32_t QGCMAVLink::highLatencyFailuresToMavSysStatus(mavlink_high_latency2_t& highLatency2)
{
    struct failure2Sensor_s {
        HL_FAILURE_FLAG         failureBit;
        MAV_SYS_STATUS_SENSOR   sensorBit;
    };

    static constexpr const failure2Sensor_s rgFailure2Sensor[] = {
        { HL_FAILURE_FLAG_GPS,                      MAV_SYS_STATUS_SENSOR_GPS },
        { HL_FAILURE_FLAG_DIFFERENTIAL_PRESSURE,    MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE },
        { HL_FAILURE_FLAG_ABSOLUTE_PRESSURE,        MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE },
        { HL_FAILURE_FLAG_3D_ACCEL,                 MAV_SYS_STATUS_SENSOR_3D_ACCEL },
        { HL_FAILURE_FLAG_3D_GYRO,                  MAV_SYS_STATUS_SENSOR_3D_GYRO },
        { HL_FAILURE_FLAG_3D_MAG,                   MAV_SYS_STATUS_SENSOR_3D_MAG },
    };

    // Map from MAV_FAILURE bits to standard SYS_STATUS message handling
    uint32_t onboardControlSensorsEnabled = 0;
    for (size_t i=0; i<sizeof(rgFailure2Sensor)/sizeof(failure2Sensor_s); i++) {
        const failure2Sensor_s* pFailure2Sensor = &rgFailure2Sensor[i];
        if (highLatency2.failure_flags & pFailure2Sensor->failureBit) {
            // Assume if reporting as unhealthy that is it present and enabled
            onboardControlSensorsEnabled |= pFailure2Sensor->sensorBit;
        }
    }

    return onboardControlSensorsEnabled;
}

QVariant QGCMAVLink::valueFromParamExt(const char *value, uint8_t param_type)
{
    QVariant var;
    param_ext_union_t u{};
    (void) memcpy(u.bytes, value, MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_VALUE_LEN);
    switch (param_type) {
    case MAV_PARAM_EXT_TYPE_REAL32:
        var = QVariant(u.param_float);
        break;
    case MAV_PARAM_EXT_TYPE_REAL64:
        var = QVariant(u.param_double);
        break;
    case MAV_PARAM_EXT_TYPE_UINT8:
        var = QVariant(u.param_uint8);
        break;
    case MAV_PARAM_EXT_TYPE_INT8:
        var = QVariant(u.param_int8);
        break;
    case MAV_PARAM_EXT_TYPE_UINT16:
        var = QVariant(u.param_uint16);
        break;
    case MAV_PARAM_EXT_TYPE_INT16:
        var = QVariant(u.param_int16);
        break;
    case MAV_PARAM_EXT_TYPE_UINT32:
        var = QVariant(u.param_uint32);
        break;
    case MAV_PARAM_EXT_TYPE_INT32:
        var = QVariant(u.param_int32);
        break;
    case MAV_PARAM_EXT_TYPE_UINT64:
        var = QVariant(static_cast<qulonglong>(u.param_uint64));
        break;
    case MAV_PARAM_EXT_TYPE_INT64:
        var = QVariant(static_cast<qulonglong>(u.param_int64));
        break;
    case MAV_PARAM_EXT_TYPE_CUSTOM:
        var = QVariant(QByteArray(value, MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_VALUE_LEN));
        break;
    default:
        var = QVariant(0);
        qCCritical(QGCMAVLinkLog) << "Invalid param_type:" << param_type;
    }
    return var;
}

MAV_PARAM_EXT_TYPE QGCMAVLink::mavParamType(FactMetaData::ValueType_t type)
{
    switch (type) {
    case FactMetaData::valueTypeUint8:
    case FactMetaData::valueTypeBool:
        return MAV_PARAM_EXT_TYPE_UINT8;
    case FactMetaData::valueTypeInt8:
        return MAV_PARAM_EXT_TYPE_INT8;
    case FactMetaData::valueTypeUint16:
        return MAV_PARAM_EXT_TYPE_UINT16;
    case FactMetaData::valueTypeInt16:
        return MAV_PARAM_EXT_TYPE_INT16;
    case FactMetaData::valueTypeUint32:
        return MAV_PARAM_EXT_TYPE_UINT32;
    case FactMetaData::valueTypeUint64:
        return MAV_PARAM_EXT_TYPE_UINT64;
    case FactMetaData::valueTypeInt64:
        return MAV_PARAM_EXT_TYPE_INT64;
    case FactMetaData::valueTypeFloat:
        return MAV_PARAM_EXT_TYPE_REAL32;
    case FactMetaData::valueTypeDouble:
        return MAV_PARAM_EXT_TYPE_REAL64;
    case FactMetaData::valueTypeString:
    case FactMetaData::valueTypeCustom:
        return MAV_PARAM_EXT_TYPE_CUSTOM;
    default:
        qCWarning(QGCMAVLinkLog) << "Unsupported fact type" << type;
    case FactMetaData::valueTypeInt32:
        return MAV_PARAM_EXT_TYPE_INT32;
    }
}

QVariant QGCMAVLink::parseParamValue(const mavlink_message_t &message)
{
    mavlink_param_value_t param_value{};
    mavlink_msg_param_value_decode(&message, &param_value);

    mavlink_param_union_t paramUnion{};
    paramUnion.param_float = param_value.param_value;
    paramUnion.type = param_value.param_type;

    QVariant parameterValue;

    switch (paramUnion.type) {
    case MAV_PARAM_TYPE_REAL32:
        parameterValue = QVariant(paramUnion.param_float);
        break;
    case MAV_PARAM_TYPE_UINT8:
        parameterValue = QVariant(paramUnion.param_uint8);
        break;
    case MAV_PARAM_TYPE_INT8:
        parameterValue = QVariant(paramUnion.param_int8);
        break;
    case MAV_PARAM_TYPE_UINT16:
        parameterValue = QVariant(paramUnion.param_uint16);
        break;
    case MAV_PARAM_TYPE_INT16:
        parameterValue = QVariant(paramUnion.param_int16);
        break;
    case MAV_PARAM_TYPE_UINT32:
        parameterValue = QVariant(paramUnion.param_uint32);
        break;
    case MAV_PARAM_TYPE_INT32:
        parameterValue = QVariant(paramUnion.param_int32);
        break;
    default:
        qCCritical(QGCMAVLinkLog) << "unsupported MAV_PARAM_TYPE" << paramUnion.type;
        break;
    }

    return parameterValue;
}

mavlink_param_set_t QGCMAVLink::createParamSet(const mavlink_message_t &message)
{
    mavlink_param_set_t paramSet{};
    mavlink_param_union_t paramUnion{};

    mavlink_msg_param_set_decode(&message, &paramSet);

    paramUnion.param_float = paramSet.param_value;

    switch (paramSet.param_type) {
    case MAV_PARAM_TYPE_UINT8:
        paramSet.param_value = paramUnion.param_uint8;
        break;
    case MAV_PARAM_TYPE_INT8:
        paramSet.param_value = paramUnion.param_int8;
        break;
    case MAV_PARAM_TYPE_UINT16:
        paramSet.param_value = paramUnion.param_uint16;
        break;
    case MAV_PARAM_TYPE_INT16:
        paramSet.param_value = paramUnion.param_int16;
        break;
    case MAV_PARAM_TYPE_UINT32:
        paramSet.param_value = paramUnion.param_uint32;
        break;
    case MAV_PARAM_TYPE_INT32:
        paramSet.param_value = paramUnion.param_int32;
        break;
    case MAV_PARAM_TYPE_REAL32:
        // Already in param_float
        break;
    default:
        qCCritical(QGCMAVLinkLog) << "Invalid/Unsupported data type used in parameter:" << paramSet.param_type;
    }

    return paramSet;
}

#if 0
mavlink_param_ext_set_t QGCMAVLink::createParamExtSet(FactMetaData::ValueType_t factType)
{
    mavlink_param_ext_set_t p{};
    (void) memset(&p, 0, sizeof(mavlink_param_ext_set_t));
    p.param_type = _mavParamType;
    param_ext_union_t union_value{};
    const FactMetaData::ValueType_t factType = _fact->type();
    switch (factType) {
    case FactMetaData::valueTypeUint8:
    case FactMetaData::valueTypeBool:
        union_value.param_uint8 = static_cast<uint8_t>(_fact->rawValue().toUInt());
        break;
    case FactMetaData::valueTypeInt8:
        union_value.param_int8 = static_cast<int8_t>(_fact->rawValue().toInt());
        break;
    case FactMetaData::valueTypeUint16:
        union_value.param_uint16 = static_cast<uint16_t>(_fact->rawValue().toUInt());
        break;
    case FactMetaData::valueTypeInt16:
        union_value.param_int16 = static_cast<int16_t>(_fact->rawValue().toInt());
        break;
    case FactMetaData::valueTypeUint32:
        union_value.param_uint32 = static_cast<uint32_t>(_fact->rawValue().toUInt());
        break;
    case FactMetaData::valueTypeInt64:
        union_value.param_int64 = static_cast<int64_t>(_fact->rawValue().toLongLong());
        break;
    case FactMetaData::valueTypeUint64:
        union_value.param_uint64 = static_cast<uint64_t>(_fact->rawValue().toULongLong());
        break;
    case FactMetaData::valueTypeFloat:
        union_value.param_float = _fact->rawValue().toFloat();
        break;
    case FactMetaData::valueTypeDouble:
        union_value.param_double = _fact->rawValue().toDouble();
        break;
        //-- String and custom are the same for now
    case FactMetaData::valueTypeString:
    case FactMetaData::valueTypeCustom: {
        const QByteArray custom = _fact->rawValue().toByteArray();
        (void) memcpy(union_value.bytes, custom.data(), static_cast<size_t>(std::max(custom.size(), static_cast<qsizetype>(MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_VALUE_LEN))));
        break;
    }
    default:
        qCCritical() << "Unsupported fact type" << factType;
    case FactMetaData::valueTypeInt32:
        union_value.param_int32 = static_cast<int32_t>(_fact->rawValue().toInt());
        break;
    }
    (void) memcpy(&p.param_value[0], &union_value.bytes[0], MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_VALUE_LEN);
}
#endif

float QGCMAVLink::floatUnionForParam(MAV_PARAM_TYPE paramType, const QVariant &paramVar, MAV_AUTOPILOT firmwareType)
{
    mavlink_param_union_t valueUnion{};

    switch (paramType) {
    case MAV_PARAM_TYPE_REAL32:
        valueUnion.param_float = paramVar.toFloat();
        break;
    case MAV_PARAM_TYPE_UINT32:
        if (firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toUInt();
        } else {
            valueUnion.param_uint32 = paramVar.toUInt();
        }
        break;
    case MAV_PARAM_TYPE_INT32:
        if (firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toInt();
        } else {
            valueUnion.param_int32 = paramVar.toInt();
        }
        break;
    case MAV_PARAM_TYPE_UINT16:
        if (firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toUInt();
        } else {
            valueUnion.param_uint16 = paramVar.toUInt();
        }
        break;
    case MAV_PARAM_TYPE_INT16:
        if (firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toInt();
        } else {
            valueUnion.param_int16 = paramVar.toInt();
        }
        break;
    case MAV_PARAM_TYPE_UINT8:
        if (firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toUInt();
        } else {
            valueUnion.param_uint8 = paramVar.toUInt();
        }
        break;
    case MAV_PARAM_TYPE_INT8:
        if (firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = (unsigned char)paramVar.toChar().toLatin1();
        } else {
            valueUnion.param_int8 = (unsigned char)paramVar.toChar().toLatin1();
        }
        break;
    default:
        if (firmwareType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            valueUnion.param_float = paramVar.toInt();
        } else {
            valueUnion.param_int32 = paramVar.toInt();
        }
        qCCritical(QGCMAVLinkLog) << "Invalid parameter type" << paramType;
    }

    return valueUnion.param_float;
}
