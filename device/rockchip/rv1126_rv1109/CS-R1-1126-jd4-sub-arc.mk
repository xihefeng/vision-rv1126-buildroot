#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/CS-R1-1126-jd4-sub-minimal.mk

# Buildroot config
export RK_CFG_BUILDROOT=firefly_rv1126_rv1109_cs_r1_arc
