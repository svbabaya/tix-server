cmake_minimum_required(VERSION 3.10)
project(camera_server)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libs in system
#find_package(OpenCV REQUIRED)

add_subdirectory(src)
