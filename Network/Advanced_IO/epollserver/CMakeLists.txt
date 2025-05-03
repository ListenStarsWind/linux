cmake_minimum_required(VERSION 3.10)
project(epollserver)

set(CMAKE_CXX_STANDARD 17)

# 包含头文件路径
include_directories(include)

# 添加可执行文件（由 main.cc 构建而成）
add_executable(epollserver src/main.cc)
