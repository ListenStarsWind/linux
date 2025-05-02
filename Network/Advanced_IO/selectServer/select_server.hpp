#pragma once

#include"log.hpp"
#include"Sockst.hpp"
#include <iostream>
#include <sys/select.h>
#include <sys/time.h>
#include <string>
#include <algorithm>


using namespace std;

static const uint16_t defaultport = 8080;
static const int max_managed_fds = sizeof(fd_set) * 8;

class SelectServer
{
    public:
        SelectServer(uint16_t port = defaultport):_port(port) {
            fill(_fds, _fds + max_managed_fds, -1);
        }

        ~SelectServer(){ _listensock.close_();}

        void init()
        {
            _listensock.create_();
            _listensock.reuse_port_address();
            _listensock.bind_(_port);
            _listensock.listen_();
            // print_netstat();
        }

        int fd_set__(fd_set& fds)
        {
            int end = 0;
            cout << "被托管的文件描述符: ";
            for(int i = 0; i < max_managed_fds; ++i)
            {
                if(_fds[i] != -1)
                {
                    cout << _fds[i]<<" ";
                    FD_SET(_fds[i], &fds);
                    end = max(_fds[i], end);
                }
            }
            cout << endl;
            return end + 1;
        }

        void start()
        {
            _fds[0] = _listensock;
            // 把监听套接字和普通套接字都使用`select`进行管理
            for(;;)
            {
                // 对于监听套接字来说, 从连接队列中获取新连接这个行为是读事件, 所以应该设置`readfds`

                fd_set readfds;
                FD_ZERO(&readfds); // 栈上对象要清空;
                int nfds = fd_set__(readfds);
                // FD_SET(static_cast<int>(_listensock), &readfds); // 把监听套接字的对应位标记上
                // struct timeval timeout;
                // timeout.tv_sec = 2; timeout.tv_usec = 0; // 等待两秒
                // cout << "剩余时间: "<<timeout.tv_sec<<"秒"<<endl<<endl;
                int n = select(nfds, &readfds, nullptr, nullptr, nullptr);
                // if(n >= 0)
                // {
                //     cout << n << "个读事件已就绪↓"<<endl;
                //     bitmap_print(static_cast<int>(_listensock) + 1, readfds);
                //     // cout << "剩余时间: "<<timeout.tv_sec<<"秒"<<endl;
                //     cout<<endl;
                // }
                if(n == 0)
                    cout << "无有效事件发生"<<endl;
                else if(n > 0)
                    event_response(readfds);
                else
                    cerr << "select error!"<<endl;

                // sleep(1);
            }
        }

        void event_response(fd_set& readfds)
        {
            cout <<endl<< "开始事件响应"<<endl;
            if(FD_ISSET(static_cast<int>(_listensock), &readfds))
                get_new_link();
            for(int i = 1; i < max_managed_fds; ++i)
            {
                if(_fds[i] != -1 && FD_ISSET(_fds[i], &readfds))
                    session(i);
            }
        }

        // 我们先不考虑面向字节流而导致的粘包问题
        // 后面还有更好的多路转接方案, 到时候我们再严谨的走一遍整个流程
        void session(int idx)
        {
            _log(Info, "收到一个新的消息, 来自%d", _fds[idx]);

            char buff[64] = {0};
            int n = read(_fds[idx], buff, sizeof(buff));
            if(n > 0)
            {
                buff[n] = 0;
                cout << "用户说: "<<buff<<endl;
            }
            else if(n == 0)
            {
                _log(Warning, "用户退出了连接");
                close(_fds[idx]);
                _fds[idx] = -1;
            }
            else
            {
                _log(Warning, "错误的读数据");
                close(_fds[idx]);
                _fds[idx] = -1;
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

            // something
            add_link(sock);
        }

        void add_link(int sock)
        {
            int idx = 1;
            while(idx < max_managed_fds && _fds[idx] != -1) ++idx;
            if(idx == max_managed_fds)
            {
                _log(Warning, "托管数目超出限制, 连接关闭:%d", sock);
                close(sock);
            }
            else
                _fds[idx] = sock;
        }

        void bitmap_print(int nfds, const fd_set& bitmap)
        {
            // 系统里的位图是静态的, 想打印也需要一个nfds
            for(int i = 0; i < nfds; ++i)
            {
                cout<<FD_ISSET(i, &bitmap);
            }
            cout << endl;
        }

    private:
    uint16_t _port;
    socket_ _listensock;
    int _fds[max_managed_fds];
    Log &_log = Log::getInstance();
};