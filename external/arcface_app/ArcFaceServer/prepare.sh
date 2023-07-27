#!/bin/bash
if [ $# -eq 0 ];
then
	param_=$PWD # 全路�?
fi

if [ $# -eq 1 ];
then
	param_=$1 # 全路�?
fi


if [ ${param_: -1} == "/" ];
then
	where_you_want_to_place=${param_:0 : -1}
else 
	where_you_want_to_place=$param_
fi

echo "move public folder to " $where_you_want_to_place

tar -xvf ./tools/execs.tar -C /usr/bin/
chmod +x /usr/bin/treefrog
chmod +x /usr/bin/tadpole
chmod +x /usr/bin/tspawn

cp -rf public $where_you_want_to_place
chmod -R 777 $where_you_want_to_place/public

if [ -f "/etc/nginx/nginx.conf.bak" ]; then
	rm -f /etc/nginx/nginx.conf.bak
fi 

mv /etc/nginx/nginx.conf /etc/nginx/nginx.conf.bak
cp nginx.conf /etc/nginx/
sed -i  "s#static_resource_path#$where_you_want_to_place/public#g" /etc/nginx/nginx.conf
nginx -s reload

if [ -f "./config/environment.ini" ]; then
	rm -f ./config/environment.ini
fi 

cp -f  environment.ini config
sed -i  "s#static_resource_path#$where_you_want_to_place#g" config/environment.ini

tmp_dir=$where_you_want_to_place/tmp
mkdir -p $tmp_dir

if [ -f "./config/application.ini" ]; then
	rm -f ./config/application.ini
fi 

cp -f  application.ini config
sed -i  "s#tmp_path#$tmp_dir#g" config/application.ini

exit 0