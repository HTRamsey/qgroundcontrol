#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_los_report_t {
    uint16_t report_type;
    float    los_x;
    float    los_y;
    float    los_z;
    float    los_upper_left_corner_lat;
    float    los_upper_left_corner_lon;
    float    los_upper_right_corner_lat;
    float    los_upper_right_corner_lon;
    float    los_lower_right_corner_lat;
    float    los_lower_right_corner_lon;
    float    los_lower_left_corner_lat;
    float    los_lower_left_corner_lon;
    float    los_elevation;
    float    los_azimuth;
}) mavlink_nvext_los_report_t;

#define MAVLINK_MSG_NVEXT_LOS_REPORT_LEN 0x36

static inline void mavlink_nvext_los_report_decode(const mavlink_message_t* msg, mavlink_nvext_los_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_LOS_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_LOS_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_LOS_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
