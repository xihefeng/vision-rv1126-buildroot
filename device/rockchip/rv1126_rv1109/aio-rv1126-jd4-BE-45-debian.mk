#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/aio-rv1126-jd4-BE-45.mk

# parameter for GPT table
export RK_PARAMETER=parameter-firefly-debian-fit.txt

# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=ext4

# rootfs image path
export RK_ROOTFS_IMG=ubuntu_rootfs/rootfs.img

# Buildroot config
export RK_CFG_BUILDROOT=

# rootfs_system
export RK_ROOTFS_SYSTEM=ubuntu
