#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/firefly-rk3399-ubuntu.mk

source $CUR_DIR/no-userdata.mk

# PRODUCT MODEL
export RK_PRODUCT_MODEL=FIREFLY_3399
