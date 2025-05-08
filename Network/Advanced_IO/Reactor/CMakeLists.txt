cmake_minimum_required(VERSION 3.10)
project(Reactor)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

# 包含头文件
include_directories(include)
include_directories(/usr/include/jsoncpp)

# 添加可执行文件
add_executable(reactor_server src/main.cc)
add_executable(reactor_client src/ClientCal.cc)

# 链接 jsoncpp 库
target_link_libraries(reactor_server PRIVATE jsoncpp)
target_link_libraries(reactor_client PRIVATE jsoncpp)

# 生成 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

