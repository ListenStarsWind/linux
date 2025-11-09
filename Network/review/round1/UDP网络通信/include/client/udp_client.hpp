#include <arpa/inet.h>  //  引入字节序转换接口
#include <arpa/inet.h>  // htons
#include <sys/socket.h>

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <format>
#include <iostream>
#include <string>  // getline

class udp_client {
    using socket_t = int;
    using socket_ptr_t = std::unique_ptr<socket_t, void(*)(socket_t*)>;
    using sockaddr_in_t = struct sockaddr_in;
    using in_port_t = ::in_port_t;
    using string = std::string;

   public:
    udp_client(in_port_t remote_port = 8080, const string& remote_ip = "47.107.254.122")
        : _isrunning(false),
          _remote_port(remote_port),
          _remote_ip(remote_ip),
           _sock(new socket_t(socket(AF_INET, SOCK_DGRAM, 0)), udp_client::sock_destroy) {
        if (*_sock < 0) {
            BOOST_LOG_TRIVIAL(error) << std::format("打开套接字失败: {}", strerror(errno));
            exit(errno);
        }

        BOOST_LOG_TRIVIAL(info) << std::format("套接字打开成功, 文件描述符为: {}", *_sock);
    }

    void run() {
        _isrunning = true;
        sockaddr_in_t remote;
        bzero(&remote, sizeof(remote));
        remote.sin_family = AF_INET;
        remote.sin_port = htons(_remote_port);
        remote.sin_addr.s_addr = inet_addr(_remote_ip.c_str());

        string message;
        char buffer[1024] = {0};
        while (true) {
            std::cout << "请输入 @ ";
            getline(std::cin, message);

            int ret = sendto(*_sock, message.c_str(), message.size(), 0,
                             reinterpret_cast<const sockaddr*>(&remote),
                             static_cast<socklen_t>(sizeof(remote)));

            if (ret < 0) {
                BOOST_LOG_TRIVIAL(warning) << std::format("出错的报文发送: {}", strerror(errno));
                continue;
            }

            ssize_t len = recv(*_sock, buffer, sizeof(buffer), 0);
            if (len < 0) {
                BOOST_LOG_TRIVIAL(warning) << std::format("出错的报文接收: {}", strerror(errno));
                continue;
            }

            buffer[len] = '\0';
            BOOST_LOG_TRIVIAL(info) << std::format("接收成功: \n{}", buffer);
        }
    }

    private:
     static void sock_destroy(socket_t* p)
    {
        close(*p);
        delete p;
    }

   private:
    bool _isrunning;
    in_port_t _remote_port;
    string _remote_ip;
    socket_ptr_t _sock;
};