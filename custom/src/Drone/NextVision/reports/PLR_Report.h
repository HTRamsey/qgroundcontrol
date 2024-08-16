#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_platform_location_refinement_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    double lat;
    double lon;
    double platform_lat;
    double platform_lon;
    uint32_t video_frame_index;
}) mavlink_nvext_plr_report_t;

#define MAVLINK_MSG_NVEXT_PLATFORM_LOCATION_REFINEMENT_REPORT_LEN 50

static inline void mavlink_nvext_plr_report_decode(const mavlink_message_t* msg, mavlink_nvext_plr_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_PLATFORM_LOCATION_REFINEMENT_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_PLATFORM_LOCATION_REFINEMENT_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_PLATFORM_LOCATION_REFINEMENT_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
