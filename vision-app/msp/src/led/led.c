/**
 * @file led.c this is part of project 'msp'
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
 * Created vitalii.nimych@gmail.com 13-06-2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "led.h"

#define STATUS_LED_PATH "/sys/class/leds/green/"

static int open_file(char *path)
{
    int fd = open(path, O_RDWR);

    if (fd < 0) {
        printf("Cannot init LED\n");
        return -1;
    }
    return fd;
}

int led_enable(bool state)
{
    char *led_path = STATUS_LED_PATH "brightness";
    int fd = open(led_path, O_RDWR);

    if (fd < 0) {
        printf("Cannot init LED\n");
        return -1;
    }

    if (state)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);

    return 0;
}

int led_set_duration(uint16_t duration)
{
    char *led_path = STATUS_LED_PATH "delay_on";
    int fd = open(led_path, O_RDWR);
    if (fd < 0) {
        printf("Cannot init LED\n");
        return -1;
    }
    char str[10];
    sprintf(str, "%d", duration);
    write(fd, str, strlen(str));
    close(fd);

    led_path = STATUS_LED_PATH "delay_off";
    fd = open(led_path, O_RDWR);
    if (fd < 0) {
        printf("Cannot init LED\n");
        return -1;
    }

    write(fd, str, strlen(str));
    close(fd);

    return 0;
}

int led_enable_blink(void)
{
    char *led_path = STATUS_LED_PATH "trigger";
    int fd = open(led_path, O_RDWR);

    if (fd < 0) {
        printf("Cannot init LED\n");
        return -1;
    }

    write(fd, "timer", strlen("timer"));

    close(fd);

    return 0;
}

int led_disable_blink(void)
{
    char *led_path = STATUS_LED_PATH "trigger";
    int fd = open(led_path, O_RDWR);

    if (fd < 0) {
        printf("Cannot init LED\n");
        return -1;
    }

    write(fd, "none", strlen("none"));

    close(fd);

    return 0;
}
