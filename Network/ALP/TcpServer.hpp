#pragma once

#include "Sockst.hpp"
#include "log.hpp"
#include <signal.h>
#include <functional>

class tcpserver
{
    typedef std::function<std::string(std::string &)> func;

public:
    template <class func_>
    tcpserver(func_ callback, uint16_t port = 8888) : _callback(callback), _port(port) {}
    void init()
    {
        _listensock.create_();
        _listensock.reuse_port_address();
        _listensock.bind_(_port);
        _listensock.listen_();
    }

    void run()
    {
        print_netstat();
        signal(SIGCHLD, SIG_IGN);
        while (true)
        {
            std::string clientip;
            uint16_t port;
            int sockfd = _listensock.accept_(&clientip, &port);
            if (sockfd == -1)
                continue;

            if (fork() == 0)
            {
                _log(Info, "获取到一个新的连接");
                _listensock.close_();
                std::string inbuffer_stream;
                try
                {
                    while (true)
                    {
                        char buffer[128];
                        ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
                        if (n > 0)
                        {
                            buffer[n] = 0;
                            inbuffer_stream += buffer;
                            _log(Debug, "接收到的数据为: %s", inbuffer_stream.c_str());

                            std::string out_stream = _callback(inbuffer_stream);


                            if (out_stream.empty())
                                continue;

                            _log(Debug, "发回的数据为: %s", out_stream.c_str());

                            write(sockfd, out_stream.c_str(), out_stream.size());
                            // break;
                        }
                        else if (n == 0)
                        {

                        }
                        else
                        {
                            _log(Warning, "read error: %s", strerror(errno));
                            break;
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    _log(Warning, e.what());
                }
                _log(Info, "连接关闭");
                close(sockfd);
                exit(0);
            }

            close(sockfd);
        }
    }

private:
    socket_ _listensock;
    Log &_log = Log::getInstance();
    func _callback;
    uint16_t _port;
};