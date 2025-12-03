#pragma once

#include "client/SessionAdapter.hpp"
#include "tcp_protocol.hpp"

class tcp_client : public tcp_protocol {
    using self_shared_ptr = std::shared_ptr<tcp_client>;

   private:
    tcp_client(SessionAdapterBase* SessionAdapter,
               const addr& server_addr = tcp_protocol::_server_addr,
               port server_port = tcp_protocol::_server_port)
        : _SessionAdapter(SessionAdapter), _server_addr(server_addr), _server_port(server_port) {}

   public:
    static self_shared_ptr create(SessionAdapterBase* SessionAdapter,
                                  const addr& server_addr = tcp_protocol::_server_addr,
                                  port server_port = tcp_protocol::_server_port) {
        auto client = self_shared_ptr(new tcp_client(SessionAdapter, server_addr, server_port));
        return client;
    }

    ~tcp_client() override = default;

    // 在客户端, 它是短连接的
    void run() override {
        int fd = tcp_protocol::socket_init();
        auto server = connection::create(fd);
        connect(server);
        std::string message = _SessionAdapter->send();
        char buffer[4096] = {0};
        start_running();

        ::write(server->file(), message.c_str(), message.size());

        ssize_t len = ::read(server->file(), buffer, sizeof(buffer));
        if (len <= 0) {
            BOOST_LOG_TRIVIAL(error) << std::format("与服务器通信不畅: {}", strerror(errno));
        }

        std::string response(buffer, buffer + len);
        try {
            _SessionAdapter->recv(response);
        } catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error) << std::format("会话层之上出现了异常: {}", e.what());
        }

        stop_running();
    }

   private:
    void connect(connect_weak_ptr s) {
        auto server = s.lock();
        int reconnection = 10;
        while (reconnection--) {
            bool ret = tcp_protocol::connect(server, _server_addr, _server_port);
            if (ret == true) return;
            BOOST_LOG_TRIVIAL(info) << std::format("正在尝试重新连接至服务器");
            ::sleep(1);
        }
        BOOST_LOG_TRIVIAL(error) << std::format("重连的次数过多, 请等待网络好转后重试");
        ::exit(errno);
    }

   private:
    SessionAdapterBase* _SessionAdapter;
    addr _server_addr;
    port _server_port;
};