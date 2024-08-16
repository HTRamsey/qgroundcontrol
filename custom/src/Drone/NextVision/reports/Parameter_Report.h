#pragma once

#include "mavlink/protocol/mavlink_types.h"
#include <string.h>

MAVPACKED(
typedef struct __mavlink_nvext_parameter_report_t {
    uint16_t report_type;
    uint8_t  status;
    uint16_t param_id;
    float    param_value;
}) mavlink_nvext_parameter_report_t;

#define MAVLINK_MSG_NVEXT_PARAMETER_REPORT_LEN 9

static inline void mavlink_nvext_parameter_report_decode(const mavlink_message_t* msg, mavlink_nvext_parameter_report_t* report)
{
    const uint8_t len = msg->len < MAVLINK_MSG_NVEXT_PARAMETER_REPORT_LEN ? msg->len : MAVLINK_MSG_NVEXT_PARAMETER_REPORT_LEN;
    memset(report, 0, MAVLINK_MSG_NVEXT_PARAMETER_REPORT_LEN);
    memcpy(report, _MAV_PAYLOAD(msg), len);
}
