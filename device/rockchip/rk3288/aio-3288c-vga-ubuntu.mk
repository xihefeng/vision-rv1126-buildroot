#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3288-ubuntu.mk

# Kernel dts
export RK_KERNEL_DTS=rk3288-firefly-aioc-vga
# PRODUCT MODEL
export RK_PRODUCT_MODEL=AIO_3288C
