#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_ar_markers_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    uint16_t marker_id;
    float 	 marker_x1;
    float 	 marker_y1;
    float 	 marker_x2;
    float 	 marker_y2;
    float 	 marker_x3;
    float 	 marker_y3;
    float 	 marker_x4;
    float 	 marker_y4;
}) mavlink_nvext_ar_markers_report_t;

#define MAVLINK_MSG_NVEXT_AR_REPORT_MIN_LEN ( 14 + 34 )
#define MAVLINK_MSG_NVEXT_AR_REPORT_MAX_LEN ( 14 + ( 34 * 16 ) )

static inline void mavlink_nvext_ar_markers_report_decode(const mavlink_message_t* msg, mavlink_nvext_ar_markers_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_AR_REPORT_MIN_LEN ? msg->len : MAVLINK_MSG_NVEXT_AR_REPORT_MIN_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_AR_REPORT_MIN_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
