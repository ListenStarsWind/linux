cmake_minimum_required(VERSION 3.10)
project(Reactor)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

# 设置源文件路径
add_executable(Reactor src/main.cc)

# 如果有头文件路径，可以加上
include_directories(include)

# 添加 jsoncpp 库的头文件目录
include_directories(/usr/include/jsoncpp)

# 添加 jsoncpp 库的链接目录
link_directories(/usr/lib/x86_64-linux-gnu/)

# 链接 jsoncpp 库
target_link_libraries(Reactor PRIVATE jsoncpp)

# 生成编译命令文件
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
