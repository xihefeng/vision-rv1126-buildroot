#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/BoardConfig_RK3308B_firefly.mk

# Uboot defconfig
export RK_UBOOT_DEFCONFIG=firefly-rk3308-debug-uart4
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=firefly-rk3308b_linux_defconfig 
# Kernel dts
export RK_KERNEL_DTS=rk3308b-roc-cc-plus-amic-ext_emmc
# parameter for GPT table
export RK_PARAMETER=parameter-64bit-ubuntu.txt
# packagefile for make update image
export RK_PACKAGE_FILE=rk3308-package-file-ubuntu
# rootfs image path
export RK_ROOTFS_IMG=rootfs/rk3308-ubuntu_rootfs.img
