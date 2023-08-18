cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)
PROJECT(xhlpr_app)

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

set(XHLPR_SERVER_DEPENDENT_LIBS
    opencv_core
    opencv_highgui
    opencv_imgcodecs
    opencv_imgproc
)
set(XHLPR_ALGORITHM
    ${CMAKE_SOURCE_DIR}/lib/libLicClient.so
    ${CMAKE_SOURCE_DIR}/lib/libxhlpr_rknn.so
)

set(FIREFLY_RTSP_LIB
    ${CMAKE_SOURCE_DIR}/ffrtsp/libffrtsp.so
)

set(ROCKCHIP_RTSP_LIB
    ${CMAKE_SOURCE_DIR}/librtsp/librtsp.a
)

install(FILES ${XHLPR_ALGORITHM} DESTINATION lib)
install(FILES ${ROCKCHIP_RTSP_LIB} DESTINATION lib)

#---------------------------------------------
#  xhlpr_http
#---------------------------------------------
add_executable(xhlpr_http ${CMAKE_SOURCE_DIR}/src/xhlpr_http.cpp ${COMMON_SOURCE})
target_link_libraries(xhlpr_http ${COMMON_DEPENDENT_LIBS} curl)
install(TARGETS xhlpr_http RUNTIME DESTINATION "bin")

#---------------------------------------------
#  xhlpr_client
#---------------------------------------------
add_executable(xhlpr_client ${CMAKE_SOURCE_DIR}/src/xhlpr_client.cpp ${COMMON_SOURCE})
target_link_libraries(xhlpr_client ${COMMON_DEPENDENT_LIBS} ${XHLPR_ALGORITHM})
install(TARGETS xhlpr_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  xhlpr_service
#---------------------------------------------
add_executable(xhlpr_service ${CMAKE_SOURCE_DIR}/src/xhlpr_service.cpp ${COMMON_SOURCE} ${RKAIQ_SOURCE})
target_link_libraries(xhlpr_service ${COMMON_DEPENDENT_LIBS} ${XHLPR_SERVER_DEPENDENT_LIBS} ${ROCKCHIP_RTSP_LIB} rkaiq)
install(TARGETS xhlpr_service RUNTIME DESTINATION "bin")

#---------------------------------------------
#  rockx_client
#---------------------------------------------
add_executable(rockx_client ${CMAKE_SOURCE_DIR}/src/rockx_client.cpp ${COMMON_SOURCE})
target_link_libraries(rockx_client rockx ${COMMON_DEPENDENT_LIBS})
install(TARGETS rockx_client RUNTIME DESTINATION "bin")

#---------------------------------------------
#  ff_provide_ip
#---------------------------------------------
aux_source_directory(ff_netserver FF_NETSERVER_SRC)
add_executable(ff_provide_ip ${CMAKE_SOURCE_DIR}/${FF_NETSERVER_SRC})
target_link_libraries(ff_provide_ip pthread PkgConfig::DBUS PkgConfig::JSON-C PkgConfig::GLIB IPCProtocol gdbus MiniLogger::MiniLogger)
install(TARGETS ff_provide_ip RUNTIME DESTINATION "bin")

#install(DIRECTORY ./xhlpr_app DESTINATION include
#        FILES_MATCHING PATTERN "*.h")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/licSever DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/xhlpr_app/")
file(COPY start_xhlpr_app.sh DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/xhlpr_app/")
file(COPY xhlpr_app.cfg DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/xhlpr_app/")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/rockx-data-rv1109 DESTINATION "${PROJECT_SOURCE_DIR}/../../target/usr/share/")

if(EXISTS "S99_xhlpr")
        set(ETC_DST "${PROJECT_SOURCE_DIR}/../../target/etc/init.d/")
        file(COPY S99_xhlpr DESTINATION ${ETC_DST})
endif(EXISTS "S99_xhlpr")
