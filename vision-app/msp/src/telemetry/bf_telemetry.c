/**
 * @file bf_telemetry.c this is part of project 'msp'
 * Copyright © 2020-2023
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Created vitalii.nimych@gmail.com 17-06-2023
 */

#include <string.h>
#include <stdio.h>
#include "bf_telemetry.h"
#include "src/msp/msp_api.h"
#include "src/msp/msp_protocol_helper.h"
#include "src/syslog/logger.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#define DEBUG_TELEMETRY \
    do {                \
        printf           \
    }


static bf_telemetry_data_t bf_telemetry_data = {0};

bf_telemetry_data_t *bf_telemetry_get_ptr(void)
{
    return &bf_telemetry_data;
}

int bf_telemetry_init(msp_interface_t *msp_interface)
{
    uint8_t tx_buff[512] = {0};
    memset(&bf_telemetry_data, 0, sizeof (bf_telemetry_data_t));
    int ret = -1;

    int len = msp_api_request_fc_version(tx_buff);
    ret = msp_interface_write(msp_interface, tx_buff, len);
    len = msp_api_request_fc_variant(tx_buff);
    ret = msp_interface_write(msp_interface, tx_buff, len);

    return ret;
}

void bf_telemetry_update(msp_interface_t msp_interface)
{

}

void bf_telemetry_handler(msp_msg_t *message)
{
    switch (message->cmd) {
        case MSP_RC:
            msp_helper_pars_rc_cmd(message->payload, message->size, bf_telemetry_data.rc_command);
            SYS_LOG_DEBUG("RC ch: Roll: '%d', Pitch: '%d' Thr: '%d' Yaw: '%d'\n",
                          bf_telemetry_data.rc_command[0],
                          bf_telemetry_data.rc_command[1],
                          bf_telemetry_data.rc_command[3],
                          bf_telemetry_data.rc_command[2]);
            break;

        case MSP_FC_VARIANT:
            if (message->size == 4) {
                memcpy(bf_telemetry_data.FC_ID, message->payload, message->size);
                SYS_LOG_DEBUG("FC variant: %c%c%c%c\n",
                              bf_telemetry_data.FC_ID[0],
                              bf_telemetry_data.FC_ID[1],
                              bf_telemetry_data.FC_ID[2],
                              bf_telemetry_data.FC_ID[3]);
            }
            break;

        case MSP_API_VERSION:
            if (message->size == 3) {
                bf_telemetry_data.fc_version.release = message->payload[0];
                bf_telemetry_data.fc_version.minor = message->payload[1];
                bf_telemetry_data.fc_version.major = message->payload[2];
                SYS_LOG_DEBUG("MSP ver: %d.%d.%d\n", message->payload[0], message->payload[1], message->payload[2]);
            }
            break;

        case MSP_FC_VERSION:
            if (message->size == 3) {
                SYS_LOG_DEBUG("FC ver: %d.%d.%d\n", message->payload[0], message->payload[1], message->payload[2]);
            }
            break;

        case MSP_MIXER_CONFIG:
            if (message->size == 2) {

            }
            break;

        case MSP_RAW_IMU:

            break;

        case MSP_ATTITUDE:
        {
            msp_helper_pars_attitude(message->payload, message->size,
                                     &bf_telemetry_data.attitude.roll,
                                     &bf_telemetry_data.attitude.pitch,
                                     &bf_telemetry_data.attitude.yaw);
            SYS_LOG_DEBUG("Attitude: Roll:'%.1f', Pitch:'%.1f' Yaw:'%d'\n",
                          bf_telemetry_data.attitude.roll / 10.0,
                          bf_telemetry_data.attitude.pitch / 10.0,
                          bf_telemetry_data.attitude.yaw);
        }
            break;

        default:
#if DEBUG // for debug payload
            printf("%s(): got MSP cmd id: '%d', len '%d'\n", __FUNCTION__, message->cmd, message->size);
            for (int i = 0; i < message->size; i++) {
                printf("'%c' ", message->payload[i]);
            }
            printf("\n");
            for (int i = 0; i < message->size; i++) {
                printf("[%d][0x%02X]", i, message->payload[i]);
            }
            printf("\n");
#endif
            break;
    }
}

