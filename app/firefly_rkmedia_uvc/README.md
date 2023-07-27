# 该 demo 只支持 sdk-ai 。 sdk-ai 源码获取：https://wiki.t-firefly.com/zh_CN/CAM-C11092U/Source_code.html。
# 由于配置项众多，目前已将 demo 固化成 mk 文件，使用 mk 文件一键编译生成 demo 固件
# 固件编译命令
```
./build.sh device/rockchip/rv1126_rv1109/aio-rv1126-rkmedia-uvcc.mk
./build.sh
```

# 将生成的固件烧录进设备，开机脚本 /etc/init.d/S58_lunch_init 默认启动 demo。不需要对设备执行任何操作。只需要执行 Linux host 端的命令即可。
# 由于 demo 未完善。Linux host 端执行一次 sudo ./client 之后需要重启板子设备才能再次打开 demo 。

# 相关命令解释：
```
# 启动复合设备
/oem/usb_config.sh rndis
```

# 单目摄像头 demo : firefly_rkmedia_vi_uvc_test.c
# 双目摄像头 demo : firefly_rkmedia_vi_uvc_double_cameras_test.c

---host---
编译：
```
cd ./host
./build.sh
```

host 端执行：
```
sudo ./client
```

---rv1126---
编译：
```
cd ./1126
./build.sh firefly_rkmedia_vi_uvc_test.c
./build.sh firefly_rkmedia_vi_uvc_double_cameras_test.c
```


如果使用手动配置修改需要注意的文件：
```
1.aio-rv1126-jd4 需要修改 /oem/usb_config.sh --> 将 eth0 修改成 eth2
2.CAM-C1109S2U 需要修改 /oem/usr/share/rtsp-nn.cfg --> 最后一行修改为：path=/live/main_stream video_type=7 width=1920 height=1080 image_type=4 video_path=rkispp_scale1
3.需要移除文件 /etc/init.d/S58_lunch_init
```

