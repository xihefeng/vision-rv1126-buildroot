#ifndef MSP_MSP_API_H
#define MSP_MSP_API_H
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

/** Create request message to FC */
uint16_t msp_api_request_fc_variant(uint8_t *buff);
uint16_t msp_api_request_api_version(uint8_t *buff);
uint16_t msp_api_request_fc_version(uint8_t *buff);
uint16_t msp_api_request_mixer_type(uint8_t *buff);

uint16_t msp_api_request_attitude(uint8_t *buff);
uint16_t msp_api_request_rc_position(uint8_t *buff);

/** Create message for FC */
uint16_t msp_api_send_rc_position(uint8_t *buff, uint16_t *rc_data, uint16_t num_channel);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif //MSP_MSP_API_H
