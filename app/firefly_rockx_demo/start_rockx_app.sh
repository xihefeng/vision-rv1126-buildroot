#!/bin/bash
source /etc/profile.d/RkEnv.sh
ROCKX_Path="/usr/share/firefly_rockx_demo/"

service_file="/usr/bin/rockx_carplate_service"
client_file="/usr/bin/rockx_carplate_client"

if [ ! -d $ROCKX_Path ]; then
  echo "error: ROCKX_Path not exist"
fi

if [ -f $service_file -a -f $client_file ];then
    /usr/bin/ff_provide_ip &
    $service_file -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
    sleep 2
    $client_file -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
    sleep 2
    ffmpeg -f rtsp -rtsp_transport tcp -i "rtsp://127.0.0.1:8554/H264_stream_0" -c  copy -f flv "rtmp://127.0.0.1/live/mainstream" &
fi
