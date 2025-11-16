#include "boost_log.hpp"
#include "client/tcp_client.hpp"
#include "codec.hpp"
#include <random>
#include "counter.hpp"

int main(int argc, char* argv[]) {
    init_logging();
    std::string server_addr = tcp_protocol::_server_addr;
    in_port_t server_port = tcp_protocol::_server_port;
    if (argc != 3) {
        BOOST_LOG_TRIVIAL(warning) << std::format("未显示指定服务端地址, 将使用缺省值");
    } else {
        server_addr = *(argv + 1);
        server_port = std::stoi(*(argv + 2));
    }
    BOOST_LOG_TRIVIAL(info) << std::format("服务端地址确认: \"{}:{}\"", server_addr, server_port);
    tcp_client client(server_addr, server_port);

    // 准备客户端的istream
    std::mt19937_64 rng{ std::random_device{}() };
    std::uniform_real_distribution<double> dist_double(-1e3, 1e3);
    std::uniform_int_distribution<int> dist_int(0, 3);
    std::stringbuf buf(std::ios::in | std::ios::out);
    std::iostream io(&buf);
    for(int i = 0; i < 10; i++)
    {
        auto x = dist_double(rng);
        auto op = counter::_ops[dist_int(rng)];
        auto y = dist_double(rng);
        std::string playout = std::format("{}{}{}{}{}", x, counter::_space, op, counter::_space, y);
        io << codec::pack(playout);
    }

    client.run(io);
    
    return 0;
}
