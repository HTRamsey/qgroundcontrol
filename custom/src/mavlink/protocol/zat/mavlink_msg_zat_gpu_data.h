#pragma once
// MESSAGE ZAT_GPU_DATA PACKING

#define MAVLINK_MSG_ID_ZAT_GPU_DATA 20000


typedef struct __mavlink_zat_gpu_data_t {
 int32_t latitude; /*<  Latitude.*/
 int32_t longitude; /*<  Longitude.*/
 float altitude; /*<  Altitude.*/
 uint16_t pressure; /*<  Pressure.*/
 uint16_t outputVoltage; /*<  Output Voltage.*/
 int8_t psuTemperature; /*<  PSU Temperature.*/
 int8_t spoolTemperature; /*<  Spool Temperature.*/
 uint8_t psuFanDuty; /*<  PSU Fan Duty.*/
 uint8_t spoolFanDuty; /*<  Spool Fan Duty.*/
 uint8_t motorDuty; /*<  Motor Duty.*/
 int8_t motorDutyOffset; /*<  Motor Duty Offset.*/
 int8_t tetherTension; /*<  Tether Tension.*/
 uint8_t tetherLength; /*<  Tether Length.*/
 int8_t pressureTemperature; /*<  Pressure Temperature.*/
} mavlink_zat_gpu_data_t;

#define MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN 25
#define MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN 25
#define MAVLINK_MSG_ID_20000_LEN 25
#define MAVLINK_MSG_ID_20000_MIN_LEN 25

#define MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC 99
#define MAVLINK_MSG_ID_20000_CRC 99



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_ZAT_GPU_DATA { \
    20000, \
    "ZAT_GPU_DATA", \
    14, \
    {  { "psuTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 16, offsetof(mavlink_zat_gpu_data_t, psuTemperature) }, \
         { "spoolTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 17, offsetof(mavlink_zat_gpu_data_t, spoolTemperature) }, \
         { "psuFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_zat_gpu_data_t, psuFanDuty) }, \
         { "spoolFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 19, offsetof(mavlink_zat_gpu_data_t, spoolFanDuty) }, \
         { "motorDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_zat_gpu_data_t, motorDuty) }, \
         { "motorDutyOffset", NULL, MAVLINK_TYPE_INT8_T, 0, 21, offsetof(mavlink_zat_gpu_data_t, motorDutyOffset) }, \
         { "tetherTension", NULL, MAVLINK_TYPE_INT8_T, 0, 22, offsetof(mavlink_zat_gpu_data_t, tetherTension) }, \
         { "tetherLength", NULL, MAVLINK_TYPE_UINT8_T, 0, 23, offsetof(mavlink_zat_gpu_data_t, tetherLength) }, \
         { "latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_zat_gpu_data_t, latitude) }, \
         { "longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_zat_gpu_data_t, longitude) }, \
         { "altitude", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_zat_gpu_data_t, altitude) }, \
         { "pressure", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_zat_gpu_data_t, pressure) }, \
         { "pressureTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 24, offsetof(mavlink_zat_gpu_data_t, pressureTemperature) }, \
         { "outputVoltage", NULL, MAVLINK_TYPE_UINT16_T, 0, 14, offsetof(mavlink_zat_gpu_data_t, outputVoltage) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_ZAT_GPU_DATA { \
    "ZAT_GPU_DATA", \
    14, \
    {  { "psuTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 16, offsetof(mavlink_zat_gpu_data_t, psuTemperature) }, \
         { "spoolTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 17, offsetof(mavlink_zat_gpu_data_t, spoolTemperature) }, \
         { "psuFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 18, offsetof(mavlink_zat_gpu_data_t, psuFanDuty) }, \
         { "spoolFanDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 19, offsetof(mavlink_zat_gpu_data_t, spoolFanDuty) }, \
         { "motorDuty", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_zat_gpu_data_t, motorDuty) }, \
         { "motorDutyOffset", NULL, MAVLINK_TYPE_INT8_T, 0, 21, offsetof(mavlink_zat_gpu_data_t, motorDutyOffset) }, \
         { "tetherTension", NULL, MAVLINK_TYPE_INT8_T, 0, 22, offsetof(mavlink_zat_gpu_data_t, tetherTension) }, \
         { "tetherLength", NULL, MAVLINK_TYPE_UINT8_T, 0, 23, offsetof(mavlink_zat_gpu_data_t, tetherLength) }, \
         { "latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_zat_gpu_data_t, latitude) }, \
         { "longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_zat_gpu_data_t, longitude) }, \
         { "altitude", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_zat_gpu_data_t, altitude) }, \
         { "pressure", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_zat_gpu_data_t, pressure) }, \
         { "pressureTemperature", NULL, MAVLINK_TYPE_INT8_T, 0, 24, offsetof(mavlink_zat_gpu_data_t, pressureTemperature) }, \
         { "outputVoltage", NULL, MAVLINK_TYPE_UINT16_T, 0, 14, offsetof(mavlink_zat_gpu_data_t, outputVoltage) }, \
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
 * @param latitude  Latitude.
 * @param longitude  Longitude.
 * @param altitude  Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_zat_gpu_data_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_uint16_t(buf, 12, pressure);
    _mav_put_uint16_t(buf, 14, outputVoltage);
    _mav_put_int8_t(buf, 16, psuTemperature);
    _mav_put_int8_t(buf, 17, spoolTemperature);
    _mav_put_uint8_t(buf, 18, psuFanDuty);
    _mav_put_uint8_t(buf, 19, spoolFanDuty);
    _mav_put_uint8_t(buf, 20, motorDuty);
    _mav_put_int8_t(buf, 21, motorDutyOffset);
    _mav_put_int8_t(buf, 22, tetherTension);
    _mav_put_uint8_t(buf, 23, tetherLength);
    _mav_put_int8_t(buf, 24, pressureTemperature);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;

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
 * @param latitude  Latitude.
 * @param longitude  Longitude.
 * @param altitude  Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_zat_gpu_data_pack_status(uint8_t system_id, uint8_t component_id, mavlink_status_t *_status, mavlink_message_t* msg,
                               int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_uint16_t(buf, 12, pressure);
    _mav_put_uint16_t(buf, 14, outputVoltage);
    _mav_put_int8_t(buf, 16, psuTemperature);
    _mav_put_int8_t(buf, 17, spoolTemperature);
    _mav_put_uint8_t(buf, 18, psuFanDuty);
    _mav_put_uint8_t(buf, 19, spoolFanDuty);
    _mav_put_uint8_t(buf, 20, motorDuty);
    _mav_put_int8_t(buf, 21, motorDutyOffset);
    _mav_put_int8_t(buf, 22, tetherTension);
    _mav_put_uint8_t(buf, 23, tetherLength);
    _mav_put_int8_t(buf, 24, pressureTemperature);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;

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
 * @param latitude  Latitude.
 * @param longitude  Longitude.
 * @param altitude  Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_zat_gpu_data_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int8_t psuTemperature,int8_t spoolTemperature,uint8_t psuFanDuty,uint8_t spoolFanDuty,uint8_t motorDuty,int8_t motorDutyOffset,int8_t tetherTension,uint8_t tetherLength,int32_t latitude,int32_t longitude,float altitude,uint16_t pressure,int8_t pressureTemperature,uint16_t outputVoltage)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_uint16_t(buf, 12, pressure);
    _mav_put_uint16_t(buf, 14, outputVoltage);
    _mav_put_int8_t(buf, 16, psuTemperature);
    _mav_put_int8_t(buf, 17, spoolTemperature);
    _mav_put_uint8_t(buf, 18, psuFanDuty);
    _mav_put_uint8_t(buf, 19, spoolFanDuty);
    _mav_put_uint8_t(buf, 20, motorDuty);
    _mav_put_int8_t(buf, 21, motorDutyOffset);
    _mav_put_int8_t(buf, 22, tetherTension);
    _mav_put_uint8_t(buf, 23, tetherLength);
    _mav_put_int8_t(buf, 24, pressureTemperature);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;

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
    return mavlink_msg_zat_gpu_data_pack(system_id, component_id, msg, zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage);
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
    return mavlink_msg_zat_gpu_data_pack_chan(system_id, component_id, chan, msg, zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage);
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
    return mavlink_msg_zat_gpu_data_pack_status(system_id, component_id, _status, msg,  zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage);
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
 * @param latitude  Latitude.
 * @param longitude  Longitude.
 * @param altitude  Altitude.
 * @param pressure  Pressure.
 * @param pressureTemperature  Pressure Temperature.
 * @param outputVoltage  Output Voltage.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_zat_gpu_data_send(mavlink_channel_t chan, int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN];
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_uint16_t(buf, 12, pressure);
    _mav_put_uint16_t(buf, 14, outputVoltage);
    _mav_put_int8_t(buf, 16, psuTemperature);
    _mav_put_int8_t(buf, 17, spoolTemperature);
    _mav_put_uint8_t(buf, 18, psuFanDuty);
    _mav_put_uint8_t(buf, 19, spoolFanDuty);
    _mav_put_uint8_t(buf, 20, motorDuty);
    _mav_put_int8_t(buf, 21, motorDutyOffset);
    _mav_put_int8_t(buf, 22, tetherTension);
    _mav_put_uint8_t(buf, 23, tetherLength);
    _mav_put_int8_t(buf, 24, pressureTemperature);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ZAT_GPU_DATA, buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#else
    mavlink_zat_gpu_data_t packet;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.altitude = altitude;
    packet.pressure = pressure;
    packet.outputVoltage = outputVoltage;
    packet.psuTemperature = psuTemperature;
    packet.spoolTemperature = spoolTemperature;
    packet.psuFanDuty = psuFanDuty;
    packet.spoolFanDuty = spoolFanDuty;
    packet.motorDuty = motorDuty;
    packet.motorDutyOffset = motorDutyOffset;
    packet.tetherTension = tetherTension;
    packet.tetherLength = tetherLength;
    packet.pressureTemperature = pressureTemperature;

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
    mavlink_msg_zat_gpu_data_send(chan, zat_gpu_data->psuTemperature, zat_gpu_data->spoolTemperature, zat_gpu_data->psuFanDuty, zat_gpu_data->spoolFanDuty, zat_gpu_data->motorDuty, zat_gpu_data->motorDutyOffset, zat_gpu_data->tetherTension, zat_gpu_data->tetherLength, zat_gpu_data->latitude, zat_gpu_data->longitude, zat_gpu_data->altitude, zat_gpu_data->pressure, zat_gpu_data->pressureTemperature, zat_gpu_data->outputVoltage);
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
static inline void mavlink_msg_zat_gpu_data_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int8_t psuTemperature, int8_t spoolTemperature, uint8_t psuFanDuty, uint8_t spoolFanDuty, uint8_t motorDuty, int8_t motorDutyOffset, int8_t tetherTension, uint8_t tetherLength, int32_t latitude, int32_t longitude, float altitude, uint16_t pressure, int8_t pressureTemperature, uint16_t outputVoltage)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, latitude);
    _mav_put_int32_t(buf, 4, longitude);
    _mav_put_float(buf, 8, altitude);
    _mav_put_uint16_t(buf, 12, pressure);
    _mav_put_uint16_t(buf, 14, outputVoltage);
    _mav_put_int8_t(buf, 16, psuTemperature);
    _mav_put_int8_t(buf, 17, spoolTemperature);
    _mav_put_uint8_t(buf, 18, psuFanDuty);
    _mav_put_uint8_t(buf, 19, spoolFanDuty);
    _mav_put_uint8_t(buf, 20, motorDuty);
    _mav_put_int8_t(buf, 21, motorDutyOffset);
    _mav_put_int8_t(buf, 22, tetherTension);
    _mav_put_uint8_t(buf, 23, tetherLength);
    _mav_put_int8_t(buf, 24, pressureTemperature);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ZAT_GPU_DATA, buf, MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN, MAVLINK_MSG_ID_ZAT_GPU_DATA_CRC);
#else
    mavlink_zat_gpu_data_t *packet = (mavlink_zat_gpu_data_t *)msgbuf;
    packet->latitude = latitude;
    packet->longitude = longitude;
    packet->altitude = altitude;
    packet->pressure = pressure;
    packet->outputVoltage = outputVoltage;
    packet->psuTemperature = psuTemperature;
    packet->spoolTemperature = spoolTemperature;
    packet->psuFanDuty = psuFanDuty;
    packet->spoolFanDuty = spoolFanDuty;
    packet->motorDuty = motorDuty;
    packet->motorDutyOffset = motorDutyOffset;
    packet->tetherTension = tetherTension;
    packet->tetherLength = tetherLength;
    packet->pressureTemperature = pressureTemperature;

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
    return _MAV_RETURN_int8_t(msg,  16);
}

/**
 * @brief Get field spoolTemperature from zat_gpu_data message
 *
 * @return  Spool Temperature.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_spoolTemperature(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  17);
}

/**
 * @brief Get field psuFanDuty from zat_gpu_data message
 *
 * @return  PSU Fan Duty.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_psuFanDuty(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  18);
}

/**
 * @brief Get field spoolFanDuty from zat_gpu_data message
 *
 * @return  Spool Fan Duty.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_spoolFanDuty(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  19);
}

/**
 * @brief Get field motorDuty from zat_gpu_data message
 *
 * @return  Motor Duty.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_motorDuty(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  20);
}

/**
 * @brief Get field motorDutyOffset from zat_gpu_data message
 *
 * @return  Motor Duty Offset.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_motorDutyOffset(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  21);
}

/**
 * @brief Get field tetherTension from zat_gpu_data message
 *
 * @return  Tether Tension.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_tetherTension(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  22);
}

/**
 * @brief Get field tetherLength from zat_gpu_data message
 *
 * @return  Tether Length.
 */
static inline uint8_t mavlink_msg_zat_gpu_data_get_tetherLength(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  23);
}

/**
 * @brief Get field latitude from zat_gpu_data message
 *
 * @return  Latitude.
 */
static inline int32_t mavlink_msg_zat_gpu_data_get_latitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field longitude from zat_gpu_data message
 *
 * @return  Longitude.
 */
static inline int32_t mavlink_msg_zat_gpu_data_get_longitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field altitude from zat_gpu_data message
 *
 * @return  Altitude.
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
    return _MAV_RETURN_uint16_t(msg,  12);
}

/**
 * @brief Get field pressureTemperature from zat_gpu_data message
 *
 * @return  Pressure Temperature.
 */
static inline int8_t mavlink_msg_zat_gpu_data_get_pressureTemperature(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int8_t(msg,  24);
}

/**
 * @brief Get field outputVoltage from zat_gpu_data message
 *
 * @return  Output Voltage.
 */
static inline uint16_t mavlink_msg_zat_gpu_data_get_outputVoltage(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  14);
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
    zat_gpu_data->pressure = mavlink_msg_zat_gpu_data_get_pressure(msg);
    zat_gpu_data->outputVoltage = mavlink_msg_zat_gpu_data_get_outputVoltage(msg);
    zat_gpu_data->psuTemperature = mavlink_msg_zat_gpu_data_get_psuTemperature(msg);
    zat_gpu_data->spoolTemperature = mavlink_msg_zat_gpu_data_get_spoolTemperature(msg);
    zat_gpu_data->psuFanDuty = mavlink_msg_zat_gpu_data_get_psuFanDuty(msg);
    zat_gpu_data->spoolFanDuty = mavlink_msg_zat_gpu_data_get_spoolFanDuty(msg);
    zat_gpu_data->motorDuty = mavlink_msg_zat_gpu_data_get_motorDuty(msg);
    zat_gpu_data->motorDutyOffset = mavlink_msg_zat_gpu_data_get_motorDutyOffset(msg);
    zat_gpu_data->tetherTension = mavlink_msg_zat_gpu_data_get_tetherTension(msg);
    zat_gpu_data->tetherLength = mavlink_msg_zat_gpu_data_get_tetherLength(msg);
    zat_gpu_data->pressureTemperature = mavlink_msg_zat_gpu_data_get_pressureTemperature(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN? msg->len : MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN;
        memset(zat_gpu_data, 0, MAVLINK_MSG_ID_ZAT_GPU_DATA_LEN);
    memcpy(zat_gpu_data, _MAV_PAYLOAD(msg), len);
#endif
}
