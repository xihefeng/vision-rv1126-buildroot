#!/bin/bash
source /etc/profile.d/RkEnv.sh
XHLPR_Path="/usr/share/xhlpr_app/"

service_file="/usr/bin/xhlpr_service"
client_file="/usr/bin/xhlpr_client"
#client_file="/usr/bin/rockx_client"
http_file="/usr/bin/xhlpr_http"

if [ ! -d $XHLPR_Path ]; then
  echo "error: XHLPR_Path not exist"
fi

ps -ef | grep ntp_sync.sh | grep grep -v >> /dev/null
cnt=$?

while [ $cnt -eq 0 ]
do
        ps -ef | grep ntp_sync.sh | grep grep -v >> /dev/null
        cnt=$?
        sleep 1
done

if [ -f $service_file -a -f $client_file ];then
    /usr/bin/ff_provide_ip &
    $service_file -c /usr/share/xhlpr_app/xhlpr_app.cfg &
    sleep 2
    $client_file -c /usr/share/xhlpr_app/xhlpr_app.cfg &
    sleep 2
    if [ -f $http_file ];then
        $http_file -c /usr/share/xhlpr_app/xhlpr_app.cfg &
    fi
    ffmpeg -f rtsp -rtsp_transport tcp -i "rtsp://127.0.0.1:8554/H264_stream_0" -c  copy -f flv "rtmp://127.0.0.1/live/mainstream" &
fi

#sh start_proc.sh 2> log_server.txt 
