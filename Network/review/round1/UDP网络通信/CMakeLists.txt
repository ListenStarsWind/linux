# ==============================================================================
# CMake 最低版本
# ==============================================================================
cmake_minimum_required(VERSION 3.20)

# 项目名
project(demo LANGUAGES CXX)

# ==============================================================================
# 1. 强制使用 Clang-21 + libc++
# ==============================================================================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-21)
endif()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "必须使用 Clang 编译器！请运行: export CXX=clang++-21")
endif()

# ==============================================================================
# 2. 基础编译设置
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)          # 允许 GNU 扩展
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成 compile_commands.json，供 clangd 等工具使用
set(CMAKE_BUILD_TYPE Debug)           # 默认 Debug，后面可以覆盖

# ==============================================================================
# 3. 源码/头文件根目录
# ==============================================================================
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ==============================================================================
# 4. 目录结构（请按此组织文件）
#   demo/
#   ├─ CMakeLists.txt
#   ├─ include/
#   │   ├─ server/   *.h / *.hpp
#   │   └─ client/   *.h / *.hpp
#   └─ src/
#       ├─ server/   *.cpp
#       └─ client/   *.cpp
# ==============================================================================

# --------------------- 服务端源码 ---------------------
file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS
     "${SOURCE_DIR}/server/*.cpp"
     "${SOURCE_DIR}/server/*.cc"
     "${SOURCE_DIR}/*.cpp"
     "${SOURCE_DIR}/*.cc")

file(GLOB SERVER_HEADERS CONFIGURE_DEPENDS
     "${INCLUDE_DIR}/server/*.h"
     "${INCLUDE_DIR}/server/*.hpp"
      "${INCLUDE_DIR}/*.h"
     "${INCLUDE_DIR}/*.hpp")

# --------------------- 客户端源码 ---------------------
file(GLOB CLIENT_SOURCES CONFIGURE_DEPENDS
     "${SOURCE_DIR}/client/*.cpp"
     "${SOURCE_DIR}/client/*.cc"
     "${SOURCE_DIR}/*.cpp"
     "${SOURCE_DIR}/*.cc")

file(GLOB CLIENT_HEADERS CONFIGURE_DEPENDS
     "${INCLUDE_DIR}/client/*.h"
     "${INCLUDE_DIR}/client/*.hpp"
      "${INCLUDE_DIR}/*.h"
     "${INCLUDE_DIR}/*.hpp")

# ==============================================================================
# 5. 可选功能开关
# ==============================================================================
option(ENABLE_BOOST "Enable Boost support (system, filesystem, thread, log)" ON)
option(ENABLE_TSAN  "Enable ThreadSanitizer" OFF)

# --------------------- Boost ---------------------
if (ENABLE_BOOST)
    find_package(Boost 1.89 REQUIRED COMPONENTS
        system filesystem thread log log_setup)
endif()

# --------------------- pthread -------------------
find_package(Threads REQUIRED)

# ==============================================================================
# 6. 公共配置函数（负责「编译选项 / 链接库 / 宏」）
# ==============================================================================
function(configure_common TARGET)
    # 头文件搜索路径
    target_include_directories(${TARGET} PRIVATE ${INCLUDE_DIR})

    # 编译宏
    target_compile_definitions(${TARGET} PRIVATE _GNU_SOURCE)

    # 编译选项（调试信息 + 警告 + libc++）
    target_compile_options(${TARGET} PRIVATE
        -Wall -Wextra -Wpedantic
        -g3 -O0
        -fno-omit-frame-pointer
        -fstandalone-debug
        -stdlib=libc++
        -grecord-gcc-switches
        -fdebug-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.
        -fno-eliminate-unused-debug-types
        -fdebug-types-section
    )

    # 链接选项（libc++ + c++abi）
    target_link_options(${TARGET} PRIVATE -stdlib=libc++ -lc++abi)
    if (NOT APPLE)
        target_link_options(${TARGET} PRIVATE -rdynamic)
    endif()

    # Boost（如果开启）
    if (ENABLE_BOOST)
        target_link_libraries(${TARGET} PRIVATE
            Boost::system Boost::filesystem Boost::thread
            Boost::log Boost::log_setup)
        target_compile_definitions(${TARGET} PRIVATE BOOST_LOG_DYN_LINK)
    endif()

    # pthread（始终需要）
    target_link_libraries(${TARGET} PRIVATE Threads::Threads)

    # ThreadSanitizer（可选）
    if (ENABLE_TSAN)
        message(STATUS "ThreadSanitizer 已启用（目标: ${TARGET})")
        target_compile_options(${TARGET} PRIVATE -fsanitize=thread)
        target_link_options(${TARGET} PRIVATE -fsanitize=thread)
    endif()
endfunction()

# ==============================================================================
# 7. 创建可执行目标
# ==============================================================================
# 服务端
add_executable(${PROJECT_NAME}_server ${SERVER_SOURCES} ${SERVER_HEADERS})
configure_common(${PROJECT_NAME}_server)

# 客户端（名字加后缀防止冲突）
add_executable(${PROJECT_NAME}_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})
configure_common(${PROJECT_NAME}_client)

# ==============================================================================
# 8. 编译信息输出
# ==============================================================================
message(STATUS "============================================ 编译信息 ============================================")
message(STATUS "源码根目录      : ${SOURCE_DIR}")
message(STATUS "头文件根目录    : ${INCLUDE_DIR}")
message(STATUS "编译器          : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ 标准库      : libc++")
message(STATUS "C++ 标准        : C++${CMAKE_CXX_STANDARD}")
message(STATUS "GNU 扩展        : 已启用 (_GNU_SOURCE + CMAKE_CXX_EXTENSIONS ON)")
message(STATUS "构建类型        : ${CMAKE_BUILD_TYPE}")
message(STATUS "compile_commands: ${CMAKE_EXPORT_COMPILE_COMMANDS}")

if (ENABLE_BOOST)
    message(STATUS "Boost 支持      : 已启用（版本 ${Boost_VERSION}）")
    message(STATUS "Boost 路径      : ${Boost_INCLUDE_DIRS}")
else()
    message(STATUS "Boost 支持      : 未启用")
endif()

if (ENABLE_TSAN)
    message(STATUS "ThreadSanitizer : 已启用")
else()
    message(STATUS "ThreadSanitizer : 未启用")
endif()

message(STATUS "生成的可执行文件:")
message(STATUS "  - 服务端: ${PROJECT_NAME}_server")
message(STATUS "  - 客户端: ${PROJECT_NAME}_client")
message(STATUS "============================================================================================")