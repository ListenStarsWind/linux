cmake_minimum_required(VERSION 3.16)
project(SystemLab LANGUAGES C CXX)

# -------------------------------
# 构建选项
# -------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)

# 使用 Clang/Clang++
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# 严格标准模式
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -------------------------------
# 警告级别（Linux 系统编程适配）
# -------------------------------
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wno-unused-parameter
    -Wno-unused-variable
    -pthread
)

# -------------------------------
# 链接线程库
# -------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# -------------------------------
# 源文件与可执行文件
# -------------------------------
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# -------------------------------
# 输出路径
# -------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    OUTPUT_NAME "pthread"
)
