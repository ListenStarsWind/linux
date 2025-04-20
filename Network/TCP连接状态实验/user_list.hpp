#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unordered_map>

namespace wind
{
    class user
    {
    public:
        user() {}
        user(const int &fd, struct sockaddr_in &sock) : _fd(fd), _sock(sock)
        {
            _port = ntohs(_sock.sin_port);
            char ipbuff[32];
            inet_ntop(AF_INET, &_sock.sin_addr, ipbuff, sizeof(ipbuff));
            _ip = ipbuff;
        }
        uint16_t get_port() { return _port; }
        const std::string &get_ip() { return _ip; }

    private:
        int _fd;
        uint16_t _port;
        std::string _ip;
        sockaddr_in _sock;
    };

    
}