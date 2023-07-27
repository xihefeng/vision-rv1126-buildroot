#!/bin/bash

CMD=`realpath $BASH_SOURCE`

CUR_DIR=`dirname $CMD`

source $CUR_DIR/cam-crv1109s2u-uvcc.mk

# Kernel dts
export RK_KERNEL_DTS=rv1126-firefly-jd4

# PRODUCT MODEL
export RK_PRODUCT_MODEL=AIO_1126_JD4
