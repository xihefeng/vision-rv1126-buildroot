cmake_minimum_required(VERSION 3.4.1)

if(COMPILES_FIREFLY_UVC_EXAMPLE)
project(firefly_rkmedia_uvc)
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

find_package(PkgConfig)

if(USE_RKAIQ)
    find_package(RkAiq REQUIRED)
    include_directories(${RKAIQ_INCLUDE_DIRS})
    add_definitions(-DRKAIQ)
endif()

set(SOURCE
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
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include/rknn
    ${CMAKE_SOURCE_DIR}/include/rkmedia
    ${CMAKE_SOURCE_DIR}/include/easymedia
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/uvc/
    ${CMAKE_SOURCE_DIR}/cJSON/
    ${CMAKE_SOURCE_DIR}/process/
    ${CMAKE_SOURCE_DIR}/include/uAPI
    ${CMAKE_SOURCE_DIR}/include/xcore
    ${CMAKE_SOURCE_DIR}/include/common
    ${CMAKE_SOURCE_DIR}/include/algos
    ${CMAKE_SOURCE_DIR}/include/ipc_server
    ${CMAKE_SOURCE_DIR}/include/iq_parser
)

add_definitions(-DCMAKE_C_FLAGS)
add_library(ff_uvc SHARED ${SOURCE})

#---------------------------------------------
#  firefly_rkmedia_vi_uvc_test
#---------------------------------------------
add_executable(firefly_rkmedia_vi_uvc_test firefly_rkmedia_vi_uvc_test.c ${SOURCE})
target_link_libraries(firefly_rkmedia_vi_uvc_test easymedia pthread drm rockchip_mpp rt rga ${CMAKE_SOURCE_DIR}/libs/libmjpeg_fps_ctr.a rknn_api ff_uvc v4l2 rkaiq)
install(TARGETS firefly_rkmedia_vi_uvc_test RUNTIME DESTINATION "bin")

#---------------------------------------------
#  firefly_rkmedia_vi_uvc_double_cameras_test
#---------------------------------------------
add_executable(firefly_rkmedia_vi_uvc_double_cameras_test firefly_rkmedia_vi_uvc_double_cameras_test.c ${SOURCE})
target_link_libraries(firefly_rkmedia_vi_uvc_double_cameras_test easymedia pthread drm rockchip_mpp rt rga ${CMAKE_SOURCE_DIR}/libs/libmjpeg_fps_ctr.a rknn_api ff_uvc v4l2 rkaiq)
install(TARGETS firefly_rkmedia_vi_uvc_double_cameras_test RUNTIME DESTINATION "bin")

install(TARGETS ff_uvc LIBRARY DESTINATION lib)

endif() #if(COMPILES_FIREFLY_UVC_EXAMPLE)
