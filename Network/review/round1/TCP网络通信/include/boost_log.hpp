#pragma once
#include <fcntl.h>   // open
#include <grp.h>     // getgrnam
#include <limits.h>  // PATH_MAX
#include <signal.h>
#include <sys/stat.h>  // 调整目录权限状态

#include <boost/filesystem.hpp>                           // 目录创建
#include <boost/log/trivial.hpp>                          // 引入Boost全局日志宏
#include <boost/log/utility/setup/common_attributes.hpp>  // 引入一些常见的日志属性, 如时间戳, 执行流ID
#include <boost/log/utility/setup/console.hpp>            // 可将日志输出到终端
#include <boost/log/utility/setup/file.hpp>               // 可将日志输出到文件
#include <format>
#include <string>

inline void init_logging() {
    // 标准输出
    boost::log::add_console_log(
        std::cout, boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%");

    // 工作目录性质判断
    char cwd[PATH_MAX] = {0};
    bool is_root_cwd = (::getcwd(cwd, sizeof(cwd)) && std::string(cwd) == "/");

    // 确定日志目录
    const std::string log_dir = is_root_cwd ? "/var/log/demo" : ".";

    // 确保日志目录存在
    boost::system::error_code ec;
    boost::filesystem::create_directories(log_dir, ec);
    if (is_root_cwd && ec) {
        std::string message = std::format("日志目录不存在且无法创建: {}", ec.message());
        // 在所有终端中广播消息
        for (auto& p : boost::filesystem::directory_iterator("/dev/pts")) {
            if (p.path().filename() == "ptmx") continue;
            int fd = ::open(p.path().c_str(), O_WRONLY | O_NONBLOCK);
            if (fd < 0) continue;
            ::write(fd, message.c_str(), message.size());
            ::close(fd);
        }
        std::quick_exit(ec.value());
    }

    // 明确目录的所属关系: 最终所属人为root, 管理组成员(adm)有权查看
    if (is_root_cwd) {
        ::chmod(log_dir.c_str(), 0755);
        struct group* adm_grp = ::getgrnam("adm");
        gid_t adm_gid = adm_grp ? adm_grp->gr_gid : 4;
        ::chown(log_dir.c_str(), 0, adm_gid);
    }

    // 注册一个日志文件落地目标
    auto sink = boost::log::add_file_log(
        boost::log::keywords::file_name = log_dir + "/app_%N.log",
        boost::log::keywords::rotation_size = 10 * 1024 * 1024,  // 文件滚动大小
        boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%",
        boost::log::keywords::open_mode = (std::ios_base::out | std::ios_base::app)  // 追加写
    );

    // 为新文件的创建注册回调函数, 创建后, 同样调整权限
    if (is_root_cwd) {
        auto backend = sink->locked_backend();

        // 1. 设置文件收集器（用于滚动）
        backend->set_file_collector(
            boost::log::sinks::file::make_collector(boost::log::keywords::target = log_dir));
        backend->scan_for_files();

        // 2. 首次文件 + 每次滚动后，设置权限
        backend->set_open_handler([log_dir](std::ostream&) {
            // 每次打开新文件时，扫描目录下所有 app_*.log
            boost::system::error_code ec;
            for (const auto& entry : boost::filesystem::directory_iterator(log_dir, ec)) {
                if (entry.path().filename().string().rfind("app_", 0) == 0 &&
                    entry.path().extension() == ".log") {
                    ::chmod(entry.path().c_str(), 0644);
                    struct group* g = ::getgrnam("adm");
                    if (g) ::chown(entry.path().c_str(), 0, g->gr_gid);
                }
            }
        });
    }

    // 添加通用属性, 如 时间戳
    boost::log::add_common_attributes();

    // 用来测试服务守护进程化后到底是日志惰性刷新, 还是出问题了
    ::signal(SIGINT, [](int) {
        BOOST_LOG_TRIVIAL(info) << "收到 SIGINT，退出程序";
        boost::log::core::get()->flush();
        ::exit(0);
    });
}
