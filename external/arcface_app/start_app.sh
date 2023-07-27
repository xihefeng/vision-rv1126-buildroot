#!/bin/bash
source /npu_arc/usr/bin/npu_arc_env.sh
ArcFaceServer_Path=/usr/share/arc/ArcFaceServer/
ArcFaceGO_Path=/usr/share/arc/ArcFaceGo/
data_path="/userdata"

if [ ! -d `$ArcFaceServer_Path` ]; then
  echo "error: ArcFaceServer_Path not exist"
fi

if [ ! -d `$ArcFaceGo_Path` ]; then
  echo "error: ArcFaceGo_Path not exist"
fi

cd ${ArcFaceGO_Path}
echo $PWD

source start.sh $1 >/dev/null 2>&1 &
wait $!

cd $ArcFaceServer_Path
echo $PWD

sleep 2

env_path="config/environment.ini"
if [ -f $env_path ]; then
  static_resource_path=`cat $env_path | grep "StaticResourcePath=" |awk -F '=' '{print $2}'`
  echo $static_resource_path
  public_path=${static_resource_path}/public
  echo $public_path
  if [ ! -d  $public_path ];then
    echo "before prepare"
    source prepare.sh  $static_resource_path  & 
  fi
else
    echo "before prepare"
	if [ -n "$data_path" ];then
		mkdir -p $data_path
		source prepare.sh $data_path &
	else
		source prepare.sh &
	fi
fi  

sh start_proc.sh 2> log_server.txt 






