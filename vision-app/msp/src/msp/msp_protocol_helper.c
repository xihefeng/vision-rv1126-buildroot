#include "msp_protocol_helper.h"
#include "msp_protocol.h"
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//#define MAX_RC_CHANNEL 16

char* msp_helper_get_str__of_mixer(mixerMode_e mode)
{
    char *str = NULL;
    switch (mode) {

        case MIXER_TRI:
            str = "TRICOPTER";
            break;
        case MIXER_QUADP:
            str = "QUAD P";
            break;
        case MIXER_QUADX:
            str = "QUAD X";
            break;
        case MIXER_BICOPTER:
            str = "BICOPTER";
            break;
        case MIXER_GIMBAL:
            str = "GIMBAL";
            break;
        case MIXER_Y6:
            str = "Y6";
            break;
        case MIXER_HEX6:
            str = "HEX6";
            break;
        case MIXER_FLYING_WING:
            str = "FLYING WING";
            break;
        case MIXER_Y4:
            str = "Y 4";
            break;
        case MIXER_HEX6X:
            str = "HEX 6X";
            break;
        case MIXER_OCTOX8:
            str = "OCTO X8";
            break;
        case MIXER_OCTOFLATP:
            str = "OCTOFLATP";
            break;
        case MIXER_OCTOFLATX:
            str = "MIXER_OCTOFLATX";
            break;
        case MIXER_AIRPLANE:
            str = "MIXER_AIRPLANE";
            break;
        case MIXER_HELI_120_CCPM:
            str = "MIXER_HELI_120_CCPM";
            break;
        case MIXER_HELI_90_DEG:
            str = "MIXER_HELI_90_DEG";
            break;
        case MIXER_VTAIL4:
            str = "MIXER_VTAIL4";
            break;
        case MIXER_HEX6H:
            str = "MIXER_HEX6H";
            break;
        case MIXER_PPM_TO_SERVO:
            str = "MIXER_PPM_TO_SERVO";
            break;
        case MIXER_DUALCOPTER:
            str = "MIXER_DUALCOPTER";
            break;
        case MIXER_SINGLECOPTER:
            str = "MIXER_SINGLECOPTER";
            break;
        case MIXER_ATAIL4:
            str = "MIXER_ATAIL4";
            break;
        case MIXER_CUSTOM:
            str = "MIXER_CUSTOM";
            break;
        case MIXER_CUSTOM_AIRPLANE:
            str = "MIXER_CUSTOM_AIRPLANE";
            break;
        case MIXER_CUSTOM_TRI:
            str = "MIXER_CUSTOM_TRI";
            break;
        case MIXER_QUADX_1234:
            str = "MIXER_QUADX_1234";
            break;

        default:
            str = "UNKNOWN";
            break;
    }
    return str;
}
#define square(x) ((x)*(x))
void msp_helper_pars_imu_data(const uint8_t *data, uint16_t len, uint16_t acc[4], uint16_t gyro[4], uint16_t mag[4])
{
    if (len != 18) {
        printf("Error pars data, wrong len\n");
        return;
    }

    int pos = 0;
    for (int j = 0; pos < 6; pos+=2, j++) {
        acc[j] = data[pos];
        acc[j] |= data[pos+1] << 8;
    }

    for (int j = 0; pos < 12; pos+=2, j++) {
        gyro[j] = data[pos];
        gyro[j] |= data[pos+1] << 8;
    }

    for (int j = 0; pos < 18; pos+=2, j++) {
        mag[j] = data[pos];
        mag[j] |= data[pos+1] << 8;
    }

#if 1
    printf("\nReceive IMU data\n");
    for (int i = 0; i < 3; i++) {
        printf("ACC[%d]=%f\n", i, (uint16_t)acc[i]/512.0);
    }

    for (int i = 0; i < 3; i++) {
        printf("GYRO[%d]=%f\n", i, ((uint16_t)gyro[i] * (4 / 16.4)));
    }

    for (int i = 0; i < 3; i++) {
        printf("MAG[%d]=%d\n", i, (uint16_t)mag[i]);
    }


#endif
}

void msp_helper_pars_attitude(const uint8_t *data, uint16_t len, int16_t *roll, int16_t *pitch, int16_t *yaw)
{
    if (len != 6)
        return;

    *roll = data[0] & 0xFF;
    *roll |= data[1] << 8;

    *pitch = data[2] & 0xFF;
    *pitch |= data[3] << 8;

    *yaw = data[4] & 0xFF;
    *yaw |= data[5] << 8;

}

void msp_helper_pars_rc_cmd(const uint8_t *msg, uint16_t len, uint16_t rc_data[MSP_MAX_RC_CHANNEL])
{
    //static uint16_t rc_data[MSP_MAX_RC_CHANNEL] = {0};
    memset(rc_data, 0, sizeof(uint16_t)*MSP_MAX_RC_CHANNEL);

    for (int i = 0, j = 0; i < len; i+=2, j++) {
        rc_data[j] = msg[i];
        rc_data[j] |= msg[i+1] << 8;
    }

#if 0  // Ford debug receive RC channel
    printf("\nRC channel\n");
    for (int i = 0; i < 4; i++) {
        printf("RC[%d]=%d\n", i, rc_data[i]);
    }

    printf("\nAUX channel\n");
    for (int i = 4; i < 16; i++) {
        printf("AUX[%d]=%d\n", i - 3, rc_data[i]);
    }
#endif

}

