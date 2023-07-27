#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3566-ubuntu.mk

# Kernel dts
export RK_KERNEL_DTS=rk3566-firefly-aiojd4-mipi_m10r800v2-cam_2ms2mf
# PRODUCT MODEL
export RK_PRODUCT_MODEL=AIO_3566_JD4
