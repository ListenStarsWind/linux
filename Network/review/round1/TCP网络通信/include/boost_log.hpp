#pragma once

#include <boost/log/trivial.hpp>                          // 引入Boost全局日志宏
#include <boost/log/utility/setup/common_attributes.hpp>  // 引入一些常见的日志属性, 如时间戳, 执行流ID
#include <boost/log/utility/setup/console.hpp>            // 可将日志输出到终端
#include <boost/log/utility/setup/file.hpp>               // 可将日志输出到文件

inline void init_logging() {
    // 1. 文件日志，按大小滚动
    boost::log::add_file_log(
        boost::log::keywords::file_name = "logs/app_%N.log",                   // 日志文件名格式
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,                // 10 MB大小滚动
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"  // 日志格式
    );

    // 2. 控制台输出
    boost::log::add_console_log(
        std::cout, boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%");

    // 3. 添加时间戳等通用属性
    boost::log::add_common_attributes();
}
