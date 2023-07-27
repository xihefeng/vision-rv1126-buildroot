#!/bin/bash

BUILDROOT=$(pwd)
TARGET=$1
NAME=$(whoami)
HOST=$(hostname)
DATETIME=`date +"%Y-%m-%d %H:%M:%S"`
if [[ $RK_ROOTFS_TYPE -eq "squashfs" ]]; then
	echo "# rootfs type is $RK_ROOTFS_TYPE, create ssh keys to $(pwd)/output/$RK_CFG_BUILDROOT/target/etc/ssh"
	ssh-keygen -A -f $(pwd)/output/$RK_CFG_BUILDROOT/target
fi

# echo "Build KERNEL modules"
# cd $BUILDROOT/../kernel
# make ARCH=arm  modules_install INSTALL_MOD_PATH="../deploy/modules"
# cd $BUILDROOT

KERNEL_MODULES_DIR=$BUILDROOT/../deploy/modules/lib/modules
if [ -d "$KERNEL_MODULES_DIR" ];
then
    echo "$KERNEL_MODULES_DIR directory exists."
	echo "Copy KERNEL modules to rootfs: $(pwd)/output/$RK_CFG_BUILDROOT/target/lib/" 
	cp -r "$KERNEL_MODULES_DIR" "$(pwd)/output/$RK_CFG_BUILDROOT/target/lib/"

else
	echo "$KERNEL_MODULES_DIR directory does not exist."
fi

echo "built by $NAME on $HOST at $DATETIME" > $TARGET/timestamp
exit 0
