#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_video_motion_detection_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    float objx;
    float objy;
    float objwidth;
    float objheight;
    float objlat;
    float objlon;
    float objalt;
}) mavlink_nvext_vmd_report_t;

#define MAVLINK_MSG_NVEXT_VIDEO_MOTION_DETECTION_REPORT_MIN_LEN     ( 14 + 28 )
#define MAVLINK_MSG_NVEXT_VIDEO_MOTION_DETECTION_REPORT_MAX_LEN     ( 14 + ( 28 * 8 ) )

static inline void mavlink_nvext_vmd_report_decode(const mavlink_message_t* msg, mavlink_nvext_vmd_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_VIDEO_MOTION_DETECTION_REPORT_MIN_LEN ? msg->len : MAVLINK_MSG_NVEXT_VIDEO_MOTION_DETECTION_REPORT_MIN_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_VIDEO_MOTION_DETECTION_REPORT_MIN_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
