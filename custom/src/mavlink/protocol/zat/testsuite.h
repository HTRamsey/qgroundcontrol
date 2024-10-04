/** @file
 *    @brief MAVLink comm protocol testsuite generated from zat.xml
 *    @see https://mavlink.io/en/
 */
#pragma once
#ifndef ZAT_TESTSUITE_H
#define ZAT_TESTSUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAVLINK_TEST_ALL
#define MAVLINK_TEST_ALL
static void mavlink_test_all(uint8_t, uint8_t, mavlink_message_t *last_msg);
static void mavlink_test_zat(uint8_t, uint8_t, mavlink_message_t *last_msg);

static void mavlink_test_all(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_all(system_id, component_id, last_msg);
    mavlink_test_zat(system_id, component_id, last_msg);
}
#endif

#include "../all/testsuite.h"


static void mavlink_test_zat_gpu_data(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_ZAT_GPU_DATA >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_zat_gpu_data_t packet_in = {
        963497464,963497672,73.0,17859,17963,53,120,187,254,65,132,199,10,77
    };
    mavlink_zat_gpu_data_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.latitude = packet_in.latitude;
        packet1.longitude = packet_in.longitude;
        packet1.altitude = packet_in.altitude;
        packet1.pressure = packet_in.pressure;
        packet1.outputVoltage = packet_in.outputVoltage;
        packet1.psuTemperature = packet_in.psuTemperature;
        packet1.spoolTemperature = packet_in.spoolTemperature;
        packet1.psuFanDuty = packet_in.psuFanDuty;
        packet1.spoolFanDuty = packet_in.spoolFanDuty;
        packet1.motorDuty = packet_in.motorDuty;
        packet1.motorDutyOffset = packet_in.motorDutyOffset;
        packet1.tetherTension = packet_in.tetherTension;
        packet1.tetherLength = packet_in.tetherLength;
        packet1.pressureTemperature = packet_in.pressureTemperature;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_ZAT_GPU_DATA_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_zat_gpu_data_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_zat_gpu_data_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_zat_gpu_data_pack(system_id, component_id, &msg , packet1.psuTemperature , packet1.spoolTemperature , packet1.psuFanDuty , packet1.spoolFanDuty , packet1.motorDuty , packet1.motorDutyOffset , packet1.tetherTension , packet1.tetherLength , packet1.latitude , packet1.longitude , packet1.altitude , packet1.pressure , packet1.pressureTemperature , packet1.outputVoltage );
    mavlink_msg_zat_gpu_data_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_zat_gpu_data_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.psuTemperature , packet1.spoolTemperature , packet1.psuFanDuty , packet1.spoolFanDuty , packet1.motorDuty , packet1.motorDutyOffset , packet1.tetherTension , packet1.tetherLength , packet1.latitude , packet1.longitude , packet1.altitude , packet1.pressure , packet1.pressureTemperature , packet1.outputVoltage );
    mavlink_msg_zat_gpu_data_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_zat_gpu_data_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_zat_gpu_data_send(MAVLINK_COMM_1 , packet1.psuTemperature , packet1.spoolTemperature , packet1.psuFanDuty , packet1.spoolFanDuty , packet1.motorDuty , packet1.motorDutyOffset , packet1.tetherTension , packet1.tetherLength , packet1.latitude , packet1.longitude , packet1.altitude , packet1.pressure , packet1.pressureTemperature , packet1.outputVoltage );
    mavlink_msg_zat_gpu_data_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

#ifdef MAVLINK_HAVE_GET_MESSAGE_INFO
    MAVLINK_ASSERT(mavlink_get_message_info_by_name("ZAT_GPU_DATA") != NULL);
    MAVLINK_ASSERT(mavlink_get_message_info_by_id(MAVLINK_MSG_ID_ZAT_GPU_DATA) != NULL);
#endif
}

static void mavlink_test_zat(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_zat_gpu_data(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ZAT_TESTSUITE_H
