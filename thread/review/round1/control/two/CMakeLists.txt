cmake_minimum_required(VERSION 3.16)
project(thread LANGUAGES C CXX)

# ---------------------
# 基础设置
# ---------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# ---------------------
# 线程库（原生 pthread）
# ---------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ---------------------
# 源文件
# ---------------------
file(GLOB SOURCES
    "src/*.c"
    "src/*.cpp"
    "src/*.cc"
)

# ---------------------
# 可执行文件
# ---------------------
add_executable(thread ${SOURCES})

# ---------------------
# 编译配置
# ---------------------
target_include_directories(thread PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(thread PRIVATE
    _GNU_SOURCE   # ← 必须在此处！
)

target_compile_options(thread PRIVATE
    -Wall -Wextra -Wpedantic
    -g -O0
)

# ---------------------
# 链接库（最后）
# ---------------------
target_link_libraries(thread PRIVATE
    Threads::Threads
)
