#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/aio-rv1126-jd4.mk

# parameter for GPT table
export RK_PARAMETER=parameter-firefly-debian-fit.txt
