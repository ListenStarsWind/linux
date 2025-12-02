#pragma once

#include <signal.h>

#include <unordered_map>

#include "Epoller.hpp"
#include "tcp_protocol.temp.hpp"
#include "util/utils.hpp"

class tcp_server : public tcp_protocol {
    using on_event_call_t = socket_t::function_base;
    using on_message_call_t = std::function<std::string(std::string&)>;
    using connects_t = std::unordered_map<socket_t*,socket_ptr_t>;

    tcp_server(const on_message_call_t& call_back, const addr_t& addr = "0.0.0.0",
               port_t port = tcp_protocol::_server_port, int eppll_size = 64)
        : _call_back(call_back), _epoll(std::make_unique<Epoller>(eppll_size)) {
        _self = make_non_owning_shared(this);
        try {
            int listen_fd = tcp_protocol::socket_init();
            auto listen_socket = std::make_unique<socket_t>(listen_fd, addr, port, weak_from_this());
            _listen_socket = listen_socket->weak_from_this();
            _connects.emplace(listen_socket.get(), std::move(listen_socket));
            

            _connects.emplace_back(std::make_unique<socket_t>(listen_fd, addr, port, weak_from_this()));
            _listen_socket = _connects.back()->weak_from_this();
            _connects.back()->set_unregister_call(
                [](std::weak_ptr<socket_t> m) { auto me = m.lock(); });
        } catch (std::system_error& e) {
            BOOST_LOG_TRIVIAL(fatal) << std::format("服务初始化发生致命错误: {}", e.what());
            ::exit(e.code().value());
        }
    }

    std::weak_ptr<tcp_server> weak_from_this() const {
        return _self;
    }

   private:
    connects_t _connects;
    on_message_call_t _call_back;
    std::unique_ptr<Epoller> _epoll;
    std::shared_ptr<tcp_server> _self;
    std::weak_ptr<socket_t> _listen_socket;
};
