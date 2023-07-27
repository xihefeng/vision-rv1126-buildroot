#ifndef MSP_MSP_PROTOCOL_HELPER_H
#define MSP_MSP_PROTOCOL_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "msp_protocol.h"

// Note: this is called MultiType/MULTITYPE_* in baseflight.
typedef enum mixerMode
{
    MIXER_TRI = 1,
    MIXER_QUADP = 2,
    MIXER_QUADX = 3,
    MIXER_BICOPTER = 4,
    MIXER_GIMBAL = 5,
    MIXER_Y6 = 6,
    MIXER_HEX6 = 7,
    MIXER_FLYING_WING = 8,
    MIXER_Y4 = 9,
    MIXER_HEX6X = 10,
    MIXER_OCTOX8 = 11,
    MIXER_OCTOFLATP = 12,
    MIXER_OCTOFLATX = 13,
    MIXER_AIRPLANE = 14,        // airplane / singlecopter / dualcopter (not yet properly supported)
    MIXER_HELI_120_CCPM = 15,
    MIXER_HELI_90_DEG = 16,
    MIXER_VTAIL4 = 17,
    MIXER_HEX6H = 18,
    MIXER_PPM_TO_SERVO = 19,    // PPM -> servo relay
    MIXER_DUALCOPTER = 20,
    MIXER_SINGLECOPTER = 21,
    MIXER_ATAIL4 = 22,
    MIXER_CUSTOM = 23,
    MIXER_CUSTOM_AIRPLANE = 24,
    MIXER_CUSTOM_TRI = 25,
    MIXER_QUADX_1234 = 26
} mixerMode_e;


char* msp_helper_get_str__of_mixer(mixerMode_e mode);

void msp_helper_pars_imu_data(const uint8_t *data, uint16_t len, uint16_t acc[4], uint16_t gyro[4], uint16_t mag[4]);

void msp_helper_pars_attitude(const uint8_t *data, uint16_t len, int16_t *roll, int16_t *pitch, int16_t *yaw);

void msp_helper_pars_rc_cmd(const uint8_t *msg, uint16_t len, uint16_t rc_data[MSP_MAX_RC_CHANNEL]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //MSP_MSP_PROTOCOL_HELPER_H
