/* -*- c -*-
 */
// Copyright (C) 2017 - 2022 Vasily Evseenko <svpcom@p2ptech.org>

/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 3.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <stdio.h>
#include <sodium.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/random.h>

static inline void print_banner(void)
{
    fprintf(stderr, "__          ________ ____                      \n");
    fprintf(stderr, "\\ \\        / /  ____|  _ \\                     \n");
    fprintf(stderr, " \\ \\  /\\  / /| |__  | |_) |                    \n");
    fprintf(stderr, "  \\ \\/  \\/ / |  __| |  _ <       _             \n");
    fprintf(stderr, "   \\  /\\  /  | |    | |_) |     | |            \n");
    fprintf(stderr, "  __\\/  \\/_ _|_|   _|____/_ __ _| |_ ___  _ __ \n");
    fprintf(stderr, " / _` |/ _ \\ '_ \\ / _ \\ '__/ _` | __/ _ \\| '__|\n");
    fprintf(stderr, "| (_| |  __/ | | |  __/ | | (_| | || (_) | |   \n");
    fprintf(stderr, " \\__, |\\___|_| |_|\\___|_|  \\__,_|\\__\\___/|_|   \n");
    fprintf(stderr, "  __/ |                                        \n");
    fprintf(stderr, " |___/                                         \n");

}

int main(void)
{
    unsigned char drone_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char drone_secretkey[crypto_box_SECRETKEYBYTES];
    unsigned char gs_publickey[crypto_box_PUBLICKEYBYTES];
    unsigned char gs_secretkey[crypto_box_SECRETKEYBYTES];
    FILE *fp;
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    fprintf(stderr, "Generate new keys\n");
    print_banner();

    {
        int fd;
        int c;

        if ((fd = open("/dev/random", O_RDONLY)) != -1) {
            if (ioctl(fd, RNDGETENTCNT, &c) == 0 && c < 160) {
                fprintf(stderr, "This system doesn't provide enough entropy to quickly generate high-quality random numbers.\n"
                        "Installing the rng-utils/rng-tools, jitterentropy or haveged packages may help.\n"
                        "On virtualized Linux environments, also consider using virtio-rng.\n"
                        "The service will not start until enough entropy has been collected.\n");
            }
            (void) close(fd);
        }
    }

    if (sodium_init() < 0)
    {
        fprintf(stderr, "Libsodium init failed\n");
        return 1;
    }

    if (crypto_box_keypair(drone_publickey, drone_secretkey) !=0 ||
        crypto_box_keypair(gs_publickey, gs_secretkey) != 0)
    {
        fprintf(stderr, "Unable to generate keys\n");
        return 1;
    }

    if((fp = fopen("drone.key", "w")) == NULL)
    {
        perror("Unable to save drone.key");
        return 1;
    }

    fwrite(drone_secretkey, crypto_box_SECRETKEYBYTES, 1, fp);
    fwrite(gs_publickey, crypto_box_PUBLICKEYBYTES, 1, fp);
    fclose(fp);

    fprintf(stderr, "Drone keypair (drone sec + gs pub) saved to '%s/drone.key'\n", cwd);

    if((fp = fopen("gs.key", "w")) == NULL)
    {
        perror("Unable to save gs.key");
        return 1;
    }

    fwrite(gs_secretkey, crypto_box_SECRETKEYBYTES, 1, fp);
    fwrite(drone_publickey, crypto_box_PUBLICKEYBYTES, 1, fp);
    fclose(fp);

    fprintf(stderr, "GS keypair (gs sec + drone pub) saved to '%s/gs.key'\n", cwd);
    fprintf(stderr, "Make backup of your keys and copy to /etc/gs.key and /etc/drone.key\nExample:\n");
    fprintf(stderr, "mv /etc/gs.key /etc/gs.key.bk \n");
    fprintf(stderr, "mv /etc/drone.key /etc/drone.key.bk \n");
    fprintf(stderr, "cp %s/gs.key /etc/gs.key \n", cwd);
    fprintf(stderr, "cp %s/drone.key /etc/drone.key \n", cwd);
    return 0;
}
