cmake_minimum_required(VERSION 3.20)
project(demo LANGUAGES CXX)

# ================== 强制使用 Clang-18 + libc++ ==================
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
set(CMAKE_CXX_EXTENSIONS ON)            # 开启 GNU 扩展以支持 _GNU_SOURCE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # LLDB 必备
set(CMAKE_BUILD_TYPE Debug)

# ================== 源码与头文件路径 ==================
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 修复：改用 CONFIGURE_DEPENDS 避免增量构建漏文件
file(GLOB SOURCES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp")
file(GLOB HEADERS CONFIGURE_DEPENDS "${INCLUDE_DIR}/*.h" "${INCLUDE_DIR}/*.hpp")

# ================== 可执行文件 ==================
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ================== 指定头文件搜索路径 ==================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
)

# ================== 启用 标准库 扩展功能及更多系统调用 ==================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _GNU_SOURCE
)

# ================== 编译选项（最大调试 + libc++） ==================
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic # 启用"常用"及"更多"警告, 严格遵循 C/C++ 标准语法
    # -Werror           # 所有警告当做错误
    -g3                 # 最大调试信息
    -O0                 # 关闭所有优化
    -fno-omit-frame-pointer # 保留栈帧基点寄存器 RBP, 用于分析栈帧细节
    -fstandalone-debug  # 生成独立调试信息
    -stdlib=libc++          # 使用 clang C++ 标准库, 避免使用clang 时混用 GUN C++标准库触发异常警告
    # -Wno-deprecated-declarations  # 在使用 "标准库中已经弃用的部件" 时警告(关闭该选项时)
    -grecord-gcc-switches      # 记录编译命令行参数, 便于场景复现
    -fdebug-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.  # 调试属性中使用相对路径, 调试时手动设定相对基点: SRC 所在的目录
    -fno-eliminate-unused-debug-types  # 保留所有调试信息, 比如代码中实际未使用的类
    -fdebug-types-section      # 将所有类的调试信息统一放到同一个特定的程序段中
)

# ================== 链接选项 ==================
target_link_options(${PROJECT_NAME} PRIVATE
    -stdlib=libc++ -lc++abi # 使用 clang C++ 标准库, C++ 底层实现库
)

# macOS 不支持 -rdynamic，改为条件添加
if(NOT APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE -rdynamic) # 公开自己实现的函数名, 便于调试时将函数指针转化为函数名
endif()

# ================== Boost 设置 ==================
find_package(Boost 1.89 REQUIRED COMPONENTS system filesystem thread)

if (Boost_FOUND)
    message(STATUS "Boost 版本: ${Boost_VERSION}")
    message(STATUS "Boost 路径: ${Boost_INCLUDE_DIRS}")
endif()

# 使用 Boost:: 目标（自动包含头文件 + 库 + 传递依赖）
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::system
    Boost::filesystem
    Boost::thread
)

# 始终链接 pthread（Boost::thread 已包含，但保险起见）
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# ================== Thread Sanitizer (可选) ==================
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
if (ENABLE_TSAN)
    message(STATUS "ThreadSanitizer 已启用")
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
endif()

# ================== 打印信息 ==================
message(STATUS "源码目录: ${SOURCE_DIR}")
message(STATUS "头文件目录: ${INCLUDE_DIR}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "标准库: libc++")
message(STATUS "Boost 版本: ${Boost_VERSION}")
message(STATUS "生成 compile_commands.json: ${CMAKE_EXPORT_COMPILE_COMMANDS}")
message(STATUS "GNU 扩展: 已启用 (_GNU_SOURCE + CMAKE_CXX_EXTENSIONS ON)")
