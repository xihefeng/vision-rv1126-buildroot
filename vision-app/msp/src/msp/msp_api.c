#include <string.h>
#include "msp_api.h"
#include "msp.h"
#include "msp_protocol.h"


#define ASSERT_NULL(x)      \
    do {                    \
         if (x == NULL) {   \
            return 0;       \
         }                  \
    } while(0)

static msp_msg_t msp_msg = {0};

static msp_msg_t* create_request(uint8_t cmd)
{
    memset(&msp_msg, 0, sizeof (msp_msg_t));
    msp_msg.direction = MSP_OUTBOUND;
    msp_msg.cmd = cmd;
    return &msp_msg;
}

uint16_t msp_api_request_fc_variant(uint8_t *buff)
{
    ASSERT_NULL(buff);
    return msp_data_from_msg(buff, create_request(MSP_FC_VARIANT));
}

uint16_t msp_api_request_api_version(uint8_t *buff)
{
    ASSERT_NULL(buff);
    return msp_data_from_msg(buff, create_request(MSP_API_VERSION));
}

uint16_t msp_api_request_fc_version(uint8_t *buff)
{
    ASSERT_NULL(buff);
    return msp_data_from_msg(buff, create_request(MSP_FC_VERSION));
}

uint16_t msp_api_request_mixer_type(uint8_t *buff)
{
    ASSERT_NULL(buff);
    return msp_data_from_msg(buff, create_request(MSP_MIXER_CONFIG));
}


uint16_t msp_api_request_attitude(uint8_t *buff)
{
    ASSERT_NULL(buff);
    return msp_data_from_msg(buff, create_request(MSP_ATTITUDE));
}

uint16_t msp_api_request_rc_position(uint8_t *buff)
{
    ASSERT_NULL(buff);
    return msp_data_from_msg(buff, create_request(MSP_RC));
}

static void put_u16(uint8_t *msg, uint16_t pos, uint16_t x)
{
    msg[pos++] = (x & 0xff);
    msg[pos] = ((x >> 8) & 0xff);
}

uint16_t msp_api_send_rc_position(uint8_t *buff, uint16_t *rc_data, uint16_t num_channel)
{
    ASSERT_NULL(rc_data);
    ASSERT_NULL(buff);

    msp_msg_t msp_message = {0};
    msp_message.direction = MSP_OUTBOUND;
    msp_message.cmd = MSP_SET_RAW_RC;
    msp_message.size = num_channel * 2;

    int pos = 0;
    for (int i = 0; i < msp_message.size; i+=2) {
        put_u16(msp_message.payload, i, rc_data[pos]);
        pos++;
    }

    return msp_data_from_msg(buff, &msp_message);
}
