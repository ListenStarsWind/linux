cmake_minimum_required(VERSION 3.20)

project(demo LANGUAGES CXX)

# ================== 强制使用 Clang-18 + libc++ ==================
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    set(CMAKE_CXX_COMPILER clang++-18)
endif()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "必须使用 Clang 编译器！请运行: export CXX=clang++-18")
endif()

# ================== 基础设置 ==================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# ================== 源码与头文件路径 ==================
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SOURCES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp")
file(GLOB HEADERS CONFIGURE_DEPENDS "${INCLUDE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp")

# ================== 可执行文件 ==================
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

# ================== 编译定义 ==================
target_compile_definitions(${PROJECT_NAME} PRIVATE _GNU_SOURCE)

# ================== 编译选项 ==================
target_compile_options(${PROJECT_NAME} PRIVATE
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

# ================== 链接选项 ==================
target_link_options(${PROJECT_NAME} PRIVATE -stdlib=libc++ -lc++abi)
if(NOT APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)
endif()

# ================== 可选功能开关 ==================
option(ENABLE_BOOST "Enable Boost support" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

# -------- Boost 支持 --------
if(ENABLE_BOOST)
    find_package(Boost 1.89 REQUIRED COMPONENTS system filesystem thread)
    if (Boost_FOUND)
        message(STATUS "Boost 版本: ${Boost_VERSION}")
        message(STATUS "Boost 路径: ${Boost_INCLUDE_DIRS}")
        target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system Boost::filesystem Boost::thread)
    endif()
endif()

# -------- pthread 支持（总是打开） --------
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# -------- ThreadSanitizer --------
if (ENABLE_TSAN)
    message(STATUS "ThreadSanitizer 已启用")
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
endif()

# ================== 打印信息 ==================
message(STATUS "============================================ 编译信息 ============================================")
message(STATUS "源码目录:       ${SOURCE_DIR}")
message(STATUS "头文件目录:     ${INCLUDE_DIR}")
message(STATUS "编译器:         ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ 标准库:     libc++")
message(STATUS "C++ 标准:       C++${CMAKE_CXX_STANDARD}")
message(STATUS "GNU 扩展:       已启用 (_GNU_SOURCE + CMAKE_CXX_EXTENSIONS ON)")
message(STATUS "编译类型:       ${CMAKE_BUILD_TYPE}")
message(STATUS "生成 compile_commands.json: ${CMAKE_EXPORT_COMPILE_COMMANDS}")

if(ENABLE_BOOST)
    message(STATUS "Boost 支持:     已启用")
    message(STATUS "Boost 版本:     ${Boost_VERSION}")
    message(STATUS "Boost 路径:     ${Boost_INCLUDE_DIRS}")
else()
    message(STATUS "Boost 支持:     未启用")
endif()

if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer: 已启用")
else()
    message(STATUS "ThreadSanitizer: 未启用")
endif()
message(STATUS "==============================================================================================")
