#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3328-ubuntu.mk

# Kernel dts
export RK_KERNEL_DTS=rk3328-roc-cc
# DRM version
export RK_DRM_VERSION=2

# PRODUCT MODEL
export RK_PRODUCT_MODEL=ROC_3328_CC
