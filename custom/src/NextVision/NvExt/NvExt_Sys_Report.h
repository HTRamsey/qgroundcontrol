#pragma once
// MESSAGE MAVLINK NvExt System Report

#define MAVLINK_MSG_NVEXT_SYS_REPORT 248

#include "mavlink_types.h"

MAVPACKED(
typedef struct __mavlink_nvext_sys_report_t {
    uint16_t            report_type;
    float               roll;
    float               pitch;
    float               fov;
    uint8_t             tracker_status;
    uint8_t             recording_status;
    uint8_t             sensor;
    uint8_t             polarity;
    uint8_t             mode;
    uint8_t             laser_status;
    int16_t             tracker_roi_x;
    int16_t             tracker_roi_y;
    float               single_yaw_cmd;
    uint8_t             snapshot_busy;
    float               cpu_temp;
    float               camera_ver;
    int32_t             trip2_ver;
    uint16_t            bit_status;
    uint8_t             status_flags;
    uint8_t             camera_type;
    float               roll_rate;
    float               pitch_rate;
    float               cam_temp;
    float               roll_derot;
}) mavlink_nvext_sys_report_t;

#define MAVLINK_MSG_NVEXT_SYS_REPORT_LEN        61
#define MAVLINK_MSG_NVEXT_SYS_REPORT_MIN_LEN    61
#define MAVLINK_MSG_NVEXT_SYS_REPORT_CRC        8

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_NVEXT_SYS_REPORT {                                                                                         \
    248,                                                                                                                                \
    "V2_EXTENSION",                                                                                                                     \
    5,                                                                                                                                  \
    {   { "target_network",    NULL, MAVLINK_TYPE_UINT8_T,     0,      2,  offsetof(mavlink_v2_extension_t, target_network)     },      \
        { "target_system",     NULL, MAVLINK_TYPE_UINT8_T,     0,      3,  offsetof(mavlink_v2_extension_t, target_system)      },      \
        { "target_component",  NULL, MAVLINK_TYPE_UINT8_T,     0,      4,  offsetof(mavlink_v2_extension_t, target_component)   },      \
        { "message_type",      NULL, MAVLINK_TYPE_UINT16_T,    0,      0,  offsetof(mavlink_v2_extension_t, message_type)       },      \
        { "payload",           NULL, MAVLINK_TYPE_UINT8_T,     249,    5,  offsetof(mavlink_v2_extension_t, payload)            },      \
    }                                                                                                                                   \
}
#else
#define MAVLINK_MESSAGE_INFO_NVEXT_SYS_REPORT {                                                                                         \
    "NVEXT_SYS_REPORT",                                                                                                                 \
    24,                                                                                                                                 \
    {   { "report_type",       NULL, MAVLINK_TYPE_UINT16_T,    0,      0,   offsetof(mavlink_nvext_sys_report_t, report_type)       },  \
        { "roll",              NULL, MAVLINK_TYPE_FLOAT,       0,      2,   offsetof(mavlink_nvext_sys_report_t, roll)              },  \
        { "pitch",             NULL, MAVLINK_TYPE_FLOAT,       0,      6,   offsetof(mavlink_nvext_sys_report_t, pitch)             },  \
        { "fov",               NULL, MAVLINK_TYPE_FLOAT,       0,      10,  offsetof(mavlink_nvext_sys_report_t, fov)               },  \
        { "tracker_status",    NULL, MAVLINK_TYPE_UINT8_T,     0,      14,  offsetof(mavlink_nvext_sys_report_t, tracker_status)    },  \
        { "recording_status",  NULL, MAVLINK_TYPE_UINT8_T,     0,      15,  offsetof(mavlink_nvext_sys_report_t, recording_status)  },  \
        { "sensor",            NULL, MAVLINK_TYPE_UINT8_T,     0,      16,  offsetof(mavlink_nvext_sys_report_t, sensor)            },  \
        { "polarity",          NULL, MAVLINK_TYPE_UINT8_T,     0,      17,  offsetof(mavlink_nvext_sys_report_t, polarity)          },  \
        { "mode",              NULL, MAVLINK_TYPE_UINT8_T,     0,      18,  offsetof(mavlink_nvext_sys_report_t, mode)              },  \
        { "laser_status",      NULL, MAVLINK_TYPE_UINT8_T,     0,      19,  offsetof(mavlink_nvext_sys_report_t, laser_status)      },  \
        { "tracker_roi_x",     NULL, MAVLINK_TYPE_INT16_T,     0,      20,  offsetof(mavlink_nvext_sys_report_t, tracker_roi_x)     },  \
        { "tracker_roi_y",     NULL, MAVLINK_TYPE_INT16_T,     0,      22,  offsetof(mavlink_nvext_sys_report_t, tracker_roi_y)     },  \
        { "single_yaw_cmd",    NULL, MAVLINK_TYPE_FLOAT,       0,      24,  offsetof(mavlink_nvext_sys_report_t, single_yaw_cmd)    },  \
        { "snapshot_busy",     NULL, MAVLINK_TYPE_UINT8_T,     0,      28,  offsetof(mavlink_nvext_sys_report_t, snapshot_busy)     },  \
        { "cpu_temp",          NULL, MAVLINK_TYPE_FLOAT,       0,      29,  offsetof(mavlink_nvext_sys_report_t, cpu_temp)          },  \
        { "camera_ver",        NULL, MAVLINK_TYPE_FLOAT,       0,      33,  offsetof(mavlink_nvext_sys_report_t, camera_ver)        },  \
        { "trip2_ver",         NULL, MAVLINK_TYPE_INT32_T,     0,      37,  offsetof(mavlink_nvext_sys_report_t, trip2_ver)         },  \
        { "bit_status",        NULL, MAVLINK_TYPE_UINT16_T,    0,      41,  offsetof(mavlink_nvext_sys_report_t, bit_status)        },  \
        { "status_flags",      NULL, MAVLINK_TYPE_UINT8_T,     0,      43,  offsetof(mavlink_nvext_sys_report_t, status_flags)      },  \
        { "camera_type",       NULL, MAVLINK_TYPE_UINT8_T,     0,      44,  offsetof(mavlink_nvext_sys_report_t, camera_type)       },  \
        { "roll_rate",         NULL, MAVLINK_TYPE_FLOAT,       0,      45,  offsetof(mavlink_nvext_sys_report_t, roll_rate)         },  \
        { "pitch_rate",        NULL, MAVLINK_TYPE_FLOAT,       0,      49,  offsetof(mavlink_nvext_sys_report_t, pitch_rate)        },  \
        { "cam_temp",          NULL, MAVLINK_TYPE_FLOAT,       0,      53,  offsetof(mavlink_nvext_sys_report_t, cam_temp)          },  \
        { "roll_derot",        NULL, MAVLINK_TYPE_FLOAT,       0,      57,  offsetof(mavlink_nvext_sys_report_t, roll_derot)        },  \
    }                                                                                                                                   \
}
#endif

// MESSAGE MAVLINK_MSG_NVEXT_SYS_REPORT UNPACKING

/**
 * @brief Get field report_type from mavlink_nvext_sys_report message
 *
 * @return Report type
 */
static inline uint16_t mavlink_nvext_sys_report_get_report_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  0);
}

/**
 * @brief Get field roll from mavlink_nvext_sys_report message
 *
 * @return Roll
 */
static inline float mavlink_nvext_sys_report_get_roll(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  2);
}

/**
 * @brief Get field pitch from mavlink_nvext_sys_report message
 *
 * @return Pitch
 */
static inline float mavlink_nvext_sys_report_get_pitch(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  6);
}

/**
 * @brief Get field fov from mavlink_nvext_sys_report message
 *
 * @return FOV
 */
static inline float mavlink_nvext_sys_report_get_fov(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  10);
}

/**
 * @brief Get field tracker_status from mavlink_nvext_sys_report message
 *
 * @return The Tracker Status
 */
static inline uint8_t mavlink_nvext_sys_report_get_tracker_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  14);
}

/**
 * @brief Get field recording_status from mavlink_nvext_sys_report message
 *
 * @return The Recording Status
 */
static inline uint8_t mavlink_nvext_sys_report_get_recording_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  15);
}

/**
 * @brief Get field sensor from mavlink_nvext_sys_report message
 *
 * @return Sensor type
 */
static inline uint8_t mavlink_nvext_sys_report_get_sensor(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  16);
}

/**
 * @brief Get field polarity from mavlink_nvext_sys_report message
 *
 * @return Polarity mode
 */
static inline uint8_t mavlink_nvext_sys_report_get_polarity(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  17);
}

/**
 * @brief Get field mode from mavlink_nvext_sys_report message
 *
 * @return Mode
 */
static inline uint8_t mavlink_nvext_sys_report_get_mode(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  18);
}

/**
 * @brief Get field laser_status from mavlink_nvext_sys_report message
 *
 * @return laser_status
 */
static inline uint8_t mavlink_nvext_sys_report_get_laser_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  19);
}

/**
 * @brief Get field tracker_roi_x from mavlink_nvext_sys_report message
 *
 * @return tracker_roi_x
 */
static inline int16_t mavlink_nvext_sys_report_get_tracker_roi_x(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  20);
}

/**
 * @brief Get field tracker_roi_y from mavlink_nvext_sys_report message
 *
 * @return tracker_roi_y
 */
static inline int16_t mavlink_nvext_sys_report_get_tracker_roi_y(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int16_t(msg,  22);
}

/**
 * @brief Get field single_yaw_cmd from mavlink_nvext_sys_report message
 *
 * @return single_yaw_cmd
 */
static inline float mavlink_nvext_sys_report_get_single_yaw_cmd(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  24);
}

/**
 * @brief Get field snapshot_busy from mavlink_nvext_sys_report message
 *
 * @return snapshot_busy
 */
static inline uint8_t mavlink_nvext_sys_report_get_snapshot_busy(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  28);
}

/**
 * @brief Get field cpu_temp from mavlink_nvext_sys_report message
 *
 * @return cpu_temp
 */
static inline float mavlink_nvext_sys_report_get_cpu_temp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  29);
}

/**
 * @brief Get field camera_ver from mavlink_nvext_sys_report message
 *
 * @return camera_ver
 */
static inline float mavlink_nvext_sys_report_get_camera_ver(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  33);
}

/**
 * @brief Get field trip2_ver from mavlink_nvext_sys_report message
 *
 * @return trip2_ver
 */
static inline int32_t mavlink_nvext_sys_report_get_trip2_ver(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  37);
}

/**
 * @brief Get field bit_status from mavlink_nvext_sys_report message
 *
 * @return bit_status
 */
static inline uint16_t mavlink_nvext_sys_report_get_bit_status(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  41);
}

/**
 * @brief Get field status_flags from mavlink_nvext_sys_report message
 *
 * @return status_flags
 */
static inline uint8_t mavlink_nvext_sys_report_get_status_flags(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  43);
}

/**
 * @brief Get field camera_type from mavlink_nvext_sys_report message
 *
 * @return camera_type
 */
static inline uint8_t mavlink_nvext_sys_report_get_camera_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  44);
}

/**
 * @brief Get field roll_rate from mavlink_nvext_sys_report message
 *
 * @return roll_rate
 */
static inline float mavlink_nvext_sys_report_get_roll_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  45);
}

/**
 * @brief Get field pitch_rate from mavlink_nvext_sys_report message
 *
 * @return pitch_rate
 */
static inline float mavlink_nvext_sys_report_get_pitch_rate(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  49);
}

/**
 * @brief Get field cam_temp from mavlink_nvext_sys_report message
 *
 * @return cam_temp
 */
static inline float mavlink_nvext_sys_report_get_cam_temp(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  53);
}

/**
 * @brief Get field roll_derot from mavlink_nvext_sys_report message
 *
 * @return roll_derot
 */
static inline float mavlink_nvext_sys_report_get_roll_derot(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  57);
}

/**
 * @brief Decode a v2_extension message into a struct
 *
 * @param msg The message to decode
 * @param nvext_sys_report C-struct to decode the message contents into
 */
static inline void mavlink_nvext_sys_report_decode(const mavlink_message_t* msg, mavlink_nvext_sys_report_t* nvext_sys_report)
{
    nvext_sys_report->report_type       = mavlink_nvext_sys_report_get_report_type      (msg);
    nvext_sys_report->roll              = mavlink_nvext_sys_report_get_roll             (msg);
    nvext_sys_report->pitch             = mavlink_nvext_sys_report_get_pitch            (msg);
    nvext_sys_report->fov               = mavlink_nvext_sys_report_get_fov              (msg);
    nvext_sys_report->tracker_status    = mavlink_nvext_sys_report_get_tracker_status   (msg);
    nvext_sys_report->recording_status  = mavlink_nvext_sys_report_get_recording_status (msg);
    nvext_sys_report->sensor            = mavlink_nvext_sys_report_get_sensor           (msg);
    nvext_sys_report->polarity          = mavlink_nvext_sys_report_get_polarity         (msg);
    nvext_sys_report->mode              = mavlink_nvext_sys_report_get_mode             (msg);
    nvext_sys_report->laser_status      = mavlink_nvext_sys_report_get_laser_status     (msg);
    nvext_sys_report->tracker_roi_x     = mavlink_nvext_sys_report_get_tracker_roi_x    (msg);
    nvext_sys_report->tracker_roi_y     = mavlink_nvext_sys_report_get_tracker_roi_y    (msg);
    nvext_sys_report->single_yaw_cmd    = mavlink_nvext_sys_report_get_single_yaw_cmd   (msg);
    nvext_sys_report->snapshot_busy     = mavlink_nvext_sys_report_get_snapshot_busy    (msg);
    nvext_sys_report->cpu_temp          = mavlink_nvext_sys_report_get_cpu_temp         (msg);
    nvext_sys_report->camera_ver        = mavlink_nvext_sys_report_get_camera_ver       (msg);
    nvext_sys_report->trip2_ver         = mavlink_nvext_sys_report_get_trip2_ver        (msg);
    nvext_sys_report->bit_status        = mavlink_nvext_sys_report_get_bit_status       (msg);
    nvext_sys_report->status_flags      = mavlink_nvext_sys_report_get_status_flags     (msg);
    nvext_sys_report->camera_type       = mavlink_nvext_sys_report_get_camera_type      (msg);
    nvext_sys_report->roll_rate         = mavlink_nvext_sys_report_get_roll_rate        (msg);
    nvext_sys_report->pitch_rate        = mavlink_nvext_sys_report_get_pitch_rate       (msg);
    nvext_sys_report->cam_temp          = mavlink_nvext_sys_report_get_cam_temp         (msg);
    nvext_sys_report->roll_derot        = mavlink_nvext_sys_report_get_roll_derot       (msg);
}
