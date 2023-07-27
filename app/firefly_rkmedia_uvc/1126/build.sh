# SDK 目录
SDK_PATH="/home/lvsx/project/rv1126-ai/"

# 编译完 buildroot 后的 sysroot 目录
sysroot="$SDK_PATH/buildroot/output/firefly_rv1126_rv1109_uvcc/host/arm-buildroot-linux-gnueabihf/sysroot/"

# 交叉编译工具路径
compliecp="$SDK_PATH/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++"
compliecc="$SDK_PATH/prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc"

INCLUDE_PATH="-I $sysroot/usr/include/rknn  -I ./include/rkmedia -I ./include/easymedia  -I ./include/ -I ./uvc/ -I ./cJSON/ -I ./process/ -I ./include/uAPI/ -I ./include/xcore/ -I ./include/common/ -I ./include/algos/ -I ./include/ipc_server/ -I ./include/iq_parser/"

LIB_PATH="-L ./librkuvc/ -L ./libs/ -L ./librkmedia/"
OP="-Wl,--copy-dt-needed-entries"
LIBCC="-leasymedia -lpthread -lrknn_api -lrkuvc -lrockchip_mpp -lmjpeg_fps_ctr -ldrm -lrga -lrt -lv4l2"
LIBCP="$LIBCC"

SOURCE_CODE="
    uvc/uvc-gadget.c 
    uvc/uvc_video.cpp
    uvc/yuv.c
    uvc/uvc_control.c
    uvc/uvc_encode.cpp
    uvc/mpi_enc.c
    uvc/uevent.c
    uvc/drm.c
    cJSON/cJSON.c
    uvc/mpp_osd.c
    common/tcp_comm.c
    process/camera_control.cpp
    process/camera_pu_control.cpp
    common/sample_common_isp.c
    common/sample_fake_isp.c
"
if [ -f $1 ]
then
    file_name=$(echo $1 | awk -F '.' '{printf $1}')
    $compliecc  $OP $SOURCE_CODE $1  -o $file_name  $LIB_PATH $LIBCC $INCLUDE_PATH -D RKAIQ --sysroot=$sysroot
else
    echo $1 No such file!
fi
