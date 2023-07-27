#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk356x-openwrt.mk

# Uboot defconfig
export RK_UBOOT_DEFCONFIG=firefly-rk3568
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=station_linux_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk3568-firefly-roc-pc
# PRODUCT MODEL
export RK_PRODUCT_MODEL=ROC_RK3568_PC

# Openwrt version select
export RK_OPENWRT_VERSION_SELECT=lede
# Openwrt defconfig
export RK_OPENWRT_DEFCONFIG=rk356x_config
