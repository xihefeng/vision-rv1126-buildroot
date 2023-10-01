cmake_minimum_required(VERSION 3.2)
project(vision-services C CXX)

add_subdirectory(services/camera)
add_subdirectory(services/msp)
add_subdirectory(services/autopilot)
add_subdirectory(services/drm)
add_subdirectory(services/tracker)
add_subdirectory(services/detection)
