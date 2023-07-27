#!/bin/bash

cmd="/usr/bin/rkaiq_switch_mode"
config="/etc/init.d/.ffswitchmode_config"
adc_night=$(cat $config | grep ADC_NIGHT | awk -F '=' '{print $2}')
adc_day=$(cat $config | grep ADC_DAY | awk -F '=' '{print $2}')
adc_dev=$(cat $config | grep ADC_DEV | awk -F '=' '{print $2}')
mode=""

board_version=$(cat /sys/firmware/devicetree/base/model | awk -F ' ' '{print $3}')
if [ $board_version = "v11" ];then
	echo 124 > /sys/class/gpio/export
	echo out > /sys/devices/platform/pinctrl/gpiochip3/gpio/gpio124/direction
	echo 1 > /sys/devices/platform/pinctrl/gpiochip3/gpio/gpio124/value

	echo 72 > /sys/class/gpio/export
	echo in > /sys/devices/platform/pinctrl/gpiochip2/gpio/gpio72/direction
fi

while true
do
	if [ -f /userdata/sysconfig.db ];then
		ircut_status=$(/usr/bin/sqlite3 /userdata/sysconfig.db "select value from SystemDeviceInfo where id=1")
		ircut_name=$(/usr/bin/sqlite3 /userdata/sysconfig.db "select name from SystemDeviceInfo where id=1")
		if [ $board_version = "v11" ];then
			gpio_judge=$(cat /sys/devices/platform/pinctrl/gpiochip2/gpio/gpio72/value)
		fi
		if [ $ircut_status -eq 1 ]&&[ $ircut_name = "ircutSwitch" ];then
			cur=$(cat "$adc_dev") 
			if [ $board_version = "v11" ];then
				if [ $gpio_judge -eq 1 ] && [ "$mode" != "day" ];then
					mode="day"
					$cmd "set" "off" > /dev/null
					v4l2-ctl -d /dev/v4l-subdev5  --set-ctrl 'band_stop_filter=1'
				elif [ $gpio_judge -eq 0 ] && [ "$mode" != "night" ];then
					mode="night"
					$cmd "set" "on" > /dev/null
					v4l2-ctl -d /dev/v4l-subdev5  --set-ctrl 'band_stop_filter=0'
				fi
			else
				if [ $cur -lt $adc_day ] && [ "$mode" != "day" ];then
					mode="day"
					$cmd "set" "off" > /dev/null
					v4l2-ctl -d /dev/v4l-subdev5  --set-ctrl 'band_stop_filter=1'
				elif [ $cur -gt $adc_night ] && [ "$mode" != "night" ];then
					mode="night"
					$cmd "set" "on" > /dev/null
					v4l2-ctl -d /dev/v4l-subdev5  --set-ctrl 'band_stop_filter=0'
				fi
			fi
		fi
	fi
	sleep 0.5
done
