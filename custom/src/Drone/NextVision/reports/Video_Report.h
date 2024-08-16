#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_video_report_t {
    uint16_t report_type;
    uint16_t bitrateStream0;
    uint16_t bitrateRecord0;
    uint16_t bitrateStream1;
    uint16_t bitrateRecord1;
}) mavlink_nvext_video_report_t;

#define MAVLINK_MSG_NVEXT_VIDEO_REPORT_LEN 0x0A

static inline void mavlink_nvext_video_report_decode(const mavlink_message_t* msg, mavlink_nvext_video_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_VIDEO_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_VIDEO_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_VIDEO_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
