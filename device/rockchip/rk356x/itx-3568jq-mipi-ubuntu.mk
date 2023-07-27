#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3568-ubuntu.mk

# Kernel dts
export RK_KERNEL_DTS=rk3568-firefly-itx-3568jq-mipi101_M101014_BE45_A1
# PRODUCT MODEL
export RK_PRODUCT_MODEL=ITX_3568JQ
