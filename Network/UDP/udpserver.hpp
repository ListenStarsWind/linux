#pragma once

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <unordered_map>
 #include <functional>
 #include <string.h>
 #include <string>
 #include <sstream>
 #include <utility>
 #include "log.hpp"

 // 我觉得用单例模式更适合些, 但这里主题是网络, 就用全局变量凑活用
 // 多加两个下划线防止标识符冲突
 wind::Log log__;

 #define DEFAULT_ADDRESS  "0.0.0.0"
 #define DEFAULT_PORT     8080
 #define BUFFER_SIZE      1024

 namespace wind
 {
    enum
    {
        SOCKET_ERR = 0,
        BIND_ERR= 1
    };

     class udpserver
     {
        typedef std::function<std::string(const std::string&, const std::string&, const uint16_t&)> function__type;
        typedef std::unordered_map<std::string, struct sockaddr_in> user_list_type;

     public:
     udpserver(const uint16_t& port = DEFAULT_PORT, const char* id = DEFAULT_ADDRESS) :_sockfd(0), _port(port), _id(id), _isrunning(false) {}

         void init()
         {
             _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd < 0)
            {
                log__(Fatal, " socket create error: %s", strerror(errno));
                exit(SOCKET_ERR);
            }
            log__(Info, " socket create success, sockfd: %d", _sockfd);

            struct sockaddr_in local;  // 服务端本地的套接字
            bzero(&local, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            local.sin_addr.s_addr = inet_addr(_id.c_str());
            // local.sin_addr.s_addr = inet_addr(INADDR_ANY);  // 因为是零, 所以其实转不转都一样
            if(bind(_sockfd, reinterpret_cast<const struct sockaddr *>(&local), static_cast<socklen_t>(sizeof(local))) != 0)
            {
                log__(Fatal, " bind error: %s", strerror(errno));
                exit(BIND_ERR);
            }
            log__(Info, " bind success");
         }

         void run()
         {
            _isrunning = true;
            char buffer[BUFFER_SIZE];   // 约定通信内容为字符串
            while(true)
            {
                struct sockaddr_in remote;
                socklen_t size = sizeof(remote);
                ssize_t len = recvfrom(_sockfd, buffer, BUFFER_SIZE - 1, 0, reinterpret_cast<struct sockaddr *>(&remote), reinterpret_cast<socklen_t*>(&size));
                if(len < 0)
                {
                    log__(Warning, " recvfrom error: %s", strerror(errno));
                    continue;
                }

                std::string who = RegisterNewUser(remote);

                buffer[len] = 0;
                _message = buffer;

                Broadcast(who);

                // // 解析远端IP和端口, 交由外层处理
                // uint16_t port = ntohs(remote.sin_port);
                // std::string ip = inet_ntoa(remote.sin_addr);

                // // 如果负载数据是整数或包含多字节的数值, 应该转换为本机字节序
                // // 但对于字符串来说, 由于构成字符串的基本元素, 字符就一个字节, 所以大小端都是一样的
                // buffer[len] = '\0';
                // std::string temp(buffer);
                // // 字符串加工,证明确实来过服务端
                // std::string result = func(temp, ip, port);

                // if(sendto(_sockfd, result.c_str(), result.size(), 0, reinterpret_cast<const sockaddr*>(&remote), static_cast<socklen_t>(sizeof(remote))) == -1)
                // {
                //     log__(Warning, "sendto error: %s", strerror(errno));
                // }
                // // 避免干扰
                // // log__(Info, "sendto success ");
            }
         }

     private:
         std::string RegisterNewUser(const struct sockaddr_in &user)
         {
             std::string ip = inet_ntoa(user.sin_addr);
             if (_online_user.find(ip) == _online_user.end())
             {
                _message = ip;
                _message += " joins group chat";
                log__(Info, _message.c_str());
                _online_user.emplace(ip, user);
                // Broadcast();
             }
             return std::move(ip);
         }

         void Broadcast(std::string& who)
         {
            _message += "\n";

            std::stringstream oss;
            oss << "["<<who<<":"<<ntohs(_online_user[who].sin_port)<<"]$ "<<_message;
            _message = oss.str();
            // log__(Info, _message.c_str());

            // ip充当账号信息
            // [ip, user]是C++17语法
            for(const auto& [ip, user] : _online_user)
            {
                if(who == ip) continue;
                log__(Info, "正在向%s发送消息", ip.c_str());
                if(sendto(_sockfd, _message.c_str(), _message.size(), 0, reinterpret_cast<const sockaddr*>(&user), static_cast<socklen_t>(sizeof(user))) == -1)
                {
                    log__(Warning, "sendto error: %s", strerror(errno));
                }
            }
         }

     private:
         int _sockfd;       // 套接字句柄
         in_port_t _port;   // 端口号
         std::string _id;   // 点分十进制IP
         bool _isrunning;
         std::string _message;
         user_list_type _online_user;
     };
 }