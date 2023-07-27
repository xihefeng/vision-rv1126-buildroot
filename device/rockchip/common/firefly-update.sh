#!/bin/bash

#其他平台可能需要修改此处
version=$(readlink -f .repo/manifest.xml | awk -F '/' '{print $NF}')
SOC=$(echo $version | awk -F '_' '{print $1}')
if [ "$SOC" == "rv1126" ];then
	check=$(echo $version | grep ai_camera)
	if [ x"$check" == x ];then
		SOC="rv1126_rv1109"
	else
		SOC="rv1126_rv1109_ai"
	fi
fi
##################

list_path=".project.list"
all_cmd=".repo/repo/repo forall -c"
current_branch="current"
firefly_branch="$SOC/firefly"
rockchip_branch="$SOC/rockchip"
firefly="firefly-linux"
gitlab="firefly-gitlab"

ALL_OFF="\e[0m"
BOLD="\e[1m"
GREEN="${BOLD}\e[32m"
RED="${BOLD}\e[31m"
YELLOW="${BOLD}\e[33m"
BLUE="${BOLD}\e[34m"
pwd_path=""

#ignore error
IERRORS="no"

function gitt(){
	pro=$(pwd)
	if [ "$pwd_path" != "$pro" ];then
		pwd_path=$pro
		echo -e "${BOLD}#####################################################${ALL_OFF}"
		echo -e ""
		echo -e ""
		echo -e "${BOLD}#####################################################${ALL_OFF}"
		echo -e -n "${BOLD}# ${ALL_OFF}"
		echo -e "${BLUE}PRO: $pro ${ALL_OFF}"
		echo -e "${BOLD}#####################################################${ALL_OFF}"
	fi
	echo -e -n "${BOLD}# ${ALL_OFF}"
	echo -e "${YELLOW}CMD: git $@ ${ALL_OFF}"
	git $@
	ret="$?"
	if [ "$ret" != "0" ];then
		echo -e -n "${BOLD}# ${ALL_OFF}"
		echo -e "${RED}ERR: $ret ${ALL_OFF}"

		if [ "$IERRORS" = "no" ];then
			exit -1
		fi
	else
		echo -e -n "${BOLD}# ${ALL_OFF}"
		echo -e "${GREEN}PAS: $ret ${ALL_OFF}"
	fi
}

function project_list(){
	list_path=".project.list"
	manifest_file="../manifest.xml"
	save_file=$manifest_file
	cd .repo/manifests/
	while [ x"$save_file" != x ];
	do
		save_file=$(cat $save_file | grep "include name" |awk -F '"' '{print $2}')
		manifest_file="$manifest_file $save_file"
	done
	cd - > /dev/null

	rm -rf $list_path
	for i in $manifest_file
	do
		cat .repo/manifests/$i | grep -v "<!--"| grep "<project"  | while read line
		do
	        	check=$(echo $line | grep "path=")
	        	if [ x"$check" == x ];then
	           		pro=$(echo $line | grep "<project" | awk -F 'name' '{print $2}'| awk -F '"' '{print $2}')
	        	else
	               		pro=$(echo $line | grep "<project" | awk -F 'path' '{print $2}'| awk -F '"' '{print $2}')
	        	fi

	        	branch=$(echo $line | grep "<project" | awk -F 'dest-branch' '{print $2}'| awk -F '"' '{print $2}')
        		if [ x"$branch" == x ];then
	        		branch=$(echo $line | grep "<project" | awk -F 'revision' '{print $2}'| awk -F '"' '{print $2}')
        			if [ x"$branch" == x ];then
					branch=$SOC/firefly
				fi
			fi
			echo $pro $branch >> $list_path
		done
	done
}

function push_firefly(){
	project_list
	err_list=".push_firefly.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro
		if git branch | grep -q $firefly_branch; then
			gitt push $firefly $firefly_branch:$bra
		else
			exit -1
		fi
		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function push_gitlab(){
	project_list
	err_list=".push_gitlab.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro
		if git branch | grep -q $firefly_branch; then
			gitt push $gitlab $firefly_branch:$bra
		else
			exit -1
		fi
		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}


function create_release(){
	project_list
	release=$1
	err_list=".create_release.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $firefly_branch; then
			gitt checkout $firefly_branch
			gitt branch $release
			gitt push $firefly $release:$release
		else
			exit -1
		fi


		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function tag_release(){
	project_list
	branch=$1
	tag=$2
	err_list=".tag_release.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $branch; then
			gitt checkout $branch
			gitt tag $tag
			gitt checkout $firefly_branch
			gitt merge --no-ff $branch
			gitt push $firefly $tag
			gitt push $firefly $firefly_branch:$bra
		else
			exit -1
		fi


		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function delete_release(){
	project_list
	branch=$1
	err_list=".delete_release.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $firefly_branch; then
			gitt checkout $firefly_branch
		else
			exit -1
		fi

		if git branch --no-merged | grep -q repo_sync;then
			echo -e "# ${RED}ERR: Please check if the branch has been merged!${ALL_OFF}"
			exit -1
		fi

		gitt branch -D $branch
		gitt push $firefly :$branch

		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function pull_branch(){
	project_list
	branch=$1
	err_list=".pull_branch.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $branch; then
			gitt checkout $branch
			gitt pull $firefly $branch:$branch
		else
			exit -1
		fi


		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function push_branch(){
	project_list
	branch=$1
	err_list=".push_branch.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $branch; then
			gitt push $firefly $branch:$branch
		else
			exit -1
		fi


		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}


function tag_local_firefly(){
	project_list
	tag=$1
	err_list=".tag_local_firefly.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $firefly_branch; then
			gitt checkout $firefly_branch
		else
			exit -1
		fi
		gitt tag $tag

		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function tag_firefly(){
	project_list
	tag=$1
	err_list=".tag_firefly.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $firefly_branch; then
			gitt checkout $firefly_branch
		else
			exit -1
		fi
		gitt push $firefly $tag

		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function tag_gitlab(){
	project_list
	tag=$1
	err_list=".tag_gitlab.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $firefly_branch; then
			gitt checkout $firefly_branch
		else
			exit -1
		fi
		gitt push $gitlab $tag

		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function gitlab_remote_init(){
	project_list
	tag=$1
	err_list=".gitlab_remote_init.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git remote -v | grep -q $firefly; then
			url=$(git remote -v | grep $firefly | grep -v $gitlab | awk -F ' ' '{print $2}' | uniq | sed "s/.*rk-linux\/\(.*\)*/\1/")
			url="git@gitlab.com:firefly-linux/$url"
			gitt remote add $gitlab $url
		else
			exit -1
		fi

		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function reset_manifest(){
	if [ x"$1" != x ] && [ "$1" == "-a" ];then
		all=true
	else
		all=false
	fi

	manifest_path=".repo/manifests/"
	val=$(echo $version | awk -F '_' '{print $1}')
	cd $manifest_path
	if $all ; then
		xml_file=( $(find -name "*.xml" | sort) )
	else
		xml_file=( $(find -name "*.xml"| grep $val | grep -v old | sort) )
	fi
	echo ${xml_file[@]}| xargs -n1 | awk -F '/' '{print $NF}' | sed "=" | sed "N;s/\n/. /"
	read -p "Which would you like? [0]: " INDEX
	INDEX=$((${INDEX:-0} - 1))
	if echo $INDEX | grep -vq [^0-9]; then
	xml=${xml_file[$INDEX]}
	else
		exit -1
	fi
	cd - > /dev/null

	cd .repo
	ln -sf manifests/$xml manifest.xml
	cd - > /dev/null
}



function pull_firefly(){
	.repo/repo/repo sync -cd --no-tags
	if [ "$?" != "0" ];then
		exit -1
	fi

	if [ x"$1" != x ] && [ "$1" == "-f" ];then
		force=true
	else
		force=false
	fi

	project_list
	err_list=".pull_firefly.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if $force ;then
			if git branch | grep -q $firefly_branch;then
				gitt branch -D $firefly_branch
			fi
			gitt checkout -b $firefly_branch
		else
			if git branch | grep -q repo_sync; then
				gitt branch -D repo_sync
			fi
			gitt checkout -b repo_sync
		fi
		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list

}


function pull_rockchip(){
	project_list
	err_list=".pull_rockchip.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro
		if git branch | grep -q $rockchip_branch; then
			gitt pull $firefly $rockchip_branch:$rockchip_branch
		else
			gitt fetch $firefly $rockchip_branch
			gitt checkout -b $rockchip_branch $firefly/$rockchip_branch
		fi

		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function merge_rockchip(){
	project_list
	err_list=".merge_rockchip.list"
	if [ -f "$err_list" ];then
		echo -e "${YELLOW}注意：本次从上次执行失败的仓库开始继续执行! $pro ${ALL_OFF}"
		while_file="$err_list"
	else
		while_file="$list_path"
		cp $list_path $err_list
	fi

	while read line
	do
		pro=$(echo $line | awk -F ' ' '{print $1}')
		bra=$(echo $line | awk -F ' ' '{print $2}')
		cd $pro

		if git branch | grep -q $rockchip_branch; then
			gitt checkout $firefly_branch
			gitt merge --no-ff $rockchip_branch
		else
			exit -1
		fi

		cd - > /dev/null
		sed -i "1d" $err_list
	done < $while_file
	rm -rf $err_list
}

function usage(){
	echo "Usage:"
	echo "常用："
	echo "$0 pull-rockchip - 更新本地 SOC/rockchip 分支，SOC/rockchip 是上游更新分支没有经过任何改动"
	echo "$0 merge-rockchip - Merge SOC/rockchip 到 SOC/firefly"
	echo "$0 pull-firefly [-f] - 更新本地 SOC/firefly 分支，repo sync -c 更新后的最新提交, -f 强制合并到本地分支"
	echo "$0 push-firefly - 更新远程(内部) SOC/firefly 分支"
	echo "$0 push-gitlab - 更新远程(外部) SOC/firefly 分支"
	echo "$0 reset [-a] - 回退到某 manifest xml 版本"

	echo ""
	echo "发布使用："
	echo "$0 create-release release_branch - 根据当前 SOC/firefly 创建临时 release 版本"
	echo "$0 tag-release release_branch tag - 发布分支上打上标签"
	echo "$0 delete-release-branch release_branch - 删除本地和远程的分支"
	echo "$0 tag-firefly tag - 推送标签到远程(内部) firefly-linux"
	echo "$0 tag-gitlab tag - 推送标签到远程(外部) firefly-linux"

	echo ""
	echo "发布阶段调试："
	echo "$0 pull-branch branch - 拉取分支，但是所有仓库分支必须同名"
	echo "$0 push-branch branch - 推送分支，但是所有仓库分支必须同名"

	echo ""
	echo "不常用："
	echo "$0 tag-local-firefly tag - 本地 SOC/firefly 分支打标签"
	echo "$0 gitlab-remote-init - 初始化外部仓库 remote"

	echo -e "\nEnvironment variable："
	echo -e "\t\t\tdefault value\t\tnotes"
	echo -e "\tIERRORS\t\tno\t\tIgnore errors when set to yes"

}

para=$1

if [ "$para" == "pull-rockchip" ];then
	pull_rockchip
elif [ "$para" == "merge-rockchip" ];then
	merge_rockchip
elif [ "$para" == "pull-firefly" ];then
	pull_firefly $2
elif [ "$para" == "push-firefly" ];then
	push_firefly
elif [ "$para" == "push-gitlab" ];then
	push_gitlab
elif [ "$para" == "create-release" ];then
	if [ x"$2" == x ];then
		usage
		exit -1
	else
		release=$2
	fi

	create_release $release
elif [ "$para" == "delete-release-branch" ];then
	if [ x"$2" == x ];then
		usage
		exit -1
	else
		branch=$2
	fi

	delete_release $branch
elif [ "$para" == "pull-branch" ];then
	if [ x"$2" == x ];then
		usage
		exit -1
	else
		branch=$2
	fi

	pull_branch $branch
elif [ "$para" == "push-branch" ];then
	if [ x"$2" == x ];then
		usage
		exit -1
	else
		branch=$2
	fi

	push_branch $branch
elif [ "$para" == "tag-release" ];then
	if [ x"$2" == x ] && [ x"$3" == x ];then
		usage
		exit -1
	else
		tag=$3
		branch=$2
	fi

	tag_release $branch $tag
elif [ "$para" == "tag-local-firefly" ];then
	if [ x"$2" == x ];then
		usage
		exit -1
	else
		tag=$2
	fi

	tag_local_firefly $tag
elif [ "$para" == "tag-firefly" ];then
	if [ x"$2" == x ];then
		usage
		exit -1
	else
		tag=$2
	fi

	tag_firefly $tag
elif [ "$para" == "tag-gitlab" ];then
	if [ x"$2" == x ];then
		usage
		exit -1
	else
		tag=$2
	fi

	tag_gitlab $tag

elif [ "$para" == "reset" ];then
	reset_manifest $2
elif [ "$para" == "gitlab-remote-init" ];then
	gitlab_remote_init $2
else
	usage
fi
