#!/bin/bash

CMD=`realpath $BASH_SOURCE`                                                                                                                                                                                          
CUR_DIR=`dirname $CMD`

source $CUR_DIR/aio-rv1126-jd4-tb.mk

export RK_UBOOT_DEFCONFIG=rv1126-firefly-emmc-tb-v11

