#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/poll.h>
#include <time.h>
#include <unistd.h>

#include "serial.h"
#include "network.h"
#include "msp.h"

#define DEFAULT_SERIAL "/dev/ttyS0"
#define UDP_PORT 5612
#define ADDRESS "127.0.0.1"

#define MSEC_PER_SEC 1000

#define OSD_CHAR_X    60
#define OSD_CHAR_Y    22

static volatile sig_atomic_t quit = 0;
int socket_fd;

static void sig_handler(int _)
{
    quit = 1;
}

static void print_usage(char *s)
{
    printf("%s -s <serial port> -p <udp port>\n", s);
    printf("%s -h this message\n", s);
    exit(0);
}

static void rx_msp_callback(msp_msg_t *msp_message)
{
    //printf("%s: cmd: %d\n", __FUNCTION__, msp_message->cmd );
    static uint8_t message_buffer[256]; // only needs to be the maximum size of an MSP packet, we only care to fwd MSP

    if (msp_message->cmd == MSP_CMD_DISPLAYPORT) {
        uint16_t size = msp_data_from_msg(message_buffer, msp_message);
        if (write(socket_fd, message_buffer, size) < 0) {
            printf("Error send data\n");
        }
    }
//    uint16_t size = msp_data_from_msg(message_buffer, msp_message);
//    copy_to_msp_frame_buffer(message_buffer, size);
//    if(msp_message->payload[0] == MSP_DISPLAYPORT_DRAW_SCREEN) {
//        // Once we have a whole frame of data, send it to the goggles.
//        write(socket_fd, frame_buffer, fb_cursor);
//        DEBUG_PRINT("DRAW! wrote %d bytes\n", fb_cursor);
//        fb_cursor = 0;
//    }
}

static void send_display_size(int serial_fd) {
    uint8_t buffer[8];
    uint8_t payload[2] = {OSD_CHAR_X, OSD_CHAR_Y};
    construct_msp_command(buffer, MSP_CMD_SET_OSD_CANVAS, payload, 2, MSP_OUTBOUND);
    write(serial_fd, &buffer, sizeof(buffer));
}

static void send_variant_request(int serial_fd) {
    uint8_t buffer[6];
    construct_msp_command(buffer, MSP_CMD_FC_VARIANT, NULL, 0, MSP_OUTBOUND);
    write(serial_fd, &buffer, sizeof(buffer));
}

static void send_version_request(int serial_fd) {
    uint8_t buffer[6];
    construct_msp_command(buffer, MSP_CMD_API_VERSION, NULL, 0, MSP_OUTBOUND);
    write(serial_fd, &buffer, sizeof(buffer));
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_handler);

    char *serial_port = DEFAULT_SERIAL;
    int udp_port = UDP_PORT;
    int opt;
    struct pollfd poll_fds[2];

    while((opt = getopt(argc, argv, "psh")) != -1){
        switch(opt){
            case 'p':
                {
                    char *endptr;
                    udp_port = strtol(argv[optind], &endptr, 10);
                }
                break;
            case 's':
                serial_port = argv[optind];
                break;
            case 'h':
                print_usage(argv[0]);
                break;
            case '?':
                printf("unknown option: %c\n", optopt);
                print_usage(argv[0]);
                break;
        }
    }

    printf("Serial: '%s', UDP port: %d\n", serial_port, udp_port);

    int serial_fd = open_serial_port(serial_port, 115200);
    if (serial_fd < 0) {
        printf("Failed to open serial port!\n");
        return 1;
    }

    socket_fd = connect_to_server(ADDRESS, UDP_PORT);
    if (socket_fd <= 0) {
        printf("Failed to connect to %s:%d!\n", ADDRESS, UDP_PORT);
        return 1;
    }

    msp_state_t *rx_msp_state = calloc(1, sizeof(msp_state_t));
    rx_msp_state->cb = &rx_msp_callback;

    uint8_t update_rate_hz = 2;

    uint8_t serial_data[265] = {0};
    ssize_t serial_data_size;

    send_version_request(serial_fd);
    send_variant_request(serial_fd);
    send_display_size(serial_fd);

//    poll_fds[0].fd = serial_fd;
//    poll_fds[0].events = POLLIN;
//    poll(poll_fds, 2, ((MSEC_PER_SEC / update_rate_hz) / 2));

    while (!quit) {
        poll_fds[0].fd = serial_fd;
        poll_fds[0].events = POLLIN;
        poll(poll_fds, 2, ((MSEC_PER_SEC / update_rate_hz) / 2));

        // We got inbound serial data, process it as MSP data.
        if (0 < (serial_data_size = read(serial_fd, serial_data, sizeof(serial_data)))) {
            //printf("RECEIVED data! length %d\n", serial_data_size);
            for (ssize_t i = 0; i < serial_data_size; i++) {
                msp_process_data(rx_msp_state, serial_data[i]);
            }
        }
    }

    close(serial_fd);
    close(socket_fd);
    free(rx_msp_state);

    return 0;
}
