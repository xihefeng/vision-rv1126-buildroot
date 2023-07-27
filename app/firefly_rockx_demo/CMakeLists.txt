cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)
PROJECT(firefly_rockx_demo)

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

SET(CMAKE_C_COMPILER "${PROJECT_SOURCE_DIR}/../../../../../prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc")
SET(CMAKE_CXX_COMPILER "${PROJECT_SOURCE_DIR}/../../../../../prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++")

find_package(PkgConfig)

if(USE_RKAIQ)
    find_package(RkAiq REQUIRED)
    include_directories(${RKAIQ_INCLUDE_DIRS})
    add_definitions(-DRKAIQ)
endif()

find_package(MiniLogger REQUIRED)
add_definitions(-DENABLE_MINILOGGER)

pkg_check_modules (GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules (DBUS REQUIRED IMPORTED_TARGET dbus-1)
pkg_check_modules (JSON-C REQUIRED IMPORTED_TARGET json-c)

option(SANITIZER_STATIC "compile with sanitizer (static library linker)" OFF)
if(SANITIZER_STATIC)
        add_definitions(-fsanitize=address -static-libasan -g -ggdb -gdwarf -funwind-tables -rdynamic -O0)
        add_definitions(-fno-stack-protector -fno-omit-frame-pointer -fsanitize-recover=address)
        # add_definitions(-fsanitize=undefined)
        target_link_libraries(udp_broadcast libasan.a dl m rt)
endif()

option(SANITIZER_DYNAMIC "compile with sanitizer (dynamic library linker)" OFF)
if(SANITIZER_DYNAMIC)
        # NOTE: copy libasan.so with manual operation
        add_definitions(-fsanitize=address -g -ggdb -gdwarf -funwind-tables -rdynamic -O0)
        add_definitions(-fno-stack-protector -fno-omit-frame-pointer -fsanitize-recover=address)
        # add_definitions(-fsanitize=undefined)
        target_link_libraries(udp_broadcast asan)
endif()

find_package(EasyMedia REQUIRED)
find_package(RockX REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/rkmedia
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/ffrtsp
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ff_netserver
)

set(COMMON_SOURCE
    common/sample_common_firefly_rkmedia.cpp
)

set(RKAIQ_SOURCE
    common/sample_common_isp.cpp
    common/sample_fake_isp.cpp
)

set(COMMON_DEPENDENT_LIBS
    EasyMedia::EasyMedia
    pthread
    rt
)

set(SERVER_DEPENDENT_LIBS
    opencv_core
    opencv_highgui
    opencv_imgcodecs
    opencv_imgproc
)

set(FIREFLY_RTSP_LIB
    ${CMAKE_SOURCE_DIR}/ffrtsp/libffrtsp.so
)

set(ROCKCHIP_RTSP_LIB
    ${CMAKE_SOURCE_DIR}/librtsp/librtsp.a
)

install(FILES ${ROCKCHIP_RTSP_LIB} DESTINATION lib)

#---------------------------------------------
#  rockx_carplate_client
#---------------------------------------------
add_executable(rockx_carplate_client ${CMAKE_SOURCE_DIR}/src/rockx_carplate_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_carplate_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_carplate_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_carplate_service
#---------------------------------------------
add_executable(rockx_carplate_service ${CMAKE_SOURCE_DIR}/src/rockx_carplate_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_carplate_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_carplate_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_pose_body_client
#---------------------------------------------
add_executable(rockx_pose_body_client ${CMAKE_SOURCE_DIR}/src/rockx_pose_body_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_pose_body_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_pose_body_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_pose_body_service
#---------------------------------------------
add_executable(rockx_pose_body_service ${CMAKE_SOURCE_DIR}/src/rockx_pose_body_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_pose_body_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_pose_body_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_pose_finger_client
#---------------------------------------------
add_executable(rockx_pose_finger_client ${CMAKE_SOURCE_DIR}/src/rockx_pose_finger_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_pose_finger_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_pose_finger_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_pose_finger_service
#---------------------------------------------
add_executable(rockx_pose_finger_service ${CMAKE_SOURCE_DIR}/src/rockx_pose_finger_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_pose_finger_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_pose_finger_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_head_detection_client
#---------------------------------------------
add_executable(rockx_head_detection_client ${CMAKE_SOURCE_DIR}/src/rockx_head_detection_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_head_detection_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_head_detection_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_head_detection_service
#---------------------------------------------
add_executable(rockx_head_detection_service ${CMAKE_SOURCE_DIR}/src/rockx_head_detection_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_head_detection_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_head_detection_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_attribute_client
#---------------------------------------------
add_executable(rockx_face_attribute_client ${CMAKE_SOURCE_DIR}/src/rockx_face_attribute_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_face_attribute_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_face_attribute_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_attribute_service
#---------------------------------------------
add_executable(rockx_face_attribute_service ${CMAKE_SOURCE_DIR}/src/rockx_face_attribute_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_face_attribute_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_face_attribute_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_attribute_aenc_venc_rtsp_service
#---------------------------------------------
add_executable(rockx_face_attribute_aenc_venc_rtsp_service ${CMAKE_SOURCE_DIR}/src/rockx_face_attribute_aenc_venc_rtsp_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_face_attribute_aenc_venc_rtsp_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx third_media)
install(TARGETS rockx_face_attribute_aenc_venc_rtsp_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_object_track_client
#---------------------------------------------
add_executable(rockx_object_track_client ${CMAKE_SOURCE_DIR}/src/rockx_object_track_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_object_track_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_object_track_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_object_track_service
#---------------------------------------------
add_executable(rockx_object_track_service ${CMAKE_SOURCE_DIR}/src/rockx_object_track_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_object_track_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_object_track_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_landmark_client
#---------------------------------------------
add_executable(rockx_face_landmark_client ${CMAKE_SOURCE_DIR}/src/rockx_face_landmark_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_face_landmark_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_face_landmark_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_landmark_service
#---------------------------------------------
add_executable(rockx_face_landmark_service ${CMAKE_SOURCE_DIR}/src/rockx_face_landmark_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_face_landmark_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_face_landmark_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_recognition_client
#---------------------------------------------
add_executable(rockx_face_recognition_client ${CMAKE_SOURCE_DIR}/src/rockx_face_recognition_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_face_recognition_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_face_recognition_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_recognition_service
#---------------------------------------------
add_executable(rockx_face_recognition_service ${CMAKE_SOURCE_DIR}/src/rockx_face_recognition_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_face_recognition_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_face_recognition_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_person_detection_client
#---------------------------------------------
add_executable(rockx_person_detection_client ${CMAKE_SOURCE_DIR}/src/rockx_person_detection_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_person_detection_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_person_detection_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_person_detection_service
#---------------------------------------------
add_executable(rockx_person_detection_service ${CMAKE_SOURCE_DIR}/src/rockx_person_detection_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_person_detection_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_person_detection_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_masks_detection_client
#---------------------------------------------
add_executable(rockx_face_masks_detection_client ${CMAKE_SOURCE_DIR}/src/rockx_face_masks_detection_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_face_masks_detection_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_face_masks_detection_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_face_masks_detection_service
#---------------------------------------------
add_executable(rockx_face_masks_detection_service ${CMAKE_SOURCE_DIR}/src/rockx_face_masks_detection_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(rockx_face_masks_detection_service ${COMMON_DEPENDENT_LIBS} ${SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq rockx)
install(TARGETS rockx_face_masks_detection_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  ff_provide_ip
#---------------------------------------------
aux_source_directory(ff_netserver FF_NETSERVER_SRC)
add_executable(ff_provide_ip ${CMAKE_SOURCE_DIR}/${FF_NETSERVER_SRC})
target_link_libraries(ff_provide_ip pthread PkgConfig::DBUS PkgConfig::JSON-C PkgConfig::GLIB IPCProtocol gdbus MiniLogger::MiniLogger)
install(TARGETS ff_provide_ip RUNTIME DESTINATION "bin")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/licSever DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/firefly_rockx_demo/")
file(COPY start_rockx_app.sh DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/firefly_rockx_demo/")
file(COPY rockx_app.cfg DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/firefly_rockx_demo/")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/rockx-data-rv1109 DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/")

if(EXISTS "S99_rockx")
        set(ETC_DST "${PROJECT_SOURCE_DIR}/../../target/etc/init.d/")
        file(COPY S99_rockx DESTINATION ${ETC_DST})
endif(EXISTS "S99_rockx")
