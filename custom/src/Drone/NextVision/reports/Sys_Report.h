#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_sys_report_t {
    uint16_t report_type;
    float    roll;
    float    pitch;
    float    fov;
    uint8_t  tracker_status;
    uint8_t  recording_status;
    uint8_t  sensor;
    uint8_t  polarity;
    uint8_t  mode;
    uint8_t  laser_status;
    int16_t  tracker_roi_x;
    int16_t  tracker_roi_y;
    float    single_yaw_cmd;
    uint8_t  snapshot_busy;
    float    cpu_temp;
    float    camera_ver;
    int32_t  trip_ver;
    uint16_t bit_status;
    uint8_t  status_flags;
    uint8_t  camera_type;
    float    roll_rate;
    float    pitch_rate;
    float    cam_temp;
    float    roll_derot;
    uint8_t  network_type;
}) mavlink_nvext_sys_report_t;

#define MAVLINK_MSG_NVEXT_SYS_REPORT_LEN 0x3E

static inline void mavlink_nvext_sys_report_decode(const mavlink_message_t* msg, mavlink_nvext_sys_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_SYS_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_SYS_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_SYS_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
