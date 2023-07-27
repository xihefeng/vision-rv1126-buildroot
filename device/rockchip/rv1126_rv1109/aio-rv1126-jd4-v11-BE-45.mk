#!/bin/bash

CMD=`realpath $BASH_SOURCE`

CUR_DIR=`dirname $CMD`

source $CUR_DIR/aio-rv1126-jd4.mk

# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rv1126_firefly_v11_defconfig
# Kernel dts
export RK_KERNEL_DTS=rv1126-firefly-jd4-v11-BE-45
# Buildroot config
export RK_CFG_BUILDROOT=firefly_rv1126_rv1109_v11
