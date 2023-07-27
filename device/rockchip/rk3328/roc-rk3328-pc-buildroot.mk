#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3328-buildroot.mk

# Kernel dts
export RK_KERNEL_DTS=rk3328-roc-pc

# PRODUCT MODEL
export RK_PRODUCT_MODEL=ROC_3328_PC
