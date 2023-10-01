cmake_minimum_required(VERSION 3.2)
project(autopilot-service C CXX)
set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} src/main.c)
