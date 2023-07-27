cmake_minimum_required(VERSION 3.1)
# if not set CMAKE_BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

project(msp C CXX)
set(CMAKE_C_STANDARD 99)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/config)

set(CMAKE_SYSROOT $ENV{STAGING_DIR})
message(STATUS  CMAKE_SYSROOT: ${CMAKE_SYSROOT})


add_executable(${PROJECT_NAME}
        src/main.c
        src/msp/msp.c
        src/msp/msp_api.c
        src/msp/msp_protocol_helper.c
        src/msp/msp_interface.c
        src/msp/msp_interface.h
        src/led/led.c
        src/telemetry/bf_telemetry.c
        src/syslog/logger.c
        src/config/configini.c)
target_link_libraries(${PROJECT_NAME} pthread )


add_executable(${PROJECT_NAME}_telemetry_client
        test/telemetry_client.c )
target_link_libraries(${PROJECT_NAME}_telemetry_client pthread )

###########################################################################
# install step

# Install to /usr/local/bin/
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
