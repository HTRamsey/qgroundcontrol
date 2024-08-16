#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_car_count_report_t {
    uint16_t report_type;
    uint32_t time_since_boot;
    uint64_t utc_timestamp;
    uint32_t car_count;
}) mavlink_nvext_car_count_report_t;

#define MAVLINK_MSG_NVEXT_CAR_COUNT_REPORT_LEN 0x12

static inline void mavlink_nvext_car_count_report_decode(const mavlink_message_t* msg, mavlink_nvext_car_count_report_t* report)
{
    uint8_t len = msg->len < MAVLINK_MSG_NVEXT_CAR_COUNT_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_CAR_COUNT_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_CAR_COUNT_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
