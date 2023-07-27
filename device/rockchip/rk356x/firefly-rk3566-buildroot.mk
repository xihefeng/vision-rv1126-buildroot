#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/BoardConfig.mk

# Uboot defconfig
export RK_UBOOT_DEFCONFIG=firefly-rk3566
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=firefly_linux_defconfig
# packagefile for make update image 
export RK_PACKAGE_FILE=rk356x-package-file
# build idblock
export RK_IDBLOCK_UPDATE=true
# update spl
export RK_LOADER_UPDATE_SPL=true

# sd_parameter for GPT table
export RK_SD_PARAMETER=parameter-recovery.txt
# packagefile for make sdupdate image
export RK_SD_PACKAGE_FILE=rk356x-recovery-package-file
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk3566
# yocto machine
export RK_YOCTO_MACHINE=rockchip-rk3566-evb
# kernel image format type: fit(flattened image tree)
export RK_KERNEL_FIT_ITS=bootramdisk.its

export RK_USERDATA_FS_TYPE=ext4
