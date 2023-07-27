# 使用手册
* demo 目前只支持 CORE-1126-JD4/CORE-1109-JD4 板型。不支持智能双目摄像头模组 CAM-C1126S2U/CAM-C1109S2U。

编译 mk 文件选择
```
./build.sh device/rockchip/rv1126_rv1109/aio-rv1126-xhlpr.mk
```

* 由于默认 mk 文件会自动编译 XHLPR_APP 车牌识别。如不需要 XHLPR_APP 请保证 buildroot 已经关闭 XHLPR_APP 编译配置
```
BR2_PACKAGE_XHLPR_APP=n
```

* 请保证 buildroot 已经打开以下配置
```
# OPENCV3
BR2_PACKAGE_OPENCV3=y
BR2_PACKAGE_OPENCV3_LIB_CALIB3D=y
BR2_PACKAGE_OPENCV3_LIB_FEATURES2D=y
BR2_PACKAGE_OPENCV3_LIB_FLANN=y
BR2_PACKAGE_OPENCV3_LIB_HIGHGUI=y
BR2_PACKAGE_OPENCV3_LIB_IMGCODECS=y
BR2_PACKAGE_OPENCV3_LIB_IMGPROC=y
BR2_PACKAGE_OPENCV3_LIB_ML=y
BR2_PACKAGE_OPENCV3_LIB_OBJDETECT=y
BR2_PACKAGE_OPENCV3_LIB_PHOTO=y
BR2_PACKAGE_OPENCV3_LIB_SHAPE=y
BR2_PACKAGE_OPENCV3_LIB_STITCHING=y
BR2_PACKAGE_OPENCV3_LIB_SUPERRES=y
BR2_PACKAGE_OPENCV3_LIB_TS=y
BR2_PACKAGE_OPENCV3_LIB_VIDEOIO=y
BR2_PACKAGE_OPENCV3_LIB_VIDEO=y
BR2_PACKAGE_OPENCV3_LIB_VIDEOSTAB=y
BR2_PACKAGE_OPENCV3_WITH_FFMPEG=y
BR2_PACKAGE_OPENCV3_WITH_GSTREAMER1=y
BR2_PACKAGE_OPENCV3_WITH_JASPER=y
BR2_PACKAGE_OPENCV3_WITH_JPEG=y
BR2_PACKAGE_OPENCV3_WITH_PNG=y
BR2_PACKAGE_OPENCV3_WITH_PROTOBUF=y
BR2_PACKAGE_OPENCV3_WITH_TIFF=y
BR2_PACKAGE_OPENCV3_WITH_V4L=y
BR2_PACKAGE_OPENCV3_WITH_WEBP=y

# firefly_rockx_demo
BR2_PACKAGE_FIREFLY_ROCKX_DEMO=y
```

* PC Linux vlc 软件预览识别画面。需要将设备 ip 对应到链接。
```
vlc rtsp://<设备ip>:8554/H264_stream_0
```

* 可直接在浏览器输入设备 ip 即可预览识别画面（由于播放器的原因，可能会有些许卡顿）。默认账号和密码登录网页：
```
账号：admin
密码：admin
```

* 修改识别分辨率为 2K。修改文件 rockx_app.cfg ：
```
修改前：c:activation_conf_path=/usr/share/firefly_rockx_demo/licSever/activation.conf licSever_path=/usr/share/firefly_rockx_demo/licSever VideoNode=rkispp_scale1 VideoType=6 EnImageType=4 EnVideoWidth=1920 EnVideoHeight=1080 InImageType=4 OutImageType=18 InVideoWidth=720 InVideoHeight=576 OutVideoWidth=720 OutVideoHeight=576

修改后：c:activation_conf_path=/usr/share/firefly_rockx_demo/licSever/activation.conf licSever_path=/usr/share/firefly_rockx_demo/licSever VideoNode=rkispp_scale1 VideoType=6 EnImageType=4 EnVideoWidth=1920 EnVideoHeight=1080 InImageType=4 OutImageType=18 InVideoWidth=2688 InVideoHeight=1520 OutVideoWidth=2688 OutVideoHeight=1520
```

* 请确保 VI 接口（摄像头）没有被占用。请尝试执行：RkLunch-stop.sh 关闭摄像头服务。
* 手动参考示例执行 demo。或根据需求修改以下脚本然后执行启动。
```
# 默认启动车牌识别
/usr/share/firefly_rockx_demo/start_rockx_app.sh
```

* 手动执行示例 demo 则需要手动执行网页推流（不执行则网页无法预览识别结果）
```
# 只推流视频命令
ffmpeg -f rtsp -rtsp_transport tcp -i "rtsp://127.0.0.1:8554/H264_stream_0" -c  copy -f flv "rtmp://127.0.0.1/live/mainstream" &

# 同时推流视频和音频命令（目前只有运行示例 rockx_face_attribute 的 rockx_face_attribute_aenc_venc_rtsp_service 能执行以下命令）
ffmpeg -f rtsp -rtsp_transport tcp -i "rtsp://127.0.0.1:8554/H264_stream_0" -i "rtsp://127.0.0.1:8555/audio_stream_0" -c  copy -f flv "rtmp://127.0.0.1/live/mainstream" &
```

# 示例

* rockx_carplate
# 说明
* 车牌识别
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_carplate_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_carplate_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```

* rockx_face_attribute
# 说明
* 人脸特征点定位（只推流视频，无音频推流）
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_face_attribute_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_face_attribute_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```

* rockx_face_attribute
# 说明
* 人脸特征点定位（同时推流音频和视频）
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_face_attribute_aenc_venc_rtsp_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_face_attribute_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &

# ffmpeg 命令需要修改成以下命令
ffmpeg -f rtsp -rtsp_transport tcp -i "rtsp://127.0.0.1:8554/H264_stream_0" -i "rtsp://127.0.0.1:8555/audio_stream_0" -c  copy -f flv "rtmp://127.0.0.1/live/mainstream" &
```

* rockx_face_landmark
# 说明
* 人脸关键点识别
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_face_landmark_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_face_landmark_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```

* rockx_face_masks_detection
# 说明
* 人脸口罩检测
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_face_masks_detection_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_face_masks_detection_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```
* rockx_face_recognition
# 说明
* 人脸识别，需要手动输入图片。图片名字即识别显示 id。不支持中文 id 显示。
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_face_recognition_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_face_recognition_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg -i /path/to/id.jpg &
```

* rockx_head_detection
# 说明
* 人头检测
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_head_detection_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_head_detection_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```

* rockx_object_track
# 说明
* 人车物检测与追踪
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_object_track_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_object_track_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```

* rockx_person_detection
# 说明
* 人体识别
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_person_detection_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_person_detection_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```

* rockx_pose_body
# 说明
* 身体骨骼关键点定位
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_pose_body_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_pose_body_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```

* rockx_pose_finger
# 说明
* 手指关键点定位
# 代码路径
* app/firefly_rockx_demo/
# 快速使用
```
rockx_pose_finger_service -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
rockx_pose_finger_client -c /usr/share/firefly_rockx_demo/rockx_app.cfg &
```
