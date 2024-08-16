#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_lpr_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    char     plate_text[16];
    float	 plate_x;
    float 	 plate_y;
    float    plate_lat;
    float 	 plate_lon;
    float  	 plate_alt;
}) mavlink_nvext_lpr_report_t;

#define MAVLINK_MSG_NVEXT_LPR_REPORT_MIN_LEN ( 14 + 36 )
#define MAVLINK_MSG_NVEXT_LPR_REPORT_MAX_LEN ( 14 + ( 36 * 6 ) )

static inline void mavlink_nvext_lpr_report_decode(const mavlink_message_t* msg, mavlink_nvext_lpr_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_LPR_REPORT_MIN_LEN ? msg->len : MAVLINK_MSG_NVEXT_LPR_REPORT_MIN_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_LPR_REPORT_MIN_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
