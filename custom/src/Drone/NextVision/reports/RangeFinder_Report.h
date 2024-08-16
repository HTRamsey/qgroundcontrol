#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_range_finder_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    float range;
}) mavlink_nvext_range_finder_report_t;

#define MAVLINK_MSG_NVEXT_RANGE_FINDER_REPORT_LEN 18

static inline void mavlink_nvext_range_finder_report_decode(const mavlink_message_t* msg, mavlink_nvext_range_finder_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_RANGE_FINDER_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_RANGE_FINDER_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_RANGE_FINDER_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
