#include "boost_log.hpp"
#include "server/udp_server.hpp"

#include <vector>

std::string func(size_t id, const std::string& buffer)
{
    (void)id;
    BOOST_LOG_TRIVIAL(info) << std::format("接收到一条消息: {}", buffer);
    std::string result = std::format("<回应 {}>", buffer);
    return result;
}

bool SecurityCheck(const std::string& str)
{
    static const std::vector<std::string> blacklists =
    {
        "rm", "sudo", "yum", "install", "uninstall",
        "mv", "cp",  "kill", "unlink"
    };

    for(const auto& e : blacklists)
    {
        if(std::string::npos != str.find(e))
            return true;
    }

    return false;
}

std::string ExcuteCommand(const std::string& str)
{
    if(SecurityCheck(str))
    {
        return "危险的指令";
    }

    FILE* fp = popen(str.c_str(), "r");
    if(fp == nullptr)
    {
        BOOST_LOG_TRIVIAL(warning) << std::format("无法识别的指令: {}", str);
        return strerror(errno);
    }

    std::string result;
    char buffer[4096] = {0};
    while(true)
    {
        char* str = fgets(buffer, sizeof(buffer), fp);
        if(str == nullptr) break;
        result += str;
    }

    fclose(fp);

    return result;
}

int main(int argc, char* argv[]) {
    init_logging();

    in_port_t port = 8080;
    if (argc != 2) {
        BOOST_LOG_TRIVIAL(warning) << std::format("没有指定端口号, 将使用缺省值: {}", port);
    } else {
        port = std::stoi(*(argv + 1) + 1);
    }

    udp_server server(port);
    server.run(func);
    return 0;
}