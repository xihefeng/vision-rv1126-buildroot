#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3399-buildroot.mk

# recovery ramdisk raw
export RK_RECOVERY_RAMDISK_RAW=recovery-arm64-raw.cpio.gz

# Uboot defconfig
export RK_UBOOT_DEFCONFIG=roc-rk3399-pc-plus
# Kernel dts
export RK_KERNEL_DTS=rk3399-roc-pc-pro
# DRM version
export RK_DRM_VERSION=2
# DRM version
export RK_DRM_VERSION=2

# PRODUCT MODEL
export RK_PRODUCT_MODEL=ROC_3399_PC_PRO
