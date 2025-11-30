#include <memory>
#include <random>
#include <vector>

#include "boost_log.hpp"
#include "client/tcp_client.hpp"
#include "codec.hpp"

int main(int argc, char* argv[]) {
    (void)argc, (void)argv;
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

    std::vector<std::string> ops{
        "+", "-", "*",  "/",              // 基本算术运算
        "%", "^", "&&", "||", ">>", "<<"  // 其他自定义操作符示例
    };

    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist_double(-1e3, 1e3);
    std::uniform_int_distribution<int> dist_int(0, ops.size() - 1);
    double left = dist_double(rng);
    std::string op = ops[dist_int(rng)];
    double right = dist_double(rng);

    auto requestApp = std::make_unique<CalculatorRequestApp1>(left, op, right);
    auto pesponseApp = std::make_unique<CalculatorPesponseApp1>(left, op, right);
    auto errorApp = std::make_unique<CalculatorErrorApp1>(left, op, right);

    auto sessionAdapter = std::make_unique<SessionAdapter>();
    sessionAdapter->register_send_app(static_cast<CalculatorAppBase1*>(requestApp.get()));
    sessionAdapter->register_recv_app(static_cast<CalculatorAppBase1*>(pesponseApp.get()),
                                      static_cast<CalculatorAppBase1*>(errorApp.get()));

    sessionAdapter->select_send_app(codec::message_type::REQUEST);

    tcp_client client(static_cast<SessionAdapterBase*>(sessionAdapter.get()), server_addr,
                      server_port);

    client.run();

    return 0;
}
