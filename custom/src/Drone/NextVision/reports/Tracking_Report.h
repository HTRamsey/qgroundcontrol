#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_tracking_report_t {
    uint16_t report_type;
    float    los_312_x;
    float    los_312_y;
    float    los_312_z;
    float    los_rate_x;
    float    los_rate_y;
    float    roi_az;
    float    roi_el;
    float    roi_lat;
    float    roi_lon;
    float    roi_alt;
    float    roi_x;
    float    roi_y;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
}) mavlink_nvext_tracking_report_t;

#define MAVLINK_MSG_NVEXT_TRACKING_REPORT_LEN 0x40

static inline void mavlink_nvext_tracking_report_decode(const mavlink_message_t* msg, mavlink_nvext_tracking_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_TRACKING_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_TRACKING_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_TRACKING_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
