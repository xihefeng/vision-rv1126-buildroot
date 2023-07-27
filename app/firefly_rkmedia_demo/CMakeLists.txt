cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)
PROJECT(firefly_rkmedia_demo)

include(FindPkgConfig)
pkg_check_modules(LIBDRM libdrm)
if(LIBDRM_FOUND)
  add_definitions(-DLIBDRM)
  include_directories(${LIBDRM_INCLUDE_DIRS})
  set(UVC_DEPENDENT_LIBS drm)
else()
pkg_check_modules(LIBION libion)
if(LIBION_FOUND)
  add_definitions(-DLIBION)
  include_directories(${LIBION_INCLUDE_DIRS})
  set(UVC_DEPENDENT_LIBS ion)
endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

SET(CMAKE_C_COMPILER "${PROJECT_SOURCE_DIR}/../../../../../prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc")
SET(CMAKE_CXX_COMPILER "${PROJECT_SOURCE_DIR}/../../../../../prebuilts/gcc/linux-x86/arm/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++")

find_package(PkgConfig)

if(USE_RKAIQ)
    find_package(RkAiq REQUIRED)
    include_directories(${RKAIQ_INCLUDE_DIRS})
    add_definitions(-DRKAIQ)
endif()

find_package(FREETYPE REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIR_freetype2})

find_package(EasyMedia REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/ffrtsp
    ${CMAKE_SOURCE_DIR}/ffsqlite
    ${CMAKE_SOURCE_DIR}/librtsp
    ${CMAKE_SOURCE_DIR}/arcsoft/inc
    ${CMAKE_SOURCE_DIR}/log_pri
    ${CMAKE_SOURCE_DIR}/base64
)

set(CC_RKAIQ_SOURCE
    common/sample_common_isp.cpp
    common/sample_fake_isp.cpp
)

set(C_RKAIQ_SOURCE
    common/sample_common_isp.c
    common/sample_fake_isp.c
)

set(LOG_PRI_SOURCE
    log_pri/log_pri.cc
)

set(BASE64_SOURCE
    base64/base64.cpp
)

set(COMMON_DEPENDENT_LIBS
    EasyMedia::EasyMedia
    pthread
)


set(OPENCV_LIBS
    opencv_core
    opencv_highgui
    opencv_imgcodecs
    opencv_imgproc
)

set(OPENCV_CHINESE_LIBS
    ${CMAKE_SOURCE_DIR}/lib/libopencv_freetype.so
    ${CMAKE_SOURCE_DIR}/lib/libharfbuzz.so
    ${CMAKE_SOURCE_DIR}/lib/libharfbuzz.so.0
    ${CMAKE_SOURCE_DIR}/lib/libharfbuzz.so.0.20600.4
    ${CMAKE_SOURCE_DIR}/lib/libharfbuzz-subset.so
    ${CMAKE_SOURCE_DIR}/lib/libharfbuzz-subset.so.0
    ${CMAKE_SOURCE_DIR}/lib/libharfbuzz-subset.so.0.20600.4
)

set(ARCSOFT_LIBS
    ${CMAKE_SOURCE_DIR}/arcsoft/lib/libarcsoft_face_engine.so
    ${CMAKE_SOURCE_DIR}/arcsoft/lib/libarcsoft_face.so
    ${CMAKE_SOURCE_DIR}/arcsoft/lib/libgomp.so.1
)

set(FIREFLY_RTSP_LIB
    ${CMAKE_SOURCE_DIR}/ffrtsp/libffrtsp.so
)

set(ROCKCHIP_RTSP_LIB
    ${CMAKE_SOURCE_DIR}/librtsp/librtsp.a
)

set(FFMPEG_RTSP_LIB
    ${CMAKE_SOURCE_DIR}/ffrtsp/libffmpeg_rtsp.so
)

set(FIREFLY_SQLITE_LIB
    ${CMAKE_SOURCE_DIR}/ffsqlite/libffsqlite.so
)

set(BASE64_LIB
    ${CMAKE_SOURCE_DIR}/base64/libbase_64.a
)

set(QT_RECEIVESERVER_BIN
    ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_arc_rtsp_test_sdk/rv1126_qt_bin/server_rv1126
)

install(FILES ${OPENCV_CHINESE_LIBS} DESTINATION lib)
install(FILES ${ARCSOFT_LIBS} DESTINATION lib)
install(FILES ${FIREFLY_RTSP_LIB} DESTINATION lib)
install(FILES ${ROCKCHIP_RTSP_LIB} DESTINATION lib)
install(FILES ${FFMPEG_RTSP_LIB} DESTINATION lib)
install(FILES ${FIREFLY_SQLITE_LIB} DESTINATION lib)
install(FILES ${QT_RECEIVESERVER_BIN} DESTINATION bin)

#---------------------------------------------
#  ffrtsp_demo_test
#---------------------------------------------
add_executable(ffrtsp_demo_test ${CMAKE_SOURCE_DIR}/ffrtsp_demo_test.cc)
target_link_libraries(ffrtsp_demo_test ${COMMON_DEPENDENT_LIBS} ${FIREFLY_RTSP_LIB})
install(TARGETS ffrtsp_demo_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_rtspget_arc_rtsp_test
#---------------------------------------------
add_executable(rkmedia_rtspget_arc_rtsp_test ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_arc_rtsp_test.cc ${LOG_PRI_SOURCE})
target_link_libraries(rkmedia_rtspget_arc_rtsp_test  ${COMMON_DEPENDENT_LIBS} ${ARCSOFT_LIBS} ${OPENCV_LIBS} ${OPENCV_CHINESE_LIBS} ${FIREFLY_SQLITE_LIB} ${FFMPEG_RTSP_LIB} rga freetype)
install(TARGETS rkmedia_rtspget_arc_rtsp_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_vi_arc_rtsp_test
#---------------------------------------------
add_executable(rkmedia_vi_arc_rtsp_test ${CMAKE_SOURCE_DIR}/rkmedia_vi_arc_rtsp_test.cc ${LOG_PRI_SOURCE} ${CC_RKAIQ_SOURCE} ${BASE64_SOURCE})
target_link_libraries(rkmedia_vi_arc_rtsp_test  ${COMMON_DEPENDENT_LIBS} ${ARCSOFT_LIBS} ${OPENCV_LIBS} ${OPENCV_CHINESE_LIBS} ${FIREFLY_SQLITE_LIB} ${FFMPEG_RTSP_LIB} ${BASE64_LIB} rga freetype rkaiq curl)
install(TARGETS rkmedia_vi_arc_rtsp_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_rtspget_multi_arc_test
#---------------------------------------------
add_executable(rkmedia_rtspget_multi_arc_test ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_multi_arc_test.cc)
target_link_libraries(rkmedia_rtspget_multi_arc_test ${COMMON_DEPENDENT_LIBS} ${ARCSOFT_LIBS} ${FIREFLY_RTSP_LIB} rga)
install(TARGETS rkmedia_rtspget_multi_arc_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_rtspget_multi_test
#---------------------------------------------
add_executable(rkmedia_rtspget_multi_test ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_multi_test.cc)
target_link_libraries(rkmedia_rtspget_multi_test ${COMMON_DEPENDENT_LIBS} ${FIREFLY_RTSP_LIB} rga)
install(TARGETS rkmedia_rtspget_multi_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_rtspget_multi_venc_rtsp
#---------------------------------------------
add_executable(rkmedia_rtspget_multi_venc_rtsp ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_multi_venc_rtsp.cc)
target_link_libraries(rkmedia_rtspget_multi_venc_rtsp ${COMMON_DEPENDENT_LIBS} ${FIREFLY_RTSP_LIB} rga)
install(TARGETS rkmedia_rtspget_multi_venc_rtsp RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_rtspget_vdec_rknn_venc_rtsp_test
#---------------------------------------------
add_executable(rkmedia_rtspget_vdec_rknn_venc_rtsp_test ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_vdec_rknn_venc_rtsp_test.cc)
target_link_libraries(rkmedia_rtspget_vdec_rknn_venc_rtsp_test ${COMMON_DEPENDENT_LIBS} ${FIREFLY_RTSP_LIB} ${OPENCV_LIBS} rga rknn_api)
install(TARGETS rkmedia_rtspget_vdec_rknn_venc_rtsp_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_rtspget_vdec_test
#---------------------------------------------
add_executable(rkmedia_rtspget_vdec_test ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_vdec_test.cc)
target_link_libraries(rkmedia_rtspget_vdec_test ${COMMON_DEPENDENT_LIBS} ${FIREFLY_RTSP_LIB})
install(TARGETS rkmedia_rtspget_vdec_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_rtspget_vdec_venc_rtsp_test
#---------------------------------------------
add_executable(rkmedia_rtspget_vdec_venc_rtsp_test ${CMAKE_SOURCE_DIR}/rkmedia_rtspget_vdec_venc_rtsp_test.cc)
target_link_libraries(rkmedia_rtspget_vdec_venc_rtsp_test ${COMMON_DEPENDENT_LIBS} ${FIREFLY_RTSP_LIB} ${ROCKCHIP_RTSP_LIB})
install(TARGETS rkmedia_rtspget_vdec_venc_rtsp_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_vdec_test
#---------------------------------------------
add_executable(rkmedia_vdec_test ${CMAKE_SOURCE_DIR}/rkmedia_vdec_test.c)
target_link_libraries(rkmedia_vdec_test ${COMMON_DEPENDENT_LIBS})
install(TARGETS rkmedia_vdec_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_vdec_venc_rtsp_test
#---------------------------------------------
add_executable(rkmedia_vdec_venc_rtsp_test ${CMAKE_SOURCE_DIR}/rkmedia_vdec_venc_rtsp_test.c)
target_link_libraries(rkmedia_vdec_venc_rtsp_test ${COMMON_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB})
install(TARGETS rkmedia_vdec_venc_rtsp_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_vi_rknn_venc_rtsp_test
#---------------------------------------------
add_executable(rkmedia_vi_rknn_venc_rtsp_test ${CMAKE_SOURCE_DIR}/rkmedia_vi_rknn_venc_rtsp_test.c ${C_RKAIQ_SOURCE})
target_link_libraries(rkmedia_vi_rknn_venc_rtsp_test ${COMMON_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rga rknn_api m rkaiq)
install(TARGETS rkmedia_vi_rknn_venc_rtsp_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_vi_venc_rtsp_test
#---------------------------------------------
add_executable(rkmedia_vi_venc_rtsp_test ${CMAKE_SOURCE_DIR}/rkmedia_vi_venc_rtsp_test.c ${C_RKAIQ_SOURCE})
target_link_libraries(rkmedia_vi_venc_rtsp_test ${COMMON_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq)
install(TARGETS rkmedia_vi_venc_rtsp_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_vi_zbar_test
#---------------------------------------------
add_executable(rkmedia_vi_zbar_test  ${CMAKE_SOURCE_DIR}/rkmedia_vi_zbar_test.c ${C_RKAIQ_SOURCE})
target_link_libraries(rkmedia_vi_zbar_test  ${COMMON_DEPENDENT_LIBS} zbar rkaiq)
install(TARGETS rkmedia_vi_zbar_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rkmedia_vi_double_cameras_zbar_test
#---------------------------------------------
add_executable(rkmedia_vi_double_cameras_zbar_test  ${CMAKE_SOURCE_DIR}/rkmedia_vi_double_cameras_zbar_test.c ${C_RKAIQ_SOURCE})
target_link_libraries(rkmedia_vi_double_cameras_zbar_test  ${COMMON_DEPENDENT_LIBS} zbar rkaiq)
install(TARGETS rkmedia_vi_double_cameras_zbar_test RUNTIME DESTINATION "bin")

file(COPY ${ARCSOFT_LIBS} DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/lib/")           
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tools/ DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/")
