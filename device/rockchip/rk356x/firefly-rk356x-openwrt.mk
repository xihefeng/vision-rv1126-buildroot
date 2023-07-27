#!/bin/bash

CMD=`realpath $BASH_SOURCE`
CUR_DIR=`dirname $CMD`

source $CUR_DIR/BoardConfig.mk

export RK_ROOTFS_SYSTEM=openwrt

# parameter for GPT table
export RK_PARAMETER=parameter-openwrt.txt
# packagefile for make update image
export RK_PACKAGE_FILE=rk356x-package-file-openwrt
# build idblock
export RK_IDBLOCK_UPDATE=true
# update spl
export RK_LOADER_UPDATE_SPL=true

# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=ext4
# recovery ramdisk
export RK_RECOVERY_RAMDISK=rk356x-recovery-arm64.cpio.gz
# recovery ramdisk raw 
export RK_RECOVERY_RAMDISK_RAW=rk356x-recovery-arm64-raw.cpio.gz
# Set userdata partition type
export RK_USERDATA_FS_TYPE=ext4
# kernel image format type: fit(flattened image tree)
export RK_KERNEL_FIT_ITS=bootramdisk.its

# Buildroot config
export RK_CFG_BUILDROOT=
# Recovery config
export RK_CFG_RECOVERY=
#OEM config
export RK_OEM_DIR=
#userdata config
export RK_USERDATA_DIR=
