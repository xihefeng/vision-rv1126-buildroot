#!/bin/sh
#

check_linker()
{
	[ ! -L "$2" ] && ln -sf $1 $2
}

[ -f /etc/profile.d/enable_coredump.sh ] && source /etc/profile.d/enable_coredump.sh

check_linker /userdata   /oem/www/userdata
check_linker /userdata   /oem/www/userdata
check_linker /media/usb0 /oem/www/usb0
check_linker /mnt/sdcard /oem/www/sdcard

if [ ! -f "/oem/sysconfig.db" ]; then
  media-ctl -p -d /dev/media1 | grep 3840x2160
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-4K.db /oem/sysconfig.db
  fi
  media-ctl -p -d /dev/media1 | grep 2688x1520
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-2K.db /oem/sysconfig.db
  fi
  media-ctl -p -d /dev/media1 | grep 1920x1080
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-1080P.db /oem/sysconfig.db
  fi
  media-ctl -p -d /dev/media1 | grep 2592x1944
  if [ $? -eq 0 ] ;then
    ln -s -f /oem/sysconfig-5M.db /oem/sysconfig.db
  fi
fi

#set max socket buffer size to 1.5MByte
sysctl -w net.core.wmem_max=1572864

export HDR_MODE=1
export enable_encoder_debug=0

#vpu 600M, kernel default 600M
#echo 600000000 >/sys/kernel/debug/mpp_service/rkvenc/clk_core

ipc-daemon --no-mediaserver &
sleep 2
