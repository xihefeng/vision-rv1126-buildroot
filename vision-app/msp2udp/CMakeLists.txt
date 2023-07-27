cmake_minimum_required(VERSION 3.1)

# if not set CMAKE_BUILD_TYPE
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

project(msp2udp C)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} main.c serial.c network.c msp.c)


###########################################################################
# install step


# Install to /usr/local/bin/
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# and set permissions RW RW RW
