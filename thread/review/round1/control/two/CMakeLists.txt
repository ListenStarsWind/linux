cmake_minimum_required(VERSION 3.16)
project(mythread_lldb_debug LANGUAGES CXX)

# ================== 强制使用 Clang-18 + libc++ ==================
# 优先使用环境变量 CXX，否则默认 clang++-18
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-18)
endif()

# 强制检查是否为 Clang
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "必须使用 Clang 编译器！请运行: export CXX=clang++-18")
endif()

# ================== 基础设置 ==================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)           # 关闭 GNU 扩展
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # LLDB 必备
set(CMAKE_BUILD_TYPE Debug)

# ================== 源码与头文件路径 ==================
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SOURCES "${SOURCE_DIR}/*.cpp")
# file(GLOB HEADERS "${INCLUDE_DIR}/*.hpp")

# ================== 可执行文件 ==================
add_executable(mythread_lldb_test ${SOURCES} ${HEADERS})

# ================== 包含目录 ==================
target_include_directories(mythread_lldb_test PRIVATE
    ${INCLUDE_DIR}
)

# ================== 编译定义（启用 clone3） ==================
target_compile_definitions(mythread_lldb_test PRIVATE
    _GNU_SOURCE
)

# ================== 编译选项（最大调试 + libc++） ==================
target_compile_options(mythread_lldb_test PRIVATE
    -Wall -Wextra -Wpedantic
    # -Werror           # 调试阶段不要开！
    -g3
    -O0
    -fno-omit-frame-pointer
    -fstandalone-debug
    -stdlib=libc++          # 关键：使用 Clang 标准库
    -Wno-deprecated-declarations  # 屏蔽 result_of 警告
    -grecord-gcc-switches      # 记录编译开关
    -fdebug-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.  # 路径映射
    -fno-eliminate-unused-debug-types  # 关键！保留头文件类型
    -fdebug-types-section      # 关键！DWARF 优化
)

# ================== 链接选项 ==================
target_link_options(mythread_lldb_test PRIVATE
    -stdlib=libc++ -lc++abi
    -rdynamic
)

# ================== Thread Sanitizer (可选) ==================
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
if (ENABLE_TSAN)
    message(STATUS "ThreadSanitizer 已启用")
    target_compile_options(mythread_lldb_test PRIVATE -fsanitize=thread)
    target_link_options(mythread_lldb_test PRIVATE -fsanitize=thread)
    find_package(Threads REQUIRED)
    target_link_libraries(mythread_lldb_test PRIVATE Threads::Threads)
endif()

# ================== 打印信息 ==================
message(STATUS "源码目录: ${SOURCE_DIR}")
message(STATUS "头文件目录: ${INCLUDE_DIR}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "标准库: libc++")
message(STATUS "生成 compile_commands.json: ${CMAKE_EXPORT_COMPILE_COMMANDS}")