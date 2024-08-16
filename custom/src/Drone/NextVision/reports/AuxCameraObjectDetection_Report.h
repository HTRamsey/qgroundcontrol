#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_aux_camera_object_detection_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    uint8_t aux_camera_id;
    uint16_t det_classID;
    float det_azimuth;
    float det_elevation;
    float det_relative_azimuth;
    float det_relative_elevation;
}) mavlink_nvext_aux_camera_object_detection_report_t;

#define MAVLINK_MSG_NVEXT_AUX_CAMERA_OBJECT_DETECTION_REPORT_MIN_LEN ( 15 + 18 )
#define MAVLINK_MSG_NVEXT_AUX_CAMERA_OBJECT_DETECTION_REPORT_MAX_LEN ( 15 + ( 18 * 13 ) )

static inline void mavlink_nvext_aux_camera_object_detection_report_decode(const mavlink_message_t* msg, mavlink_nvext_aux_camera_object_detection_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_AUX_CAMERA_OBJECT_DETECTION_REPORT_MIN_LEN ? msg->len : MAVLINK_MSG_NVEXT_AUX_CAMERA_OBJECT_DETECTION_REPORT_MIN_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_AUX_CAMERA_OBJECT_DETECTION_REPORT_MIN_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
