#!/bin/bash
function getpid(){
	ps -ef | grep ArcFaceGo | grep -v grep | awk '{print $2}'
}

function startApp(){
	echo start ArcFaceGo
  DIR=$(cd $(dirname  ${BASH_SOURCE[0]}); pwd)
  export LD_LIBRARY_PATH=$DIR:$LD_LIBRARY_PATH
	export QT_QPA_FB_DRM=1
	export QT_QPA_PLATFORM=linuxfb:rotation=$1
  $DIR/ArcFaceGo -d $1 >/dev/null &
}

if [ -n "$1" ];then
rotation=$1
else
rotation=0
fi

pid=$(getpid)
if [ -n "$pid" ];then
	echo "kill ArcFaceGo"
	kill -9 $pid
	while true;do
		if [ -z $(getpid) ];then
			startApp $rotation
			break;
		else
			sleep 1
		fi
	done 
else
	startApp $rotation
fi

