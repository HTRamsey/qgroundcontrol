#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_object_detection_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    uint16_t det_classID;
    uint16_t det_uniqueID;
    float    det_x;
    float    det_y;
    float    det_width;
    float    det_height;
    float    det_lat;
    float    det_lon;
    float    det_alt;
}) mavlink_nvext_object_detection_report_t;

#define MAVLINK_MSG_NVEXT_OBJECT_DETECTION_REPORT_MIN_LEN        ( 14 + 32 )
#define MAVLINK_MSG_NVEXT_OBJECT_DETECTION_REPORT_MAX_LEN        ( 14 + ( 32 * 7 ) )

static inline void mavlink_nvext_object_detection_report_decode(const mavlink_message_t* msg, mavlink_nvext_object_detection_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_OBJECT_DETECTION_REPORT_MIN_LEN ? msg->len : MAVLINK_MSG_NVEXT_OBJECT_DETECTION_REPORT_MIN_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_OBJECT_DETECTION_REPORT_MIN_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
