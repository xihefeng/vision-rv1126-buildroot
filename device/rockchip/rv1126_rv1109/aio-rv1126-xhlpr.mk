#!/bin/bash

CMD=`realpath $BASH_SOURCE`

CUR_DIR=`dirname $CMD`

source $CUR_DIR/aio-rv1126-jd4.mk

# Buildroot config
export RK_CFG_BUILDROOT=firefly_rv1126_rv1109_xhlpr
# OEM config
export RK_OEM_DIR=oem_ipc_firefly_xhlpr
# parameter for GPT table
export RK_PARAMETER=parameter-firefly-debian-fit.txt
