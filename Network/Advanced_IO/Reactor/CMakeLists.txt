cmake_minimum_required(VERSION 3.10)
project(Reactor)

set(CMAKE_CXX_STANDARD 11)

# 设置源文件路径
add_executable(Reactor src/main.cc)

# 如果有头文件路径，可以加上
include_directories(include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
