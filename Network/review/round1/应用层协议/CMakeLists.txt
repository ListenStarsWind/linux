cmake_minimum_required(VERSION 3.22)
project(demo LANGUAGES CXX)

# 如果环境变量中显式设置了编辑器, 则 cmake 就使用用户指定的编译器
if (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER $ENV{CXX})
else()
    # 否则 cmake 默认使用 clang++-21 作为编译器
    set(CMAKE_CXX_COMPILER clang++-21)
endif()

# 本项目的代码基于 llvm 环境, 因此必须使用对应的编译器
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "需要用 Clang 21 + libc++ 编译: export CXX=clang++-21")
endif()

# C++ 标准相关设置
set(CMAKE_CXX_STANDARD 20)            # 使用 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 强制要求 C++20
set(CMAKE_CXX_EXTENSIONS OFF)         # 是否允许 GNU 扩展, OFF 表示严格标准
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成 compile_commands.json，供 clangd 等工具使用
set(CMAKE_BUILD_TYPE Debug)           # 默认 Debug，可在命令行覆盖
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # 生成位置无关代码，便于链接静态库或共享库

# 为 find_package 提供额外路径
list(PREPEND CMAKE_PREFIX_PATH "/opt/libcxx-pkgs") # 比如 Protobuf/Abseil 自定义安装目录

# 找 Abseil/Protobuf/线程库
find_package(absl    CONFIG REQUIRED)   # absl 库
find_package(Protobuf CONFIG REQUIRED)  # protobuf 编译器及库
find_package(Threads REQUIRED)          # 线程库（跨平台）

# ====================== Boost 常用组件开关（推荐） ======================
# 每个组件用 option 提供开关，用户可在 cmake GUI 或命令行修改
option(USE_BOOST_SYSTEM          "启用 Boost.System"          ON)
option(USE_BOOST_FILESYSTEM      "启用 Boost.Filesystem"      ON)
option(USE_BOOST_THREAD          "启用 Boost.Thread"          ON)
option(USE_BOOST_LOG             "启用 Boost.Log"             ON)
option(USE_BOOST_LOG_SETUP       "启用 Boost.Log.Setup"       ON)
option(USE_BOOST_PROGRAM_OPTIONS "启用 Boost.ProgramOptions"  ON)
option(USE_BOOST_ASIO            "启用 Boost.Asio"            OFF)
option(USE_BOOST_BEAST           "启用 Boost.Beast"           OFF)
option(USE_BOOST_JSON            "启用 Boost.Json"            OFF)
option(USE_BOOST_REGEX           "启用 Boost.Regex"           OFF)
option(USE_BOOST_IOSTREAMS       "启用 Boost.Iostreams"       OFF)
option(USE_BOOST_SERIALIZATION   "启用 Boost.Serialization"   OFF)
option(USE_BOOST_ATOMIC          "启用 Boost.Atomic"          OFF)

# 根据开关动态收集需要查找和链接的组件
set(BOOST_COMPONENTS_TO_FIND "")   # 用于 find_package(Boost COMPONENTS ...)
set(BOOST_LIBRARIES_TO_LINK "")    # 用于 target_link_libraries(...)

# 宏：检查开关，如果启用则加入列表
macro(add_boost_component name opt)
    if(${opt})
        list(APPEND BOOST_COMPONENTS_TO_FIND ${name})  # 传给 find_package 查找
        list(APPEND BOOST_LIBRARIES_TO_LINK Boost::${name}) # 传给 target_link_libraries
    endif()
endmacro()

# 遍历所有开关，将启用的组件加入列表
add_boost_component(system          USE_BOOST_SYSTEM)
add_boost_component(filesystem      USE_BOOST_FILESYSTEM)
add_boost_component(thread          USE_BOOST_THREAD)
add_boost_component(log             USE_BOOST_LOG)
add_boost_component(log_setup       USE_BOOST_LOG_SETUP)
add_boost_component(program_options USE_BOOST_PROGRAM_OPTIONS)
add_boost_component(asio            USE_BOOST_ASIO)
add_boost_component(beast           USE_BOOST_BEAST)
add_boost_component(json            USE_BOOST_JSON)
add_boost_component(regex           USE_BOOST_REGEX)
add_boost_component(iostreams       USE_BOOST_IOSTREAMS)
add_boost_component(serialization   USE_BOOST_SERIALIZATION)
add_boost_component(atomic          USE_BOOST_ATOMIC)

# 只有至少启用了一个组件时才查找 Boost
if(BOOST_COMPONENTS_TO_FIND)
    find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS_TO_FIND})
endif()

# ====================== 路径与源文件 ======================
set(PROTO_DIR   "${CMAKE_SOURCE_DIR}/proto")       # protobuf 源文件目录
set(GEN_ROOT    "${CMAKE_BINARY_DIR}/gen")         # protobuf 生成文件目录
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include) # 项目头文件目录
set(SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src)     # 项目源文件目录

# 递归收集 server 源文件（包含 .cpp 和 .cc）
file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS
     "${SOURCE_DIR}/server/*.cpp"
     "${SOURCE_DIR}/server/*.cc"
     "${SOURCE_DIR}/*.cpp"
     "${SOURCE_DIR}/*.cc")

# 递归收集 client 源文件（包含 .cpp 和 .cc）
file(GLOB CLIENT_SOURCES CONFIGURE_DEPENDS
     "${SOURCE_DIR}/client/*.cpp"
     "${SOURCE_DIR}/client/*.cc"
     "${SOURCE_DIR}/*.cpp"
     "${SOURCE_DIR}/*.cc")

# ====================== protobuf 生成函数 ======================
function(add_protobuf_generation)
    # 解析函数调用时输入的参数，初始化 ARG_GEN_ROOT 和 ARG_PROTO_DIR
    cmake_parse_arguments(ARG "" "GEN_ROOT;PROTO_DIR" "" ${ARGN})
    if(NOT ARG_GEN_ROOT OR NOT ARG_PROTO_DIR)
        message(FATAL_ERROR "add_protobuf_generation() 需要提供 GEN_ROOT 和 PROTO_DIR")
    endif()

    # 根生成目录下新建 proto 子目录
    set(proto_gen_dir "${ARG_GEN_ROOT}/proto")
    file(MAKE_DIRECTORY "${proto_gen_dir}")

    # 递归收集所有 proto 源文件（相对 ARG_PROTO_DIR）
    file(GLOB_RECURSE proto_files RELATIVE "${ARG_PROTO_DIR}" "${ARG_PROTO_DIR}/*.proto")
    if(NOT proto_files)
        message(FATAL_ERROR "在 ${ARG_PROTO_DIR} 里没有找到 .proto 文件")
    endif()

    set(generated_srcs "")
    set(generated_hdrs "")

    # 循环生成每个 proto 的 cc/h 文件
    foreach(proto_rel IN LISTS proto_files)
        get_filename_component(proto_dir  "${proto_rel}" DIRECTORY) # proto 所在路径
        get_filename_component(proto_name "${proto_rel}" NAME_WE)  # proto 文件名，不带扩展

        # 仿照 proto 源目录结构生成输出目录
        set(out_dir "${proto_gen_dir}/${proto_dir}")
        set(out_cc "${out_dir}/${proto_name}.pb.cc")
        set(out_h   "${out_dir}/${proto_name}.pb.h")
        file(MAKE_DIRECTORY "${out_dir}") # 确保目录存在

        add_custom_command(
            OUTPUT "${out_cc}" "${out_h}"
            COMMAND protobuf::protoc
                    --cpp_out=${proto_gen_dir}
                    --proto_path=${ARG_PROTO_DIR}
                    "${ARG_PROTO_DIR}/${proto_rel}"
            DEPENDS "${ARG_PROTO_DIR}/${proto_rel}" protobuf::protoc
            COMMENT "protoc → ${proto_rel}" # 命令输出提示
            VERBATIM                       # 不要解析命令中的特殊字符
            USES_TERMINAL                  # 直接输出到终端，方便调试
        )
        list(APPEND generated_srcs "${out_cc}") # 收集生成的 cpp
        list(APPEND generated_hdrs "${out_h}")   # 收集生成的 h
    endforeach()

    # 将生成的文件列表写入父级作用域
    set(PROTOBUF_GENERATED_SRCS ${generated_srcs} PARENT_SCOPE)
    set(PROTOBUF_GENERATED_HDRS ${generated_hdrs} PARENT_SCOPE)
    set(PROTOBUF_GEN_ROOT ${ARG_GEN_ROOT} PARENT_SCOPE)
endfunction()

# ====================== 可执行目标生成函数 ======================
function(add_cpp_executable target_name sources)
    add_executable(${target_name}
        ${sources}
        ${PROTOBUF_GENERATED_SRCS} # 自动包含 protobuf 生成的 cpp
    )

    target_include_directories(${target_name} PRIVATE 
        ${GEN_ROOT}                                     # protobuf 生成头文件目录
        "${CMAKE_SOURCE_DIR}/include"                   # 公共头文件
        "${CMAKE_SOURCE_DIR}/include/${target_name}"    # 每个模块自己的头文件
    )

    # 根据 CMAKE_CXX_EXTENSIONS 打开 GNU 扩展相关宏
    if(CMAKE_CXX_EXTENSIONS)
        target_compile_definitions(${target_name} PRIVATE _GNU_SOURCE)
    endif()

    target_compile_options(${target_name} PRIVATE
        -Wall -Wextra -Wpedantic -Werror       # 严格警告
        -g3 -O0                                # Debug 信息
        -fno-omit-frame-pointer
        -fstandalone-debug
        -stdlib=libc++                          # 强制使用 libc++
        -grecord-gcc-switches
        -fdebug-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.
        -fno-eliminate-unused-debug-types
        -fdebug-types-section
    )

    # 链接库
    target_link_libraries(${target_name} PRIVATE
        protobuf::libprotobuf
        absl::strings
        absl::log
        ${BOOST_LIBRARIES_TO_LINK}         
        Threads::Threads
        c++
        c++abi
    )

    # 谷歌为了兼容苹果, 其 absl 使用了苹果内部的一个非标准语法
    # protoc 生成的代码依赖于 absl, 所以也用了这个语法
    # llvm 是苹果的, 它认识这个语法, 但也知道这不是标准的
    # 发出警告, 严格警告, 编译崩溃
    # 对 *.pb.* 硬性取消这种语法检查
    # GUN 因为用的是标准语法, 所以根本不进行语法检查, 反而没事
    set_source_files_properties(
        ${PROTOBUF_GENERATED_SRCS}
        PROPERTIES
        COMPILE_FLAGS "-Wno-nullability-extension"
    )

    message(STATUS "已添加可执行目标 → ${target_name}")
endfunction()

# ====================== 自定义清理指令 ======================
if(CMAKE_GENERATOR STREQUAL "Ninja")
    # 创建一个自定义目标，依赖于默认 clean
    add_custom_target(clean_all
        # 执行 Ninja 的默认 clean
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
        # 删除 protobuf 生成目录
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${GEN_ROOT}"
    )
endif()


# ====================== 生成 protobuf 代码 ======================
add_protobuf_generation(
    GEN_ROOT ${GEN_ROOT}
    PROTO_DIR ${PROTO_DIR}
)

# ====================== 添加可执行目标 ======================
add_cpp_executable(server ${SERVER_SOURCES})
add_cpp_executable(client ${CLIENT_SOURCES})

message(STATUS "============================================ 编译信息 ============================================")
message(STATUS "源码根目录 : ${SOURCE_DIR}")
message(STATUS "头文件根目录 : ${INCLUDE_DIR}")
message(STATUS "proto目录 : ${PROTO_DIR}")
message(STATUS "生成目录 : ${GEN_ROOT}")
message(STATUS "编译器 : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ 标准库 : libc++")
message(STATUS "C++ 标准 : C++${CMAKE_CXX_STANDARD}")

# 统一输出 C++ 扩展状态
if(CMAKE_CXX_EXTENSIONS)
    message(STATUS "GNU 扩展 : ON (C++ 扩展已启用, _GNU_SOURCE 已定义)")
else()
    message(STATUS "GNU 扩展 : OFF (C++ 扩展未启用)")
endif()

message(STATUS "构建类型 : ${CMAKE_BUILD_TYPE}")
message(STATUS "compile_commands: ${CMAKE_EXPORT_COMPILE_COMMANDS}")

# 动态输出 Boost 组件开关状态
message(STATUS "Boost 组件开关 :")
set(BOOST_OPTIONS_LIST
    USE_BOOST_SYSTEM
    USE_BOOST_FILESYSTEM
    USE_BOOST_THREAD
    USE_BOOST_LOG
    USE_BOOST_LOG_SETUP
    USE_BOOST_PROGRAM_OPTIONS
    USE_BOOST_ASIO
    USE_BOOST_BEAST
    USE_BOOST_JSON
    USE_BOOST_REGEX
    USE_BOOST_IOSTREAMS
    USE_BOOST_SERIALIZATION
    USE_BOOST_ATOMIC
)

foreach(opt IN LISTS BOOST_OPTIONS_LIST)
    # 使用 CMake 的变量引用机制
    if(DEFINED ${opt})
        message(STATUS "  ${opt} = ${${opt}}")
    endif()
endforeach()
