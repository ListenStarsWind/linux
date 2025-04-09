#pragma once

#include "log.hpp"
#include "Sockst.hpp"
#include <string>
#include <pthread.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#define DEFAULT_POTR 8888
#define BUFFER_SIZE 10240
#define WEB_ROOT "wwwroot"
#define HTTP_HEADER_DELIMITER "\r\n"
#define HOME_PAGE "wwwroot/index.html"
#define COSMIC_404 "wwwroot/cosmic_404.html"

struct threadArgs
{
    int _sockfd;

    threadArgs(int sockfd) : _sockfd(sockfd) {};
    ~threadArgs()
    {
        if (_sockfd > 0)
            close(_sockfd);
    }
};

std::string readHtml(const std::string& resource_path)
{
    std::ifstream in(resource_path.c_str(), std::ios::binary);
    if(!in.is_open()) return std::string();
    // in.seekg(0, std::ios::end);                                     // 将文件指针移到文件末尾
    // size_t size = in.tellg();                                       // 获取当前文件指针距离文件开头的偏移量
    // in.seekg(0, std::ios::beg);                                     // 将文件指针重新移到开头
    // std::string result(size, ' ');
    // in.read(const_cast<char*>(result.c_str()), size);               // 把文件读到缓冲区

    std::string result((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
    in.close();
    return result;

    // std::ifstream in(resource_path.c_str());
    // if(!in.is_open()) return std::string();
    // std::string result, line;
    // while(std::getline(in, line))
    //     result += line;
    // in.close();
    // return result;

    // std::ifstream in(resource_path.c_str());
    // if(!in) in.open(COSMIC_404);  // operator bool 重载, 打开失败返回假
    // std::string result, line;
    // while (std::getline(in, line))
    //     result += line;
    // in.close();
    // return result;
}

class HttpRequest
{
public:
    void init(std::string req)
    {
        while (true)
        {
            size_t pos = req.find(HTTP_HEADER_DELIMITER);
            if (pos == std::string::npos)
                break;
            std::string temp = req.substr(0, pos);
            if (temp.empty())
                break;
            _req_header.emplace_back(temp);
            req.erase(0, pos + strlen(HTTP_HEADER_DELIMITER));
        }
        _body = req;
        // std::cout << _body<<std::endl;
    }

    void parse()
    {
        // std::cout << _body<<std::endl;
        std::string temp;
        std::stringstream ss(_req_header[0]);
        ss >> method_ >> url_query_ >> http_version_;
        size_t pos = url_query_.find('?');
        if (pos == std::string::npos)
            temp = url_query_;
        temp = url_query_.substr(0, pos);
        url_query_.erase(0, pos);
        if (temp == "/")
        {
            path_ = HOME_PAGE;
        }
        else
        {
            path_ += WEB_ROOT;
            path_ += temp;
        }
        mime_ = "text/html";
        pos = path_.rfind('.');
        if(pos == std::string::npos) return;
        temp = path_.substr(pos);
        if(temp == ".jpg" || temp == ".jpeg") mime_ = "image/jpeg";
        // std::cout << _body<<std::endl;
    }

    const std::string& getPath()
    {
        return path_;
    }

    const std::string& getMime()
    {
        return mime_;
    }

    // AI以我的原型代码改出来的
    void print()
    {
        // 标题（亮青色+下划线）
        std::cout << "\033[1;4;36m" // 亮青色+粗体+下划线
                  << "HTTP Request Details"
                  << "\033[0m" << std::endl
                  << std::endl;

        // 元信息（黄色标签+白色值）
        std::cout << "\033[33mMethod: \033[37m" << method_ << std::endl
                  << "\033[33mPath:   \033[37m" << path_ << std::endl
                  << "\033[33mHTTP:   \033[37m" << http_version_ << std::endl
                  << "\033[33mQuery:  \033[37m" << (url_query_.empty() ? "(empty)" : url_query_) << std::endl;

        // 请求头（品红色标题+绿色内容）
        if (!_req_header.empty())
        {
            std::cout << std::endl
                      << "\033[35m" << "── Headers ──" << "\033[0m" << std::endl;
            for (auto it = std::next(_req_header.begin()); it != _req_header.end(); ++it)
            {
                std::cout << "\033[32m" << *it << HTTP_HEADER_DELIMITER << "\033[0m" << std::endl;
            }
        }

        std::cout << _body<<std::endl;
        // 请求体（橙色标题+灰色内容）
        if (!_body.empty())
        {
            std::cout << std::endl
                      << "\033[38;5;208m" << "── Body ──" << "\033[0m" << std::endl
                      << "\033[90m" << _body << "\033[0m" << std::endl;
        }

        // 结尾线
        std::cout << "\033[90m" << "────────────────────" << "\033[0m" << std::endl;
    }

    // void print()
    // {
    //     std::cout << "--------------------" << std::endl;
    //     auto it = _req_header.begin();
    //     ++it;
    //     std::cout << "method       : " << method_ << std::endl;
    //     std::cout << "path         : " << path_ << std::endl;
    //     std::cout << "http_version : " << http_version_ << std::endl;
    //     std::cout << "url_query    : " << url_query_ << std::endl;
    //     while (it != _req_header.end())
    //     {
    //         std::cout << *it << HTTP_HEADER_DELIMITER;
    //         ++it;
    //     }
    //     std::cout << _body << std::endl;
    // }

private:
    std::vector<std::string> _req_header; // 把非正文部分以每行为单位, 存在数组里
    std::string _body;                    // 存正文

    std::string method_;
    std::string url_query_;
    std::string http_version_;

    std::string path_;
    std::string mime_;
};

class HttpServer
{
public:
    HttpServer(uint16_t port = DEFAULT_POTR) : _port(port) {};
    void start()
    {
        _listensock.create_();
        _listensock.reuse_port_address();
        _listensock.bind_(_port);
        _listensock.listen_();
        // print_netstat();

        while (true)
        {
            std::string clientip;
            uint16_t clientport;
            int sockfd = _listensock.accept_(&clientip, &clientport);
            if (sockfd == -1)
                continue;

            pthread_t session;
            threadArgs *a = new threadArgs(sockfd);
            pthread_create(&session, nullptr, threadRun, a);
        }
    }
    ~HttpServer() {}

private:
    static void *threadRun(void *args_)
    {
        pthread_detach(pthread_self());
        char buffer[BUFFER_SIZE];
        std::shared_ptr<threadArgs> args(reinterpret_cast<threadArgs *>(args_));
        ssize_t n = recv(args->_sockfd, buffer, sizeof(buffer) - 1, 0);
        // std::cout << n << std::endl;
        if (n > 0)
        {
            buffer[n] = 0;
            HttpRequest req;
            req.init(buffer); // 默认报文完整
            req.parse();
            req.print();
            // std::cout <<buffer;

            // 返回响应的过程     初学阶段, 硬编码
            std::string body = readHtml(req.getPath());
            std::string response_line = "HTTP/1.0 200 OK\r\n";
            if(body.empty())
            {
                body = readHtml(COSMIC_404);
                response_line = "HTTP/1.0 404 Not Found\r\n";
            }
            // response_line = "HTTP/1.0 302 Found\r\n";               // 硬编码
            std::string response_header = "Content-Length: ";
            response_header += std::to_string(body.size());
            response_header += "\r\n"; // 把"Content-Length"这行末尾加换行
            response_header += "Content-Type: ";
            response_header += req.getMime();
            response_header += "\r\n"; // 把"Content-Type"这行末尾加换行
            response_header += "Set-Cookie: name=2131";
            response_header += "\r\n"; // 把"Set-Cookie"这行末尾加换行
            response_header += "Set-Cookie: passwd=12345";
            response_header += "\r\n"; // 把"Set-Cookie"这行末尾加换行
            // response_header += "Location: https://www.qq.com";
            // response_header += "\r\n"; // 把"Location"这行末尾加换行
            response_header += "\r\n"; // 加一个空行, 作为报头和正文的分隔符
            std::string response = response_line;
            response += response_header;
            std::cout << response<<std::endl;    // 不看正文了
            response += body;
            send(args->_sockfd, response.c_str(), response.size(), 0); // 差错处理不做了, send和write的关系l类似于read和recv
        }
        return nullptr;
    }

private:
    socket_ _listensock;
    uint16_t _port;
    Log &_log = Log::getInstance();
};