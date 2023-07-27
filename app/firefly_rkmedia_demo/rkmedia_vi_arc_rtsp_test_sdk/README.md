# 首次执行步骤
1. adb 推送 sdk 包到板子
adb push rv1126_arc_test.tar.gz /

2. 使用扫描应用获取摄像头 IP 地址。

3. ssh 登录到摄像头 IP 地址。ssh root@<ip>。密码：firefly。

4. 解压压缩包。
tar xvf rv1126_arc_test.tar.gz

5. 配置授权码，http 服务端的地址 httpUrl=http://<设备 IP 地址>:5000/arc_face
vi /etc/ffarc.cfg # 授权码找业务拿

6. 手动关闭默认开启的摄像头应用
/oem/RkLunch-stop.sh

7. 执行环境配置脚本（含人脸注册；含执行网页服务器。注：网页服务器只能运行一次）
/userdata/rv1126_env.sh

8. 运行程序
/userdata/rv1126_run.sh



# 程序/脚本说明：
1. 手动创建 db 人脸数据库
/userdata/ffarc/table /userdata/ffarc/firefly.db

2. 环境配置脚本（含人脸注册，含执行网页服务器。人脸注册只运行一次，如果想要重新注册只能删掉 firefly.db 数据库，重新创建，重新注册）
/userdata/rv1126_env.sh

3. 自动运行程序
/userdata/rv1126_run.sh

* 获取推流预览地址：
1. 浏览器输入 http://<设备 ip 地址>:10008
2. 点击推流列表-->播放地址。默认预览地址：rtsp://<设备 ip 地址>:8554/mainmain

* Python 服务端应用
1. 目录：python_server/arcPywebservice.py
2. 配置：修改 python_server/ip_config.txt 配置服务端的ip地址和端口
3. 执行：python3 ./python_server/arcPywebservice.py

* 人脸识别分辨率切换
1. 修改 /userdata/rv1126_run.sh 选择 rkmedia_vi_arc_rtsp_test_2688_1520 或 rkmedia_vi_arc_rtsp_test_640_360 程序运行。分辨率选择分别为 2688x1520 和 640x360。

* 设置环境变量
export LD_LIBRARY_PATH=/userdata/ffarc/lib/

* 手动注册人脸
把 test_data 里的图片放到 /userdata/ffarc/ffarc_tmp/
cd /userdata/ffarc/ffarc_tmp/
/userdata/ffarc/bin/register 李四.jpg  王五.jpg  张三.jpg  赵六.jpg # 也可以添加其他人脸，这里适合使用脚本完成

* 运行服务器（注：必须运行服务器才能再运行人脸识别程序）
/userdata/EasyDarwin-linux-8.1.0-21102107/easydarwin &

* 运行人脸识别程序
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/userdata/ffarc/lib/
/userdata/ffarc/bin/rkmedia_vi_arc_rtsp_test_640_360 rtsp://127.0.0.1:8554/main

* 首次执行环境配置脚本会删除镜头的黑白和彩色效果。重启生效。

使用固件名称：AIO-RV1126_RV1109-JD4_IPC_2021_0904_1333
