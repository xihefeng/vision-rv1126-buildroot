#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3399-ubuntu.mk

# Kernel defconfig
export RK_KERNEL_DEFCONFIG=firefly_linux_defconfig
#export RK_KERNEL_DEFCONFIG=firefly_roc-rk3399-pc_defconfig

# Uboot defconfig
export RK_UBOOT_DEFCONFIG=roc-rk3399-pc-plus

# Kernel dts
export RK_KERNEL_DTS=rk3399-roc-pc-pro

# recovery ramdisk raw
export RK_RECOVERY_RAMDISK_RAW=recovery-arm64-raw.cpio.gz

# PRODUCT MODEL
export RK_PRODUCT_MODEL=ROC_3399_PC_PRO
