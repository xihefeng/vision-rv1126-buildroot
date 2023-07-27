/**
 * @file telemetry_client.c this is part of project 'msp'
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
#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "src/telemetry/bf_telemetry.h"

#define SHM_KEY 0x1234

struct shmseg {
    int cnt;
    int w_complete;
    int r_complete;
    bf_telemetry_data_t bf_telemetry_data;
};

static volatile bool run = true;

void handle_sigint(int sig)
{
    printf("Caught signal %d\n", sig);
    run = false;
}

int main(int argc, char *argv[])
{
    int shmid;
    struct shmseg *shmp;
    shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("Shared memory");
        return 1;
    }

    // Attach to the segment to get a pointer to it.
    shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1) {
        perror("Shared memory attach");
        return 1;
    }

    int cnt = 0;
    while (run) {
        if (shmp->w_complete == 1 && shmp->cnt != cnt) {
            shmp->r_complete = 1;
            printf("Receive data: cnt %d\n", shmp->cnt);
            printf("FC ID: %s\n", shmp->bf_telemetry_data.FC_ID);
            printf("Attitude: Roll:'%.1f', Pitch:'%.1f' Yaw:'%d'\n",
                   shmp->bf_telemetry_data.attitude.roll / 10.0,
                   shmp->bf_telemetry_data.attitude.pitch / 10.0,
                   shmp->bf_telemetry_data.attitude.yaw);
            cnt = shmp->cnt;
            shmp->r_complete = 0;

        } else {
            usleep(10*1000);
        }

        //usleep(100*1000);
    }

    if (shmdt(shmp) == -1) {
        perror("shmdt");
        return 1;
    }

    return 0;
}