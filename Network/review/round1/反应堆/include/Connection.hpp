#pragma once

#include <netinet/in.h>  // 引入 in_port_t, sockaddr_in
#include <unistd.h>

#include <functional>
#include <memory>
#include <string>

class tcp_server;

class Connection : public std::enable_shared_from_this<Connection> {
   public:
    using socket = int;
    using string = std::string;
    using addr_t = std::string;
    using port_t = ::in_port_t;
    using sockaddr_t = ::sockaddr_in;
    using function = std::function<void(void)>;
    using self_weak_ptr = std::weak_ptr<Connection>;
    using self_shared_ptr = std::shared_ptr<Connection>;
    using function_base = std::function<void(std::weak_ptr<Connection>)>;

   private:
    Connection() : _socket(-1) {}

    Connection(int socket, const addr_t& addr, port_t port, const std::weak_ptr<tcp_server>& server)
        : _socket(socket), _is_register(false), _addr(addr), _port(port), _server(server) {}

    // 针对客户端的劣化版本
    Connection(int socket) : _socket(socket) {}

   public:
    static self_shared_ptr create() {
        return self_shared_ptr(new Connection);
    }

    static self_shared_ptr create(int fd, const addr_t& addr, port_t port,
                                  std::weak_ptr<tcp_server> server) {
        return self_shared_ptr(new Connection(fd, addr, port, server));
    }

    static self_shared_ptr create(int socket) {
        return self_shared_ptr(new Connection(socket));
    }

    ~Connection() {
        close();
    }

    self_weak_ptr weak_from_this() {
        return shared_from_this();
    }

    void close() {
        if (!is_closed()) {
            ::close(_socket);
            _socket = -1;
        }
    }

    bool is_closed() const {
        return _socket == -1;
    }

    void set_in_call(const function_base& call) {
        if (call) {
            _in_call = std::bind(call, weak_from_this());
        }
    }

    void set_out_call(const function_base& call) {
        if (call) {
            _out_call = std::bind(call, weak_from_this());
        }
    }

    void set_pri_call(const function_base& call) {
        if (call) {
            _pri_call = std::bind(call, weak_from_this());
        }
    }

    void set_err_call(const function_base& call) {
        if (call) {
            _err_call = std::bind(call, weak_from_this());
        }
    }

    void set_hup_call(const function_base& call) {
        if (call) {
            _hup_call = std::bind(call, weak_from_this());
        }
    }

    void set_unregister_call(const function_base& call) {
        if (call) {
            _unregister_call = std::bind(call, weak_from_this());
        }
    }

    void set_register_call(const function_base& call) {
        if (call) {
            _register_call = std::bind(call, weak_from_this());
        }
    }

    void in_call() {
        if (_in_call) {
            _in_call();
        }
    }

    void out_call() {
        if (_out_call) {
            _out_call();
        }
    }

    void pri_call() {
        if (_pri_call) {
            _pri_call();
        }
    }

    void err_call() {
        if (_err_call) {
            _err_call();
        }
    }

    void hup_call() {
        if (_hup_call) {
            _hup_call();
        }
    }

    void register_call() {
        if (_register_call) {
            _is_register = true;
            _register_call();
        }
    }

    void unregister_call() {
        if (_unregister_call) {
            _is_register = false;
            _unregister_call();
        }
    }

    bool& is_register() {
        return _is_register;
    }

    const std::weak_ptr<tcp_server>& server() const {
        return _server;
    }

    void set_file(int fd) {
        _socket = fd;
    }

    void set_server(const std::weak_ptr<tcp_server>& server) {
        _server = server;
    }

    int file() const {
        return _socket;
    }

    const addr_t& addr() const {
        return _addr;
    }

    port_t port() const {
        return _port;
    }

    string& inBuff() {
        return _inBuff;
    }

    string& outBuff() {
        return _outBuff;
    }

   private:
    socket _socket;
    bool _is_register;
    string _addr;
    port_t _port;
    string _inBuff;
    string _outBuff;

    function _in_call;          // epoll 事件可读时调用
    function _out_call;         // epoll 事件可写时调用
    function _pri_call;         // epoll 紧急数据可读时调用
    function _err_call;         // epoll 错误事件时调用
    function _hup_call;         // epoll 对端关闭事件调用
    function _register_call;    // 当Connection 被加入会话层时调用
    function _unregister_call;  // 当Connection 被移除会话层时调用

    std::weak_ptr<tcp_server> _server;
};
