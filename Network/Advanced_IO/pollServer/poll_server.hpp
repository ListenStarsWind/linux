#pragma once

#include"log.hpp"
#include"Sockst.hpp"
#include <poll.h>
#include <iostream>
#include <sys/select.h>
#include <sys/time.h>
#include <string>
#include <algorithm>


using namespace std;

static const short POLLNEVT = 0;            // events的缺省参数, 表示不关心任何事件
static const int defaultfd = -1;
static const int max_managed_fds = 64;      // 可以随便改, 只要系统受得住
static const uint16_t defaultport = 8080;

class PollServer
{
    public:
        PollServer(uint16_t port = defaultport):_port(port) {
            struct pollfd d;
            d.fd = defaultfd;           // 系统会跳过无效的描述符
            d.events = POLLNEVT;
            d.revents = POLLNEVT;
            fill(_fds, _fds + max_managed_fds, d);
        }

        ~PollServer(){ _listensock.close_();}

        void init()
        {
            _listensock.create_();
            _listensock.reuse_port_address();
            _listensock.bind_(_port);
            _listensock.listen_();
            _fds[0].fd = _listensock;
            _fds[0].events = POLLIN;
        }

        void start()
        {
            const int timeout = 2000;
            for(;;)
            {
                int n = poll(_fds, max_managed_fds, timeout);
                if(n == 0)
                    cout << "无有效事件发生"<<endl;
                else if(n > 0)
                    event_response();
                else
                    cerr << "poll error!"<<endl;
            }
        }

        void event_response()
        {
            cout <<endl<< "开始事件响应"<<endl;
            if(_fds[0].revents == POLLIN)
                get_new_link();
            for(int i = 1; i < max_managed_fds; ++i)
            {
                if(_fds[i].fd != -1 && _fds[i].revents & POLLIN) // 按位或也可以
                    session(i);
            }
        }

        // 我们先不考虑面向字节流而导致的粘包问题
        // 后面还有更好的多路转接方案, 到时候我们再严谨的走一遍整个流程
        void session(int idx)
        {
            _log(Info, "收到一个新的消息, 来自%d", _fds[idx]);
            char buff[128] = {0};
            int n = read(_fds[idx].fd, buff, sizeof(buff));
            if(n > 0)
            {
                buff[n] = 0;
                cout << "用户说: "<<buff<<endl;
            }
            else if(n == 0)
            {
                _log(Warning, "用户退出了连接: %d", _fds[idx].fd);
                close(_fds[idx].fd);
                _fds[idx].fd = defaultfd;
            }
            else
            {
                _log(Warning, "错误的读数据");
                close(_fds[idx].fd);
                _fds[idx].fd = defaultfd;
            }

            cout << endl;
        }

        void get_new_link()
        {
            string clientip;
            uint16_t clientport;
            int sock = _listensock.accept_(&clientip, &clientport);
            if(sock < 0) return;
            _log(Info, "获得一个新的客户端连接: ip:%s, 端口:%d, 占有的文件描述符为:%d", clientip.c_str(), clientport, sock);

            add_link(sock);
        }

        void add_link(int sock)
        {
            int idx = 1;
            while(idx < max_managed_fds && _fds[idx].fd != defaultfd) ++idx;
            if(idx == max_managed_fds)
            {
                // 由于poll已经不受到fd_set的限制, 所以现在也可以扩容
                // 或者把_fds换成vector
                _log(Warning, "托管数目超出限制, 连接关闭:%d", sock);
                close(sock);
            }
            else
            {
                _fds[idx].fd = sock;
                _fds[idx].events = POLLIN;
            }
        }

    private:
    uint16_t _port;
    socket_ _listensock;
    struct pollfd _fds[max_managed_fds];
    Log &_log = Log::getInstance();
};