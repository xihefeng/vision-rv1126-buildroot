cmake_minimum_required(VERSION 3.1)
project(wfb C CXX)
set(CMAKE_C_STANDARD 99)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

option(PC_BUILD "" OFF)
option(BUILD_TX "" ON)
option(BUILD_RX "" ON)
option(BUILD_KEYGEN "" ON)
option(COPY_KEYS "" OFF)

set(CFLAGS "-Wall -O2")

set(CMAKE_C_FLAGS ${CFLAGS})
set(CMAKE_CXX_FLAGS ${CFLAGS})

set(CMAKE_SYSROOT $ENV{STAGING_DIR})
message(STATUS  CMAKE_SYSROOT: ${CMAKE_SYSROOT})

if (BUILD_TX)
    add_executable(${PROJECT_NAME}_tx
            src/tx.cpp
            src/fec.c
            src/wifibroadcast.cpp
    )
    target_link_libraries(${PROJECT_NAME}_tx pthread rt pcap sodium)
endif ()

if (BUILD_RX)
    add_executable(${PROJECT_NAME}_rx
            src/rx.cpp
            src/fec.c
            src/radiotap.c
            src/wifibroadcast.cpp)
    target_link_libraries(${PROJECT_NAME}_rx pthread rt pcap sodium)
endif ()

if (BUILD_KEYGEN)
    add_executable(${PROJECT_NAME}_keygen src/keygen.c)
    target_link_libraries(${PROJECT_NAME}_keygen -lsodium)
endif ()



if (PC_BUILD)
    message(STATUS "Build RX and keygen")
    add_executable(${PROJECT_NAME}_rx
            src/rx.cpp
            src/fec.c
            src/radiotap.c
            src/wifibroadcast.cpp)
    target_link_libraries(${PROJECT_NAME}_rx pthread rt pcap sodium)

    add_executable(${PROJECT_NAME}_keygen src/keygen.c)
    target_link_libraries(${PROJECT_NAME}_keygen -lsodium)
endif()


###########################################################################
# install step

# Install bin to /usr/bin/
if (BUILD_TX)
    install(TARGETS ${PROJECT_NAME}_tx DESTINATION bin)
endif()

if (BUILD_RX)
    install(TARGETS ${PROJECT_NAME}_rx DESTINATION bin)
endif()

if (BUILD_KEYGEN)
    install(TARGETS ${PROJECT_NAME}_keygen DESTINATION bin)
endif()


if (COPY_KEYS)
    set(KEYS_FILES
            "./keys/drone.key"
            "./keys/gs.key")

    set(KEYS_DES_DIR "/etc")


    # Set permissions RW RW RW
    install(FILES ${KEYS_FILES}
            PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ WORLD_WRITE
            DESTINATION "${KEYS_DES_DIR}")
endif()