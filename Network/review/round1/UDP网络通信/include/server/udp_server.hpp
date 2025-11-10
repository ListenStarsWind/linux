#pragma once

#include <arpa/inet.h>   //  引入字节序转换接口
#include <netinet/in.h>  // struct sockaddr_in
#include <sys/socket.h>
#include <sys/types.h>

#include <boost/log/trivial.hpp>  // 引入Boost全局日志宏
#include <format>
#include <functional>
#include <memory>  // 引入智能指针
#include <string>
#include <unordered_map>

class udp_server {
    using socket_t = int;
    using user_id_t = size_t;
    using socket_ptr_t = std::unique_ptr<socket_t, void (*)(socket_t*)>;
    using sockaddr_in_t = struct sockaddr_in;
    using in_port_t = ::in_port_t;
    using string = std::string;
    using function_t = std::function<string(user_id_t, const string&)>;
    using link_t = std::function<ssize_t(const string&)>;
    using users_t = std::unordered_map<user_id_t, link_t>;
    using addrs_t = std::unordered_map<in_port_t, user_id_t>;

   public:
    udp_server(in_port_t port = 8080, const string& ip = "0.0.0.0")
        : _isrunning(false),
          _port(port),
          _ip(ip),
          _sock(new socket_t(socket(AF_INET, SOCK_DGRAM, 0)), udp_server::sock_destroy) {
        if (*_sock < 0) {
            BOOST_LOG_TRIVIAL(error) << std::format("打开套接字失败: {}", strerror(errno));
            exit(errno);
        } else {
            BOOST_LOG_TRIVIAL(info) << std::format("套接字打开成功, 文件描述符为: {}", *_sock);
        }

        sockaddr_in_t local;
        bzero(&local, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = inet_addr(_ip.c_str());

        int ret = bind(*_sock, reinterpret_cast<const struct sockaddr*>(&local), sizeof(local));

        if (ret != 0) {
            BOOST_LOG_TRIVIAL(error) << std::format("套接字绑定失败: {}", strerror(errno));
            exit(errno);
        }

        BOOST_LOG_TRIVIAL(info) << std::format("套接字绑定成功");
    }

    void run(const function_t& func) {
        _isrunning = true;
        char buffer[1024] = {0};
        while (true) {
            sockaddr_in_t remote;
            socklen_t size = sizeof(remote);
            ssize_t len = recvfrom(*_sock, buffer, sizeof(buffer) - 1, 0,
                                   reinterpret_cast<struct sockaddr*>(&remote),
                                   reinterpret_cast<socklen_t*>(&size));
            if (len < 0) {
                BOOST_LOG_TRIVIAL(warning) << std::format("出错的报文接收: {}", strerror(errno));
                continue;
            }

            buffer[len] = '\0';

            user_id_t id_ = _addrs.size();
            in_addr_t addr = remote.sin_addr.s_addr;
            auto it = _addrs.find(addr);
            if (it == _addrs.end()) {
                _addrs.emplace(addr, id_);

                link_t send = [fd = *_sock, remote](const string& echo) -> ssize_t {
                    return sendto(fd, echo.c_str(), echo.size(), 0,
                                  reinterpret_cast<const struct sockaddr*>(&remote), sizeof(remote));
                };

                _users.emplace(id_, std::move(send));
            } else {
                id_ = it->second;
            }

            string echo = func(id_, buffer);

            for (const auto& [id, send] : _users) {
                if (id != id_) {
                    int ret = send(echo);
                    if (ret < 0) {
                        BOOST_LOG_TRIVIAL(warning)
                            << std::format("向{}回应失败: {}", id, strerror(errno));
                        continue;
                    }
                    BOOST_LOG_TRIVIAL(info) << std::format("向{}发送成功: {}", id, echo);
                }
            }
        }
    }

   private:
    static void sock_destroy(socket_t* p) {
        close(*p);
        delete p;
    }

   private:
    bool _isrunning;
    in_port_t _port;
    string _ip;
    socket_ptr_t _sock;
    users_t _users;
    addrs_t _addrs;
};
