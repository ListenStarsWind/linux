#include <arpa/inet.h>  //  引入字节序转换接口
#include <arpa/inet.h>  // htons
#include <sys/socket.h>

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <format>
#include <functional>
#include <iostream>
#include <string>  // getline
#include <thread>

class udp_client {
    using socket_t = int;
    using socket_ptr_t = std::unique_ptr<socket_t, void (*)(socket_t*)>;
    using sockaddr_in_t = struct sockaddr_in;
    using in_port_t = ::in_port_t;
    using string = std::string;
    using self_t = udp_client;
    using send_t = std::function<ssize_t(const string&)>;
    using recv_t = std::function<ssize_t(char*, size_t)>;
    using thread = std::thread;

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

        sockaddr_in_t remote;
        bzero(&remote, sizeof(remote));
        remote.sin_family = AF_INET;
        remote.sin_port = htons(_remote_port);
        remote.sin_addr.s_addr =
            inet_addr(_remote_ip.c_str());

        _send = [remote, fd = *_sock](const string& message) -> ssize_t {
            return ::sendto(fd, message.c_str(), message.size(), 0,
                            reinterpret_cast<const sockaddr*>(&remote),
                            static_cast<socklen_t>(sizeof(remote)));
        };

        _recv = [fd = *_sock](char* buff, size_t buff_len) -> ssize_t {
            return ::recv(fd, buff, buff_len, 0);
        };
    }

    void run() {
        _isrunning = true;
        thread sender(&udp_client::push, this);
        thread recver(&udp_client::poll, this);

        sender.join();
        recver.join();

        _isrunning = false;
    }

   private:
    static void sock_destroy(socket_t* p) {
        close(*p);
        delete p;
    }

    static void push(self_t* me) {
        string message;
        while (true) {
            std::cout << "请输入 @ ";
            getline(std::cin, message);

            int ret = me->_send(message);

            if (ret < 0) {
                BOOST_LOG_TRIVIAL(warning) << std::format("出错的报文发送: {}", strerror(errno));
                continue;
            }

            BOOST_LOG_TRIVIAL(info) << std::format("报文发送成功");
        }
    }

    static void poll(self_t* me) {
        char buffer[_buff_size] = {0};
        while (true) {
            ssize_t len = me->_recv(buffer, sizeof(buffer));
            if (len < 0) {
                BOOST_LOG_TRIVIAL(warning) << std::format("出错的报文接收: {}", strerror(errno));
                return;
            }
            buffer[len] = '\0';
            BOOST_LOG_TRIVIAL(info) << std::format("接收成功: \n{}", buffer);
        }
    }

   private:
    bool _isrunning;
    in_port_t _remote_port;
    string _remote_ip;
    socket_ptr_t _sock;
    send_t _send;
    recv_t _recv;
    inline static const size_t _buff_size = 1024;
};