#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/BoardConfig.mk

# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rk3399pro
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=firefly3399pro_linux_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk3399pro-firefly-aioc
# packagefile for make update image
export RK_PACKAGE_FILE=rk3399-package-file

# sd_parameter for GPT table
export RK_SD_PARAMETER=parameter-recovery.txt
# packagefile for make sdupdate image
export RK_SD_PACKAGE_FILE=rk3399-recovery-package-file

export RK_USERDATA_FS_TYPE=ext4

# PRODUCT MODEL
export RK_PRODUCT_MODEL=AIO_3399PROC
