#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_oglr_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    uint32_t pts_timestamp;
    double   target_refined_lat;
    double   target_refined_lon;
    float    target_refined_asl;
    float    target_refined_agl;
}) mavlink_nvext_oglr_report_t;

#define MAVLINK_MSG_NVEXT_OGLR_REPORT_LEN 55

static inline void mavlink_nvext_oglr_report_decode(const mavlink_message_t* msg, mavlink_nvext_oglr_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_OGLR_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_OGLR_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_OGLR_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
