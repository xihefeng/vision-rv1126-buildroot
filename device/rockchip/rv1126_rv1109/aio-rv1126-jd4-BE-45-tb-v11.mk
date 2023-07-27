#!/bin/bash

CMD=`realpath $BASH_SOURCE`

CUR_DIR=`dirname $CMD`

source $CUR_DIR/aio-rv1126-jd4-tb-v11.mk

export RK_UBOOT_DEFCONFIG=rv1126-firefly-emmc-tb-v11

# Kernel dts
export RK_KERNEL_DTS=rv1126-firefly-jd4-BE-45-tb

