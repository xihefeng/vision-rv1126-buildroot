#!/bin/bash
#cd ArcfaceServer
function get_treefrog_id(){
	lsof -i:8900 | grep treefrog |head -1 |awk '{print $1}' 
}

function get_tadpole_id(){
	lsof -i:8900 | grep tadpole |tail -1 |awk '{print $1}'
}

function get_netserver_id(){
  ps -ef | grep netserver | grep -v grep | awk '{print $2}'
}

function get_storage_manager_id(){
  ps -ef | grep storage_manager | grep -v grep | awk '{print $2}'
}

function get_exec_result(){
  treefrog -d -e dev |awk '{print $1}'
}

while true;do
	pid=$(get_treefrog_id) 
  if [ -n "$pid" ];then
      kill -9 $pid
      pid=$(get_treefrog_id)
    	if [ -z "$pid" ];then
          break
    	else
    	  echo treefrog $pid 
    		sleep 1
    	fi
  else 
    break
  fi
done

while true;do
	pid=$(get_tadpole_id) 
  if [ -n "$pid" ];then
      kill -9 $pid
      pid=$(get_tadpole_id) 
    	if [ -z "$pid" ];then
          break
    	else
    	  echo tadpole $pid 
    		sleep 1
    	fi
  else 
    break
  fi
done

while true;do
	pid=$(get_netserver_id) 
  echo  $pid
  if [ -n $pid ];then
    break
  else 
    sleep 1
  fi
done

while true;do
	pid=$(get_storage_manager_id) 
  echo  $pid
  if [ -n $pid ];then
    break
  else 
    sleep 1
  fi
done

if [ -f "/usr/bin/treefrog" ]; then
	rm -rf /usr/bin/treefrog
fi 

cp ./tools/treefrog /usr/bin/
chmod +x /usr/bin/treefrog

if [ -f "/usr/bin/tadpole" ]; then
	rm -rf /usr/bin/tadpole
fi 

cp ./tools/tadpole /usr/bin/
chmod +x /usr/bin/tadpole

if [ -f "/usr/bin/tspawn" ]; then
  rm -rf /usr/bin/tspawn
fi

cp ./tools/tspawn /usr/bin/
chmod +x /usr/bin/tspawn

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/tools/
export QT_PLUGIN_PATH=$PWD/plugins

treefrog -d -e dev

sleep 2

pid=$(get_treefrog_id) 
if [ -z "$pid" ];then
  treefrog -d -e dev
  sleep 2
fi

exit