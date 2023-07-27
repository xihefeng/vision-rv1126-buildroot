#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include "src/msp/msp_interface.h"
#include "src/msp/msp_api.h"
#include "src/msp/msp_protocol.h"
#include "src/msp/msp_protocol_helper.h"
#include "src/led/led.h"
#include "src/telemetry/bf_telemetry.h"
#include "src/syslog/logger.h"
#include "src/config/configini.h"


#define DEFAULT_CONFIG_FILE "/etc/msp.conf"
#define USER_CONFIG_FILE    "/sdcard/msp.conf"

#define SHM_KEY 0x1234
#define DEFAULT_UART "/dev/ttyS0"
#define UART_SPEED 1000000;

struct shmseg {
    int cnt;
    int w_complete;
    int r_complete;
    bf_telemetry_data_t bf_telemetry_data;
};

int shmid = -1;
struct shmseg *shmp;

static volatile bool run = true;
static volatile bool led_act = false;
static volatile bool ai_control = false;

static uint16_t send_rc_data[4] = {900};

void handle_sigint(int sig)
{
    SYS_LOG_INFO("Caught signal %d\n", sig);
    run = false;
}

void msp_message_callback(msp_msg_t * message)
{
    if (!led_act) {
        led_set_duration(50);
        led_act = true;
    }

    bf_telemetry_handler(message);

#if 0
    bf_telemetry_data_t *bf_telemetry_data = bf_telemetry_get_ptr();
    switch (message->cmd) {
        case MSP_RC: {
            uint16_t *rc = msp_helper_pars_rc_cmd(message->payload, message->size);
            send_rc_data[0] = rc[0];
            send_rc_data[1] = rc[1];
            send_rc_data[2] = rc[3];
            send_rc_data[3] = rc[2];

            if (rc[8] > 1500) {
                printf("AI Drone control is activated\n");
                ai_control = true;
            } else {
                ai_control = false;
            }
        }
            break;

        case MSP_FC_VARIANT:
            if (message->size == 4)
                memcpy(bf_telemetry_data->FC_ID, message->payload, message->size);
                printf("FC: %c%c%c%c\n", message->payload[0], message->payload[1], message->payload[2],
                       message->payload[3]);
            break;

        case MSP_API_VERSION:
            if (message->size == 3)
                printf("MSP ver: %d.%d.%d\n", message->payload[0], message->payload[1], message->payload[2]);
            break;

        case MSP_FC_VERSION:
            if (message->size == 3)
                printf("FC ver: %d.%d.%d\n", message->payload[0], message->payload[1], message->payload[2]);
            break;

        case MSP_MIXER_CONFIG:
            if (message->size == 2)
                printf("Type: '%s', motor: %s\n", msp_helper_get_str__of_mixer(
                        (message->payload[0])), message->payload[1] ? "Reversed" : "Normal");
            break;

        case MSP_RAW_IMU:
        {
            uint16_t acc[4] = {0};
            uint16_t gyro[4] = {0};
            uint16_t mag[4] = {0};
            msp_helper_pars_imu_data(message->payload, message->size, acc, gyro, mag);
        }
            break;

        case MSP_ATTITUDE:
        {
            int16_t roll = 0;
            int16_t pitch = 0;
            int16_t yaw = 0;
            msp_helper_pars_attitude(message->payload, message->size, &roll, &pitch, &yaw);
//            printf("\nAttitude\n");
//            printf("X: %.1f\n", (roll / 10.0));
//            printf("Y: %.1f\n", (pitch / 10.0));
//            printf("Z: %d\n\n", yaw);
            drone_control_update_attitude((roll / 10.0), (pitch / 10.0), yaw);
        }
            break;

        default:
#if 0 // for debug payload
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
#endif
}

int init_shared_mem(void)
{
    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);

    if (shmid == -1) {
        perror("Shared memory");
        return -1;
    }

    // Attach to the segment to get a pointer to it.
    shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1) {
        perror("Shared memory attach");
        return -1;
    }

    return 0;
}

void sync_shared_mem(void)
{
    bf_telemetry_data_t *data = bf_telemetry_get_ptr();
    if (!data && !shmp)
        return;

    if (shmp->r_complete == 1) {
        SYS_LOG_INFO("Reader has read data\n");
        return;
    }

    shmp->w_complete = 0;
    memcpy(&shmp->bf_telemetry_data, data, sizeof(bf_telemetry_data_t));
    shmp->w_complete = 1;
    shmp->cnt++;

}

int detach_shared_mem(void)
{
    if (shmdt(shmp) == -1) {
        perror("shmdt");
        return -1;
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl");
        return -1;
    }
    return 0;
}

void* receiver_task(void *arg)
{
    SYS_LOG_INFO("Start thread: '%s'\n", __FUNCTION__ );
    msp_interface_t *msp_interface = (msp_interface_t*)arg;

    while (run) {

        int ret = msp_interface_read(msp_interface);
        if (ret != MSP_INTERFACE_OK) {
            if (ret == MSP_INTERFACE_RX_TIME_OUT) {
                led_set_duration(500);
                led_act = false;
            } else {
                // another rx error
                fprintf(stderr, "Error receive\n");
            }
        }
    }

    SYS_LOG_INFO("Exit thread: '%s'\n", __FUNCTION__ );
    return NULL;
}

static void pars_config(Config *cfg)
{

}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);
    init_logger("MSP service");
    SYS_LOG_INFO("Start service\n");
    bool user_config = false;

    Config *cfg = NULL;
    if (ConfigReadFile(USER_CONFIG_FILE, &cfg) != CONFIG_OK) {
        SYS_LOG_INFO("User config not found: '%s'", USER_CONFIG_FILE);
    } else {
        user_config = true;
    }

    if (!user_config && ConfigReadFile(DEFAULT_CONFIG_FILE, &cfg) != CONFIG_OK) {
        SYS_LOG_INFO("Default config not found: '%s'", DEFAULT_CONFIG_FILE);
    }

    msp_interface_t msp_interface;

    if (argc > 1) {
        msp_interface.uart_name = argv[1];
        SYS_LOG_INFO("Use dev uart: %s\n", argv[1]);
    } else {
        msp_interface.uart_name = DEFAULT_UART;
        SYS_LOG_INFO("Use default uart: %s\n", DEFAULT_UART);
    }

    msp_interface.baud_rate = UART_SPEED;
    msp_interface.msp_state.callback = msp_message_callback;

    if (msp_interface_init(&msp_interface) != MSP_INTERFACE_OK) {
        SYS_LOG_ERROR("Error init MSP interface\n");
        return 1;
    }

    init_shared_mem();

    pthread_t rx_thread_id;
    pthread_create(&rx_thread_id, NULL, receiver_task, &msp_interface);

    bf_telemetry_init(&msp_interface);

    send_rc_data[0] = 1500;
    send_rc_data[1] = 1500;
    send_rc_data[2] = 1500;
    send_rc_data[3] = 1500;

    led_enable_blink();

    led_set_duration(500);

    uint8_t tx_buff[512] = {0};

    while (run) {

//        drone_control_rc_update(send_rc_data);
//        len = msp_api_request_attitude(tx_buff);
//        msp_interface_write(&msp_interface, tx_buff, len);
//        //usleep(5*1000);
//
//        len = msp_api_request_rc_position(tx_buff);
//        msp_interface_write(&msp_interface, tx_buff, len);
//
//        //usleep(5*1000);

        uint16_t len;

        len = msp_api_request_rc_position(tx_buff);
        msp_interface_write(&msp_interface, tx_buff, len);

        len = msp_api_send_rc_position(tx_buff, send_rc_data, MSP_NON_AUX_CHANNEL);
        msp_interface_write(&msp_interface, tx_buff, len);

        len = msp_api_request_attitude(tx_buff);
        msp_interface_write(&msp_interface, tx_buff, len);
        usleep(10*1000);

        sync_shared_mem();

    }

    led_disable_blink();

    pthread_join(rx_thread_id, NULL);

    detach_shared_mem();

    SYS_LOG_INFO("Stop service\n");

    return 0;
}