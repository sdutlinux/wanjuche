# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
# 项目信息
project (wanjuche)
# 指定生成目标
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
set ( CMAKE_VERBOSE_MAKEFILE on )
aux_source_directory(./src DIR_SRCS)
add_subdirectory(./src/thirdparty)
add_executable(wanjuche src/wanjuche.cpp)
target_link_libraries(wanjuche event thirdparty)
