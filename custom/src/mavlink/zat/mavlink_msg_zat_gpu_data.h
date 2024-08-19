#pragma once
// MESSAGE ZAT_GPU_DATA PACKING

#define MAVLINK_MSG_ID_ZAT_GPU_DATA 20000


typedef struct __mavlink_zat_gpu_data_t {
 int32_t latitude; /*<  GPS Latitude.*/
 int32_t longitude; /*<  GPS Longitude.*/
 float altitude; /*<  GPS Altitude.*/
 float psuVoltage; /*<  PSU Voltage.*/
 float heading; /*<  GPS Heading.*/
 uint16_t pressure; /*<  Pressure.*/
 uint16_t outputVoltage; /*<  Output Voltage.*/
 int16_t motorRpm; /*<  Motor RPM.*/
 int8_t psuTemperature; /*<  PSU Temperature.*/
 int8_t spoolTemperature; /*<  Spool Temperature.*/
 uint8_t psuFanDuty; /*<  PSU Fan Duty.*/
 uint8_t spoolFanDuty; /*<  Spool Fan Duty.*/
 uint8_t motorDuty; /*<  Motor Duty.*/
 int8_t motorDutyOffset; /*<  Motor Duty Offset.*/
 int8_t tetherTension; /*<  Tether Tension.*/
 uint8_t tetherLength; /*<  Tether Length.*/
 int8_t pressureTemperature; /*<  Pressure Temperature.*/
 uint8_t psuHighPower; /*<  PSU High Power.*/
 uint8_t satsInUse; /*<  GPS Satellites In Use.*/
 uint8_t gpsFixType; /*<  GPS Fix Type.*/
 uint8_t gpsFixMode; /*<  GPS Fix Mode.*/
 uint8_t gpsIsValid; /*<  GPS Is Valid.*/
} mavlink_zat_gpu_data_t;

#define MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN 40
#define MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN 40
#define MAVLINK_MSG_ID_20000_LEN 40
#define MAVLINK_MSG_ID_20000_MIN_LEN 40

#define MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC 22
#define MAVLINK_MSG_ID_20000_CRC 22



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_ZAT_GPU_DATA { \
    20000, \
    "ZAT_GPU_DATA", \
    22, \
    {  { "psuTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 26, offsetof(mavlink_zat_gpu_data_t, psuTemperature) }, \
         { "spoolTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 27, offsetof(mavlink_zat_gpu_data_t, spoolTemperature) }, \
         { "psuFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_zat_gpu_data_t, psuFanDuty) }, \
         { "spoolFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_zat_gpu_data_t, spoolFanDuty) }, \
         { "motorDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_zat_gpu_data_t, motorDuty) }, \
         { "motorDutyOffset", NULL, MAVLINK_TYPE_INT8_T, 0, 31, offsetof(mavlink_zat_gpu_data_t, motorDutyOffset) }, \
         { "tetherTension", NULL, MAVLINK_TYPE_INT8_T, 0, 32, offsetof(mavlink_zat_gpu_data_t, tetherTension) }, \
         { "tetherLength", NULL, MAVLINK_TYPE_UINT8_T, 0, 33, offsetof(mavlink_zat_gpu_data_t, tetherLength) }, \
         { "latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_zat_gpu_data_t, latitude) }, \
         { "longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_zat_gpu_data_t, longitude) }, \
         { "altitude", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_zat_gpu_data_t, altitude) }, \
         { "pressure", NULL, MAVLINK_TYPE_UINT16_T, 0, 20, offsetof(mavlink_zat_gpu_data_t, pressure) }, \
         { "pressureTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 34, offsetof(mavlink_zat_gpu_data_t, pressureTemperature) }, \
         { "outputVoltage", NULL, MAVLINK_TYPE_UINT16_T, 0, 22, offsetof(mavlink_zat_gpu_data_t, outputVoltage) }, \
         { "psuHighPower", NULL, MAVLINK_TYPE_UINT8_T, 0, 35, offsetof(mavlink_zat_gpu_data_t, psuHighPower) }, \
         { "psuVoltage", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_zat_gpu_data_t, psuVoltage) }, \
         { "motorRpm", NULL, MAVLINK_TYPE_INT16_T, 0, 24, offsetof(mavlink_zat_gpu_data_t, motorRpm) }, \
         { "heading", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_zat_gpu_data_t, heading) }, \
         { "satsInUse", NULL, MAVLINK_TYPE_UINT8_T, 0, 36, offsetof(mavlink_zat_gpu_data_t, satsInUse) }, \
         { "gpsFixType", NULL, MAVLINK_TYPE_UINT8_T, 0, 37, offsetof(mavlink_zat_gpu_data_t, gpsFixType) }, \
         { "gpsFixMode", NULL, MAVLINK_TYPE_UINT8_T, 0, 38, offsetof(mavlink_zat_gpu_data_t, gpsFixMode) }, \
         { "gpsIsValid", NULL, MAVLINK_TYPE_UINT8_T, 0, 39, offsetof(mavlink_zat_gpu_data_t, gpsIsValid) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_ZAT_GPU_DATA { \
    "ZAT_GPU_DATA", \
    22, \
    {  { "psuTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 26, offsetof(mavlink_zat_gpu_data_t, psuTemperature) }, \
         { "spoolTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 27, offsetof(mavlink_zat_gpu_data_t, spoolTemperature) }, \
         { "psuFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 28, offsetof(mavlink_zat_gpu_data_t, psuFanDuty) }, \
         { "spoolFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 29, offsetof(mavlink_zat_gpu_data_t, spoolFanDuty) }, \
         { "motorDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 30, offsetof(mavlink_zat_gpu_data_t, motorDuty) }, \
         { "motorDutyOffset", NULL, MAVLINK_TYPE_INT8_T, 0, 31, offsetof(mavlink_zat_gpu_data_t, motorDutyOffset) }, \
         { "tetherTension", NULL, MAVLINK_TYPE_INT8_T, 0, 32, offsetof(mavlink_zat_gpu_data_t, tetherTension) }, \
         { "tetherLength", NULL, MAVLINK_TYPE_UINT8_T, 0, 33, offsetof(mavlink_zat_gpu_data_t, tetherLength) }, \
         { "latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_zat_gpu_data_t, latitude) }, \
         { "longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_zat_gpu_data_t, longitude) }, \
         { "altitude", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_zat_gpu_data_t, altitude) }, \
         { "pressure", NULL, MAVLINK_TYPE_UINT16_T, 0, 20, offsetof(mavlink_zat_gpu_data_t, pressure) }, \
         { "pressureTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 34, offsetof(mavlink_zat_gpu_data_t, pressureTemperature) }, \
         { "outputVoltage", NULL, MAVLINK_TYPE_UINT16_T, 0, 22, offsetof(mavlink_zat_gpu_data_t, outputVoltage) }, \
         { "psuHighPower", NULL, MAVLINK_TYPE_UINT8_T, 0, 35, offsetof(mavlink_zat_gpu_data_t, psuHighPower) }, \
         { "psuVoltage", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_zat_gpu_data_t, psuVoltage) }, \
         { "motorRpm", NULL, MAVLINK_TYPE_INT16_T, 0, 24, offsetof(mavlink_zat_gpu_data_t, motorRpm) }, \
         { "heading", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_zat_gpu_data_t, heading) }, \
         { "satsInUse", NULL, MAVLINK_TYPE_UINT8_T, 0, 36, offsetof(mavlink_zat_gpu_data_t, satsInUse) }, \
         { "gpsFixType", NULL, MAVLINK_TYPE_UINT8_T, 0, 37, offsetof(mavlink_zat_gpu_data_t, gpsFixType) }, \
         { "gpsFixMode", NULL, MAVLINK_TYPE_UINT8_T, 0, 38, offsetof(mavlink_zat_gpu_data_t, gpsFixMode) }, \
         { "gpsIsValid", NULL, MAVLINK_TYPE_UINT8_T, 0, 39, offsetof(mavlink_zat_gpu_data_t, gpsIsValid) }, \
         } \
}
#endif

/**
 * @brief Pack a zat_gpu_data message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param psuTemperature  PSU Temperature.
 * @param spoolTemperature  Spool Temperature.
 * @param psuFanDuty  PSU Fan Duty.
 * @param spoolFanDuty  Spool Fan Duty.
 * @param motorDuty  Motor Duty.
 * @param motorDutyOffset  Motor Duty Offset.
 * @param tetherTension  Tether Tension.
 * @param tetherLength  Tether Length.
 * @param latitude  GPS Latitude.
 * @param longitude  GPS Longitude.
 * @param altitude  GPS Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 * @param psuHighPower  PSU High Power.
 * @param psuVoltage  PSU Voltage.
 * @param motorRpm  Motor RPM.
 * @param heading  GPS Heading.
 * @param satsInUse  GPS Satellites In Use.
 * @param gpsFixType  GPS Fix Type.
 * @param gpsFixMode  GPS Fix Mode.
 * @param gpsIsValid  GPS Is Valid.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_zat_gpu_data_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage, uint8_t psuHighPower, float psuVoltage, int16_t motorRpm, float heading, uint8_t satsInUse, uint8_t gpsFixType, uint8_t gpsFixMode, uint8_t gpsIsValid)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_float(buf, 12, psuVoltage);
    _mav_put_float(buf, 16, heading);
    _mav_put_uint16_t(buf, 20, pressure);
    _mav_put_uint16_t(buf, 22, outputVoltage);
    _mav_put_int16_t(buf, 24, motorRpm);
    _mav_put_int8_t(buf, 26, psuTemperature);
    _mav_put_int8_t(buf, 27, spoolTemperature);
    _mav_put_uint8_t(buf, 28, psuFanDuty);
    _mav_put_uint8_t(buf, 29, spoolFanDuty);
    _mav_put_uint8_t(buf, 30, motorDuty);
    _mav_put_int8_t(buf, 31, motorDutyOffset);
    _mav_put_int8_t(buf, 32, tetherTension);
    _mav_put_uint8_t(buf, 33, tetherLength);
    _mav_put_int8_t(buf, 34, pressureTemperature);
    _mav_put_uint8_t(buf, 35, psuHighPower);
    _mav_put_uint8_t(buf, 36, satsInUse);
    _mav_put_uint8_t(buf, 37, gpsFixType);
    _mav_put_uint8_t(buf, 38, gpsFixMode);
    _mav_put_uint8_t(buf, 39, gpsIsValid);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.psuVoltage = psuVoltage;
    packet.heading = heading;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.motorRpm = motorRpm;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;
    packet.psuHighPower = psuHighPower;
    packet.satsInUse = satsInUse;
    packet.gpsFixType = gpsFixType;
    packet.gpsFixMode = gpsFixMode;
    packet.gpsIsValid = gpsIsValid;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_ZAT_GPU_DATA;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
}

/**
 * @brief Pack a zat_gpu_data message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 *
 * @param psuTemperature  PSU Temperature.
 * @param spoolTemperature  Spool Temperature.
 * @param psuFanDuty  PSU Fan Duty.
 * @param spoolFanDuty  Spool Fan Duty.
 * @param motorDuty  Motor Duty.
 * @param motorDutyOffset  Motor Duty Offset.
 * @param tetherTension  Tether Tension.
 * @param tetherLength  Tether Length.
 * @param latitude  GPS Latitude.
 * @param longitude  GPS Longitude.
 * @param altitude  GPS Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 * @param psuHighPower  PSU High Power.
 * @param psuVoltage  PSU Voltage.
 * @param motorRpm  Motor RPM.
 * @param heading  GPS Heading.
 * @param satsInUse  GPS Satellites In Use.
 * @param gpsFixType  GPS Fix Type.
 * @param gpsFixMode  GPS Fix Mode.
 * @param gpsIsValid  GPS Is Valid.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_zat_gpu_data_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage, uint8_t psuHighPower, float psuVoltage, int16_t motorRpm, float heading, uint8_t satsInUse, uint8_t gpsFixType, uint8_t gpsFixMode, uint8_t gpsIsValid)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_float(buf, 12, psuVoltage);
    _mav_put_float(buf, 16, heading);
    _mav_put_uint16_t(buf, 20, pressure);
    _mav_put_uint16_t(buf, 22, outputVoltage);
    _mav_put_int16_t(buf, 24, motorRpm);
    _mav_put_int8_t(buf, 26, psuTemperature);
    _mav_put_int8_t(buf, 27, spoolTemperature);
    _mav_put_uint8_t(buf, 28, psuFanDuty);
    _mav_put_uint8_t(buf, 29, spoolFanDuty);
    _mav_put_uint8_t(buf, 30, motorDuty);
    _mav_put_int8_t(buf, 31, motorDutyOffset);
    _mav_put_int8_t(buf, 32, tetherTension);
    _mav_put_uint8_t(buf, 33, tetherLength);
    _mav_put_int8_t(buf, 34, pressureTemperature);
    _mav_put_uint8_t(buf, 35, psuHighPower);
    _mav_put_uint8_t(buf, 36, satsInUse);
    _mav_put_uint8_t(buf, 37, gpsFixType);
    _mav_put_uint8_t(buf, 38, gpsFixMode);
    _mav_put_uint8_t(buf, 39, gpsIsValid);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.psuVoltage = psuVoltage;
    packet.heading = heading;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.motorRpm = motorRpm;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;
    packet.psuHighPower = psuHighPower;
    packet.satsInUse = satsInUse;
    packet.gpsFixType = gpsFixType;
    packet.gpsFixMode = gpsFixMode;
    packet.gpsIsValid = gpsIsValid;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_ZAT_GPU_DATA;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#else
    return mavlink_finalize_message_buffer(msg, system_id, component_id, _status, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#endif
}

/**
 * @brief Pack a zat_gpu_data message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param psuTemperature  PSU Temperature.
 * @param spoolTemperature  Spool Temperature.
 * @param psuFanDuty  PSU Fan Duty.
 * @param spoolFanDuty  Spool Fan Duty.
 * @param motorDuty  Motor Duty.
 * @param motorDutyOffset  Motor Duty Offset.
 * @param tetherTension  Tether Tension.
 * @param tetherLength  Tether Length.
 * @param latitude  GPS Latitude.
 * @param longitude  GPS Longitude.
 * @param altitude  GPS Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 * @param psuHighPower  PSU High Power.
 * @param psuVoltage  PSU Voltage.
 * @param motorRpm  Motor RPM.
 * @param heading  GPS Heading.
 * @param satsInUse  GPS Satellites In Use.
 * @param gpsFixType  GPS Fix Type.
 * @param gpsFixMode  GPS Fix Mode.
 * @param gpsIsValid  GPS Is Valid.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_zat_gpu_data_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int8_t psuTemperature,int8_t spoolTemperature,uint8_t psuFanDuty,uint8_t spoolFanDuty,uint8_t motorDuty,int8_t motorDutyOffset,int8_t tetherTension,uint8_t tetherLength,int32_t latitude,int32_t longitude,float altitude,uint16_t pressure,int8_t pressureTemperature,uint16_t outputVoltage,uint8_t psuHighPower,float psuVoltage,int16_t motorRpm,float heading,uint8_t satsInUse,uint8_t gpsFixType,uint8_t gpsFixMode,uint8_t gpsIsValid)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_float(buf, 12, psuVoltage);
    _mav_put_float(buf, 16, heading);
    _mav_put_uint16_t(buf, 20, pressure);
    _mav_put_uint16_t(buf, 22, outputVoltage);
    _mav_put_int16_t(buf, 24, motorRpm);
    _mav_put_int8_t(buf, 26, psuTemperature);
    _mav_put_int8_t(buf, 27, spoolTemperature);
    _mav_put_uint8_t(buf, 28, psuFanDuty);
    _mav_put_uint8_t(buf, 29, spoolFanDuty);
    _mav_put_uint8_t(buf, 30, motorDuty);
    _mav_put_int8_t(buf, 31, motorDutyOffset);
    _mav_put_int8_t(buf, 32, tetherTension);
    _mav_put_uint8_t(buf, 33, tetherLength);
    _mav_put_int8_t(buf, 34, pressureTemperature);
    _mav_put_uint8_t(buf, 35, psuHighPower);
    _mav_put_uint8_t(buf, 36, satsInUse);
    _mav_put_uint8_t(buf, 37, gpsFixType);
    _mav_put_uint8_t(buf, 38, gpsFixMode);
    _mav_put_uint8_t(buf, 39, gpsIsValid);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.psuVoltage = psuVoltage;
    packet.heading = heading;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.motorRpm = motorRpm;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;
    packet.psuHighPower = psuHighPower;
    packet.satsInUse = satsInUse;
    packet.gpsFixType = gpsFixType;
    packet.gpsFixMode = gpsFixMode;
    packet.gpsIsValid = gpsIsValid;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_ZAT_GPU_DATA;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
}

/**
 * @brief Encode a zat_gpu_data struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param zat_gpu_data C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_zat_gpu_data_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_zat_gpu_data_t* zat_gpu_data)
{
    return mavlink_msg_zat_gpu_data_pack(system_id, component_id, msg, zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage, zat_gpu_data->psuHighPower, zat_gpu_data->psuVoltage, zat_gpu_data->motorRpm, zat_gpu_data->heading, zat_gpu_data->satsInUse, zat_gpu_data->gpsFixType, zat_gpu_data->gpsFixMode, zat_gpu_data->gpsIsValid);
}

/**
 * @brief Encode a zat_gpu_data struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param zat_gpu_data C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_zat_gpu_data_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_zat_gpu_data_t* zat_gpu_data)
{
    return mavlink_msg_zat_gpu_data_pack_chan(system_id, component_id, chan, msg, zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage, zat_gpu_data->psuHighPower, zat_gpu_data->psuVoltage, zat_gpu_data->motorRpm, zat_gpu_data->heading, zat_gpu_data->satsInUse, zat_gpu_data->gpsFixType, zat_gpu_data->gpsFixMode, zat_gpu_data->gpsIsValid);
}

/**
 * @brief Encode a zat_gpu_data struct with provided status structure
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param status MAVLink status structure
 * @param msg The MAVLink message to compress the data into
 * @param zat_gpu_data C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_zat_gpu_data_encode_status(uint8_t system_id, uint8_t component_id, mavlink_status_t* _status, mavlink_message_t* msg, const mavlink_zat_gpu_data_t* zat_gpu_data)
{
    return mavlink_msg_zat_gpu_data_pack_status(system_id, component_id, _status, msg,  zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage, zat_gpu_data->psuHighPower, zat_gpu_data->psuVoltage, zat_gpu_data->motorRpm, zat_gpu_data->heading, zat_gpu_data->satsInUse, zat_gpu_data->gpsFixType, zat_gpu_data->gpsFixMode, zat_gpu_data->gpsIsValid);
}

/**
 * @brief Send a zat_gpu_data message
 * @param chan MAVLink channel to send the message
 *
 * @param psuTemperature  PSU Temperature.
 * @param spoolTemperature  Spool Temperature.
 * @param psuFanDuty  PSU Fan Duty.
 * @param spoolFanDuty  Spool Fan Duty.
 * @param motorDuty  Motor Duty.
 * @param motorDutyOffset  Motor Duty Offset.
 * @param tetherTension  Tether Tension.
 * @param tetherLength  Tether Length.
 * @param latitude  GPS Latitude.
 * @param longitude  GPS Longitude.
 * @param altitude  GPS Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 * @param psuHighPower  PSU High Power.
 * @param psuVoltage  PSU Voltage.
 * @param motorRpm  Motor RPM.
 * @param heading  GPS Heading.
 * @param satsInUse  GPS Satellites In Use.
 * @param gpsFixType  GPS Fix Type.
 * @param gpsFixMode  GPS Fix Mode.
 * @param gpsIsValid  GPS Is Valid.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_zat_gpu_data_send(mavlink_channel_t chan, int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage, uint8_t psuHighPower, float psuVoltage, int16_t motorRpm, float heading, uint8_t satsInUse, uint8_t gpsFixType, uint8_t gpsFixMode, uint8_t gpsIsValid)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_float(buf, 12, psuVoltage);
    _mav_put_float(buf, 16, heading);
    _mav_put_uint16_t(buf, 20, pressure);
    _mav_put_uint16_t(buf, 22, outputVoltage);
    _mav_put_int16_t(buf, 24, motorRpm);
    _mav_put_int8_t(buf, 26, psuTemperature);
    _mav_put_int8_t(buf, 27, spoolTemperature);
    _mav_put_uint8_t(buf, 28, psuFanDuty);
    _mav_put_uint8_t(buf, 29, spoolFanDuty);
    _mav_put_uint8_t(buf, 30, motorDuty);
    _mav_put_int8_t(buf, 31, motorDutyOffset);
    _mav_put_int8_t(buf, 32, tetherTension);
    _mav_put_uint8_t(buf, 33, tetherLength);
    _mav_put_int8_t(buf, 34, pressureTemperature);
    _mav_put_uint8_t(buf, 35, psuHighPower);
    _mav_put_uint8_t(buf, 36, satsInUse);
    _mav_put_uint8_t(buf, 37, gpsFixType);
    _mav_put_uint8_t(buf, 38, gpsFixMode);
    _mav_put_uint8_t(buf, 39, gpsIsValid);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ZAT_GPU_DATA, buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.psuVoltage = psuVoltage;
    packet.heading = heading;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.motorRpm = motorRpm;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;
    packet.psuHighPower = psuHighPower;
    packet.satsInUse = satsInUse;
    packet.gpsFixType = gpsFixType;
    packet.gpsFixMode = gpsFixMode;
    packet.gpsIsValid = gpsIsValid;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ZAT_GPU_DATA, (const char *)&packet, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#endif
}

/**
 * @brief Send a zat_gpu_data message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_zat_gpu_data_send_struct(mavlink_channel_t chan, const mavlink_zat_gpu_data_t* zat_gpu_data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_zat_gpu_data_send(chan, zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage, zat_gpu_data->psuHighPower, zat_gpu_data->psuVoltage, zat_gpu_data->motorRpm, zat_gpu_data->heading, zat_gpu_data->satsInUse, zat_gpu_data->gpsFixType, zat_gpu_data->gpsFixMode, zat_gpu_data->gpsIsValid);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ZAT_GPU_DATA, (const char *)zat_gpu_data, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#endif
}

#if MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This variant of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_zat_gpu_data_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage, uint8_t psuHighPower, float psuVoltage, int16_t motorRpm, float heading, uint8_t satsInUse, uint8_t gpsFixType, uint8_t gpsFixMode, uint8_t gpsIsValid)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_float(buf, 12, psuVoltage);
    _mav_put_float(buf, 16, heading);
    _mav_put_uint16_t(buf, 20, pressure);
    _mav_put_uint16_t(buf, 22, outputVoltage);
    _mav_put_int16_t(buf, 24, motorRpm);
    _mav_put_int8_t(buf, 26, psuTemperature);
    _mav_put_int8_t(buf, 27, spoolTemperature);
    _mav_put_uint8_t(buf, 28, psuFanDuty);
    _mav_put_uint8_t(buf, 29, spoolFanDuty);
    _mav_put_uint8_t(buf, 30, motorDuty);
    _mav_put_int8_t(buf, 31, motorDutyOffset);
    _mav_put_int8_t(buf, 32, tetherTension);
    _mav_put_uint8_t(buf, 33, tetherLength);
    _mav_put_int8_t(buf, 34, pressureTemperature);
    _mav_put_uint8_t(buf, 35, psuHighPower);
    _mav_put_uint8_t(buf, 36, satsInUse);
    _mav_put_uint8_t(buf, 37, gpsFixType);
    _mav_put_uint8_t(buf, 38, gpsFixMode);
    _mav_put_uint8_t(buf, 39, gpsIsValid);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ZAT_GPU_DATA, buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#else
    mavlink_zat_gpu_data_t *packet = (mavlink_zat_gpu_data_t *)msgbuf;
    packet->latitude = latitude;
    packet->longitude = longitude;
    packet->altitude = altitude;
    packet->psuVoltage = psuVoltage;
    packet->heading = heading;
    packet->pressure = pressure;
    packet->outputVoltage = outputVoltage;
    packet->motorRpm = motorRpm;
    packet->psuTemperature = psuTemperature;
    packet->spoolTemperature = spoolTemperature;
    packet->psuFanDuty = psuFanDuty;
    packet->spoolFanDuty = spoolFanDuty;
    packet->motorDuty = motorDuty;
    packet->motorDutyOffset = motorDutyOffset;
    packet->tetherTension = tetherTension;
    packet->tetherLength = tetherLength;
    packet->pressureTemperature = pressureTemperature;
    packet->psuHighPower = psuHighPower;
    packet->satsInUse = satsInUse;
    packet->gpsFixType = gpsFixType;
    packet->gpsFixMode = gpsFixMode;
    packet->gpsIsValid = gpsIsValid;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ZAT_GPU_DATA, (const char *)packet, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#endif
}
#endif

#endif

// MESSAGE ZAT_GPU_DATA UNPACKING


/**
 * @brief Get field psuTemperature from zat_gpu_data message
 *
 * @return  PSU Temperature.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_psuTemperature(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  26);
}

/**
 * @brief Get field spoolTemperature from zat_gpu_data message
 *
 * @return  Spool Temperature.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_spoolTemperature(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  27);
}

/**
 * @brief Get field psuFanDuty from zat_gpu_data message
 *
 * @return  PSU Fan Duty.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_psuFanDuty(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  28);
}

/**
 * @brief Get field spoolFanDuty from zat_gpu_data message
 *
 * @return  Spool Fan Duty.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_spoolFanDuty(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  29);
}

/**
 * @brief Get field motorDuty from zat_gpu_data message
 *
 * @return  Motor Duty.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_motorDuty(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  30);
}

/**
 * @brief Get field motorDutyOffset from zat_gpu_data message
 *
 * @return  Motor Duty Offset.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_motorDutyOffset(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  31);
}

/**
 * @brief Get field tetherTension from zat_gpu_data message
 *
 * @return  Tether Tension.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_tetherTension(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  32);
}

/**
 * @brief Get field tetherLength from zat_gpu_data message
 *
 * @return  Tether Length.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_tetherLength(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  33);
}

/**
 * @brief Get field latitude from zat_gpu_data message
 *
 * @return  GPS Latitude.
 */
static inline int32_t mavlink_msg_zat_gpu_data_get_latitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field longitude from zat_gpu_data message
 *
 * @return  GPS Longitude.
 */
static inline int32_t mavlink_msg_zat_gpu_data_get_longitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field altitude from zat_gpu_data message
 *
 * @return  GPS Altitude.
 */
static inline float mavlink_msg_zat_gpu_data_get_altitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field pressure from zat_gpu_data message
 *
 * @return  Pressure.
 */
static inline uint16_t mavlink_msg_zat_gpu_data_get_pressure(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  20);
}

/**
 * @brief Get field pressureTemperature from zat_gpu_data message
 *
 * @return  Pressure Temperature.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_pressureTemperature(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  34);
}

/**
 * @brief Get field outputVoltage from zat_gpu_data message
 *
 * @return  Output Voltage.
 */
static inline uint16_t mavlink_msg_zat_gpu_data_get_outputVoltage(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  22);
}

/**
 * @brief Get field psuHighPower from zat_gpu_data message
 *
 * @return  PSU High Power.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_psuHighPower(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  35);
}

/**
 * @brief Get field psuVoltage from zat_gpu_data message
 *
 * @return  PSU Voltage.
 */
static inline float mavlink_msg_zat_gpu_data_get_psuVoltage(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Get field motorRpm from zat_gpu_data message
 *
 * @return  Motor RPM.
 */
static inline int16_t mavlink_msg_zat_gpu_data_get_motorRpm(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  24);
}

/**
 * @brief Get field heading from zat_gpu_data message
 *
 * @return  GPS Heading.
 */
static inline float mavlink_msg_zat_gpu_data_get_heading(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  16);
}

/**
 * @brief Get field satsInUse from zat_gpu_data message
 *
 * @return  GPS Satellites In Use.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_satsInUse(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  36);
}

/**
 * @brief Get field gpsFixType from zat_gpu_data message
 *
 * @return  GPS Fix Type.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_gpsFixType(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  37);
}

/**
 * @brief Get field gpsFixMode from zat_gpu_data message
 *
 * @return  GPS Fix Mode.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_gpsFixMode(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  38);
}

/**
 * @brief Get field gpsIsValid from zat_gpu_data message
 *
 * @return  GPS Is Valid.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_gpsIsValid(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  39);
}

/**
 * @brief Decode a zat_gpu_data message into a struct
 *
 * @param msg The message to decode
 * @param zat_gpu_data C-struct to decode the message contents into
 */
static inline void mavlink_msg_zat_gpu_data_decode(const mavlink_message_t* msg, mavlink_zat_gpu_data_t* zat_gpu_data)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    zat_gpu_data->latitude = mavlink_msg_zat_gpu_data_get_latitude(msg);
    zat_gpu_data->longitude = mavlink_msg_zat_gpu_data_get_longitude(msg);
    zat_gpu_data->altitude = mavlink_msg_zat_gpu_data_get_altitude(msg);
    zat_gpu_data->psuVoltage = mavlink_msg_zat_gpu_data_get_psuVoltage(msg);
    zat_gpu_data->heading = mavlink_msg_zat_gpu_data_get_heading(msg);
    zat_gpu_data->pressure = mavlink_msg_zat_gpu_data_get_pressure(msg);
    zat_gpu_data->outputVoltage = mavlink_msg_zat_gpu_data_get_outputVoltage(msg);
    zat_gpu_data->motorRpm = mavlink_msg_zat_gpu_data_get_motorRpm(msg);
    zat_gpu_data->psuTemperature = mavlink_msg_zat_gpu_data_get_psuTemperature(msg);
    zat_gpu_data->spoolTemperature = mavlink_msg_zat_gpu_data_get_spoolTemperature(msg);
    zat_gpu_data->psuFanDuty = mavlink_msg_zat_gpu_data_get_psuFanDuty(msg);
    zat_gpu_data->spoolFanDuty = mavlink_msg_zat_gpu_data_get_spoolFanDuty(msg);
    zat_gpu_data->motorDuty = mavlink_msg_zat_gpu_data_get_motorDuty(msg);
    zat_gpu_data->motorDutyOffset = mavlink_msg_zat_gpu_data_get_motorDutyOffset(msg);
    zat_gpu_data->tetherTension = mavlink_msg_zat_gpu_data_get_tetherTension(msg);
    zat_gpu_data->tetherLength = mavlink_msg_zat_gpu_data_get_tetherLength(msg);
    zat_gpu_data->pressureTemperature = mavlink_msg_zat_gpu_data_get_pressureTemperature(msg);
    zat_gpu_data->psuHighPower = mavlink_msg_zat_gpu_data_get_psuHighPower(msg);
    zat_gpu_data->satsInUse = mavlink_msg_zat_gpu_data_get_satsInUse(msg);
    zat_gpu_data->gpsFixType = mavlink_msg_zat_gpu_data_get_gpsFixType(msg);
    zat_gpu_data->gpsFixMode = mavlink_msg_zat_gpu_data_get_gpsFixMode(msg);
    zat_gpu_data->gpsIsValid = mavlink_msg_zat_gpu_data_get_gpsIsValid(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN? msg->len : MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN;
        memset(zat_gpu_data, 0, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
    memcpy(zat_gpu_data, _MAV_PAYLOAD(msg), len);
#endif
}
