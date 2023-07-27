#!/bin/sh

led="/sys/class/leds/firefly:yellow:user/brightness"
path="/sys/kernel/debug/ffd00000.dwc3/mode"
adb_shell="/etc/init.d/S50usbdevice"
mode=$(cat $path)
if [ $mode == "device" ];then
	echo 0 > $led
	$adb_shell stop
	echo host > $path
elif [ $mode == "host" ];then
	echo 1 > $led
	echo device > $path
	$adb_shell start
fi

