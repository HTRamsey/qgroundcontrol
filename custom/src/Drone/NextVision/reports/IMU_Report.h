#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_imu_report_t {
    uint16_t report_type;
    float    cam_roll;
    float    cam_pitch;
    float    cam_x;
    float    cam_y;
    float    los_rate_x;
    float    los_rate_y;
    float    accel_x;
    float    accel_y;
    float    accel_z;
    float    gyro_x;
    float    gyro_y;
    float    gyro_z;
    float    compass_x;
    float    compass_y;
    float    compass_z;
    uint16_t timestamp;
    uint32_t boot_time;
    uint64_t utc_timestamp;
}) mavlink_nvext_imu_report_t;

#define MAVLINK_MSG_NVEXT_IMU_REPORT_LEN 0x40

static inline void mavlink_nvext_imu_report_decode(const mavlink_message_t* msg, mavlink_nvext_imu_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_IMU_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_IMU_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_IMU_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
