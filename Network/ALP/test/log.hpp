#pragma once

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <string>
#include <cstdarg>
#include <vector>
#include <cassert>
#include<cstring>

// 面向系统
#ifdef __linux__

#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#define dMODE 0755 // 新建路径权限
#define fMODE 0666 // 新建文件权限

#endif

namespace wind
{
    // 日志等级
    // 枚举常量的值含有映射关系
    // 必须从0开始，并且连续递增
    enum level
    {
        Info = 0,
        Debug = 1,
        Warning = 2,
        Error = 3,
        Fatal = 4,
        count // 获知enum level中的有效成员个数
    };

    const char *levelTostr(level le)
    {
        switch (le)
        {
        case Info:
            return "Info";
        case Debug:
            return "Debug";
        case Warning:
            return "Warning";
        case Error:
            return "Error";
        case Fatal:
            return "Fatal";
        default:
            return "None";
        }
    }

    const char *levelTostr(int le)
    {
        switch (le)
        {
        case Info:
            return "Info";
        case Debug:
            return "Debug";
        case Warning:
            return "Warning";
        case Error:
            return "Error";
        case Fatal:
            return "Fatal";
        default:
            return "None";
        }
    }

    enum out_mode
    {
        display = 1,  // 所有信息都输出到显示屏幕上
        one_fife = 2, // 所有信息都输出到某个文件中
        category = 3  // 按照日志等级进行多文件分类输出
    };


    class Log
    {
    private:
        // 实例化对象前不要关闭stderr，这会引发未知错误
        // flag用于指定日志的输出模式
        Log(out_mode flag = display, std::string name = "log")
            : _name(name)
        {
#ifdef __linux__

            if (flag == one_fife || flag == category)
            {
                if (mkdir(_name.c_str(), dMODE) != 0);
                    // perror("Error");

                std::string s = "./";
                s += _name;
                s += "/log.txt";

                if (flag == one_fife)
                {
                    int fd = open(s.c_str(), O_WRONLY | O_CREAT | O_APPEND, fMODE);
                    if (fd < 0)
                        perror("Error");
                    _fd.push_back(fd);
                }
                else if (flag == category)
                {
                    s += ".";
                    int n = count;
                    for (int i = 0; i < n; i++)
                    {
                        std::string str = s + levelTostr(i);
                        int fd = open(str.c_str(), O_WRONLY | O_CREAT | O_APPEND, fMODE);
                        if (fd < 0)
                            perror("Error");
                        _fd.push_back(fd);
                    }
                }
                else
                {
                    // 不存在的分支
                    fprintf(stderr, "%s\n", "Non-existent branch");
                    assert(0);
                }
            }

            if (flag == display)
            {
                int fd = dup(2);
                if (fd < 0)
                    perror("Error");
                _fd.push_back(fd);
            }

#endif
        }

        public:
        static Log& getInstance()
        {
            static Log inst;
            return inst;
        }

        void operator()(level le, const char *format, ...)
        {
            // 日志固定部分的构建
            char fix[64] = {0};
            time_t t = time(nullptr);
            struct tm *ct = localtime(&t);
            snprintf(fix, sizeof(fix) - 1, "[%s][%d-%d-%d %d:%d:%d]", levelTostr(le), ct->tm_year + 1900, ct->tm_mon + 1, ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec);

            // 构建用户输入的变化部分
            char dynamic[1024] = {0};
            va_list s;
            va_start(s, format);
            vsnprintf(dynamic, sizeof(dynamic) - 1, format, s);
            va_end(s);

            // 拼接上述两个部分
            snprintf(_buffer, sizeof(_buffer) - 1, "%s::%s\n", fix, dynamic);

            out(le);
        }

        ~Log()
        {
            for(auto e : _fd)
            {
                close(e);
            }
        }

    private:
        void out()
        {
            printf("%s", _buffer);
            _buffer[0] = 0;
        }

        void out(level le)
        {
            if(_fd.size() == 1)
            {
                write(_fd[0], _buffer, strlen(_buffer));
            }
            else if(_fd.size() > 1)
            {
                write(_fd[le], _buffer, strlen(_buffer));
            }
            else
            {
                fprintf(stderr, "%s\n", "Non-existent branch");
                    assert(0);
            }
        }

        std::string _name;       // 日志文件夹名称
        char _buffer[1088];
        std::vector<int> _fd;
    };
}