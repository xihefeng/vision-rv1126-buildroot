#!/bin/bash

case "$1" in
start)
	touch /tmp/firefly.cluster.board
	while true
	do
		if [ -e "/tmp/firefly.cluster.board" ] ;then
			rm /tmp/firefly.cluster.board -rf
		else
			/etc/init.d/S50usbdevice restart
		fi
		sleep 70
	done
	;;
stop)
	;;
restart|reload)
	;;
*)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

exit 0
