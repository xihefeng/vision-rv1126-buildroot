/**
 * @file telemetry.h this is part of project 'msp'
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

#ifndef MSP_TELEMETRY_H
#define MSP_TELEMETRY_H

#include <stdint.h>
#include "src/msp/msp_protocol.h"

typedef struct {
    int16_t pitch;
    int16_t roll;
    int16_t yaw;
} attitude_t;

typedef struct {
    uint8_t release;
    uint8_t minor;
    uint8_t major;
} fc_version_t;

typedef struct {
    char FC_ID[FLIGHT_CONTROLLER_IDENTIFIER_LENGTH];
    fc_version_t fc_version;
    attitude_t attitude;
    uint16_t rc_command[MSP_MAX_RC_CHANNEL];
    uint16_t vbat;
    uint16_t ibat;
    int8_t rssi;

} bf_telemetry_data_t;

#endif //MSP_TELEMETRY_H
