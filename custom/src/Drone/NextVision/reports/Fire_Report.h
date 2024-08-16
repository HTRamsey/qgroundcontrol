#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_fire_report_t {
    uint16_t report_type;
    float    gnd_cross_lat;
    float    gnd_cross_long;
    float    gnd_cross_alt;
    float    fov;
    uint16_t total_sat_pixels;
    uint8_t  regional_line_0;
    uint8_t  regional_line_1;
    uint8_t  regional_line_2;
    uint8_t  regional_line_3;
    uint8_t  regional_line_4;
    uint8_t  regional_line_5;
    uint8_t  regional_line_6;
    uint8_t  regional_line_7;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
}) mavlink_nvext_fire_report_t;

#define MAVLINK_MSG_NVEXT_FIRE_REPORT_LEN 0x1C

static inline void mavlink_nvext_fire_report_decode(const mavlink_message_t* msg, mavlink_nvext_fire_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_FIRE_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_FIRE_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_FIRE_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
