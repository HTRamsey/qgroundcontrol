#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_los_dir_rate_report_t {
    uint16_t report_type;
    float    los_rate_x;
    float    los_rate_y;
    float    los_pan;
    float    los_x;
    float    los_tilt;
    float    los_y;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
}) mavlink_nvext_los_dir_rate_report_t;

#define MAVLINK_MSG_NVEXT_LOS_DIR_RATE_REPORT_LEN 0x1A

static inline void mavlink_nvext_los_dir_rate_report_decode(const mavlink_message_t* msg, mavlink_nvext_los_dir_rate_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_LOS_DIR_RATE_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_LOS_DIR_RATE_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_LOS_DIR_RATE_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
