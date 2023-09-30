#!/bin/sh

modprobe g_ether
sleep 1
/etc/init.d/S50usbdevice restart
sleep 1
ifconfig usb0 192.168.55.1
ifconfig usb0 up
sleep 1
/etc/init.d/S80dhcp-server restart
