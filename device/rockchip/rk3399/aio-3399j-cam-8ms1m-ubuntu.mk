#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3399-ubuntu.mk

# Kernel dts
export RK_KERNEL_DTS=rk3399-firefly-aio-cam-8ms1m

# PRODUCT MODEL
export RK_PRODUCT_MODEL=CORE_3399J
