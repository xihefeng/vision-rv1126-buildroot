* 请保证 buildroot 开启以下配置
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

BR2_PACKAGE_FIREFLY_RKMEDIA_DEMO=y
BR2_PACKAGE_ZBAR=y
```

* aio-1126-jd4 / aio-1109-jd4 板型适用以下 demo
```
ffrtsp_demo_test.cc
rkmedia_rtspget_multi_arc_test.cc
rkmedia_rtspget_multi_test.cc
rkmedia_rtspget_multi_venc_rtsp.cc
rkmedia_rtspget_vdec_rknn_venc_rtsp_test.cc
rkmedia_rtspget_vdec_test.cc
rkmedia_rtspget_vdec_venc_rtsp_test.cc
rkmedia_vdec_test.c
rkmedia_vdec_venc_rtsp_test.c
rkmedia_vi_rknn_venc_rtsp_test.c
rkmedia_vi_venc_rtsp_test.c
rkmedia_vi_zbar_test.c
```

* CAM-C1126S2U / CAM-C1109S2U 智能双目摄像头模组适用以下 demo
```
rkmedia_vdec_test.c
rkmedia_vi_double_cameras_zbar_test.c
```

# 注：所有 demo 默认使用 H264 视频流格式。使用到 VI 接口的 demo 请先关闭摄像头 ISP 服务。例：执行：RkLunch-stop.sh。

# rtsp 链接解释
```
示例链接：rtsp://admin:firefly123@168.168.100.100:554/av_stream
admin 是摄像头的账户名
firefly123 是摄像头的密码
168.168.100.100 是摄像头的 IP 地址
```

# 示例

* ffrtsp_demo_test
# 说明
* ffrtsp rtsp 拉流和推流测试。ffrtsp 拉取 rtsp 流不做任何处理然后推流
# 代码路径
* app/firefly_rkmedia_demo/ffrtsp_demo_test.cc
# 快速使用
```
ffrtsp_demo_test rtsp://admin:firefly123@168.168.100.100:554/av_stream
# PC 播放 rtsp 推流地址：rtsp://168.168.108.135:8554/H264_stream_0。注：168.168.108.135 是开发板的 IP 地址
```

* rkmedia_rtspget_multi_arc_test
# 说明
* 解码多路 rtsp 流并使用虹软算法进行人脸识别（需要获取虹软激活码，请联系商务）
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_rtspget_multi_arc_test.cc
# 快速使用
```
rkmedia_rtspget_multi_arc_test rtsp://admin:firefly123@168.168.100.99:554/av_stream rtsp://admin:firefly123@168.168.100.100:554/av_stream
```

* rkmedia_rtspget_multi_test
# 说明
* 解码多路 rtsp 流，将获得的多路 rtsp 流图像数据进行拼接，拼接结果显示于 MIPI 屏上
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_rtspget_multi_test.cc
# 快速使用
```
# rtsp 取流 2 个网络摄像头，每个摄像头取流 2 次，并输出显示到显示屏
rkmedia_rtspget_multi_test rtsp://admin:firefly123@168.168.100.94:554/av_stream rtsp://admin:firefly123@168.168.100.94:554/av_stream rtsp://admin:firefly123@168.168.100.97:554/av_stream rtsp://admin:firefly123@168.168.100.97:554/av_stream
```

* rkmedia_rtspget_multi_venc_rtsp
# 说明
* 解码多路 rtsp 流，将获得的多路 rtsp 流图像数据进行拼接，拼接结果并显示于 MIPI 屏上，同时将拼接结果进行编码推流 rtsp
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_rtspget_multi_venc_rtsp.cc
# 快速使用
```
# rtsp 取流 2 个网络摄像头，每个摄像头取流 2 次，摄像头分辨率为 2560x1440 。
rkmedia_rtspget_multi_venc_rtsp rtsp://admin:firefly123@168.168.100.94:554/av_stream rtsp://admin:firefly123@168.168.100.94:554/av_stream rtsp://admin:firefly123@168.168.100.97:554/av_stream rtsp://admin:firefly123@168.168.100.97:554/av_stream
```

* rkmedia_rtspget_vdec_rknn_venc_rtsp_test
# 说明
* 解码多路 rtsp 流并使用 NPU 进行 AI 推理，将识别结果进行多路编码推流
# ffrtsp-nn.cfg 文件路径 app/firefly_rkmedia_demo/tools/ffrtsp-nn.cfg 。ffrtsp-nn.cfg 两个网络摄像头的配置分别为
```
video_type=6 video_fps=25 width=1920 height=1080 image_type=4 port=8554 video_url=rtsp://admin:firefly123@168.168.100.94:554/av_stream
video_type=6 video_fps=25 width=1920 height=1080 image_type=4 port=8555 video_url=rtsp://admin:firefly123@168.168.100.97:554/av_stream
```
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_rtspget_vdec_rknn_venc_rtsp_test.cc
# 快速使用
```
rkmedia_rtspget_vdec_rknn_venc_rtsp_test -c /usr/share/ffrtsp-nn.cfg -p /usr/share/rknn_model/ssd_inception_v2_rv1109_rv1126.rknn -l /usr/share/rknn_model/coco_labels_list.txt -b /usr/share/rknn_model/box_priors.txt

#则 PC 端使用 VLC 预览 RTSP 推流画面命令为
vlc rtsp://168.168.101.208:8554/H264_stream_0
vlc rtsp://168.168.101.208:8555/H264_stream_1
```

* rkmedia_rtspget_vdec_test
# 说明
* 解码一路 rtsp 流并显示于 MIPI 屏上
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_rtspget_vdec_test.cc
# 快速使用
```
rkmedia_rtspget_vdec_test rtsp://admin:firefly123@168.168.100.94:554/av_stream
```

* rkmedia_rtspget_vdec_venc_rtsp_test
# 说明
* 解码一路 rtsp 流，将解码得到 rtsp 流图像数据进行编码推流 rtsp
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_rtspget_vdec_venc_rtsp_test.cc
# 快速使用
```
rkmedia_rtspget_vdec_venc_rtsp_test rtsp://admin:firefly123@168.168.100.94:554/av_stream
# PC 播放链接 rtsp://168.168.108.135/live/main_stream
```

* rkmedia_vdec_test
# 说明
* 输入文件进行解码显示
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_vdec_test.c
# 快速使用
```
rkmedia_vdec_test -i /input_file.h264
```

* rkmedia_vdec_venc_rtsp_test
# 说明
* 输入文件进行解码，将解码得到的图像数据推流 rtsp
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_vdec_venc_rtsp_test.c
# 快速使用
```
rkmedia_vdec_venc_rtsp_test -i /input_file.h264
# PC 播放链接 rtsp://168.168.108.135/live/main_stream
```

* rkmedia_vi_rknn_venc_rtsp_test
# 说明
* 摄像头获取图像数据，使用 NPU AI 推理图像数据，将识别结果编码推流 rtsp。由于没有使用 rga 过滤 VI 接口数据。预览 rtsp 流会有卡顿。
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_vi_rknn_venc_rtsp_test.c
# 快速使用
```
rkmedia_vi_rknn_venc_rtsp_test -a /oem/etc/iqfiles/ -c /oem/usr/share/rtsp-nn.cfg -b /oem/usr/share/rknn_model/box_priors.txt -l /oem/usr/share/rknn_model/coco_labels_list.txt -p /oem/usr/share/rknn_model/ssd_inception_v2_rv1109_rv1126.rknn
# PC 播放链接 rtsp://168.168.108.135/live/main_stream
```

* rkmedia_vi_venc_rtsp_test
# 说明
* 摄像头获取图像数据并编码推流 rtsp
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_vi_venc_rtsp_test.c
# 快速使用
```
rkmedia_vi_venc_rtsp_test -a /oem/etc/iqfiles/
# PC 播放链接 rtsp://168.168.108.135/live/main_stream
```

* rkmedia_vi_zbar_test
# 说明
* 二维码识别
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_ao_test.c
# 快速使用
```
rkmedia_vi_zbar_test -a /oem/etc/iqfiles/
```

* rkmedia_vi_double_cameras_zbar_test
# 说明
* 二维码识别
# 代码路径
* app/firefly_rkmedia_demo/rkmedia_vi_double_cameras_zbar_test.c
# 快速使用
```
rkmedia_vi_double_cameras_zbar_test -a /oem/etc/iqfiles/
```
