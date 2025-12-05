#pragma once

#include <fcntl.h>
#include <signal.h>

#include <unordered_map>

#include "Epoller.hpp"
#include "tcp_protocol.hpp"

class tcp_server : public std::enable_shared_from_this<tcp_server>, public tcp_protocol {
    using EpollerItem = Epoller::EpollerItem;
    using self_weak_ptr = std::weak_ptr<tcp_server>;
    using on_event_call_t = connection::function_base;
    using self_shared_ptr = std::shared_ptr<tcp_server>;
    using on_message_call_t = std::function<std::string(std::string&)>;
    using connects = std::unordered_map<connection*, connect_shared_ptr>;

   private:
    tcp_server(const on_message_call_t& call_back, const addr& addr, port port, int epoll_size)
        : _call_back(call_back),
          _listen_addr(addr),
          _listen_port(port),
          _epoll(std::make_unique<Epoller>(epoll_size)) {}

   public:
    ~tcp_server() override = default;

    static self_shared_ptr create(const on_message_call_t& call_back, const addr& addr = "0.0.0.0",
                                  port port = tcp_protocol::_server_port, int epoll_size = 64) {
        auto server = self_shared_ptr(new tcp_server(call_back, addr, port, epoll_size));
        server->init();
        return server;
    }

    void run() override {
        start_running();
        while (is_running()) {
            // BOOST_LOG_TRIVIAL(debug) << std::format("注册的连接个数 {}", _connects.size());
            dispatch(-1);
        }
        stop_running();
    }

    const addr& get_addr() {
        return _listen_addr;
    }

    port get_port() {
        return _listen_port;
    }

   private:
    void init() {
        try {
            sockaddr listen_sockaddr;
            tcp_protocol::sockaddr_in_init(listen_sockaddr, _listen_addr, _listen_port);
            int listen_fd = tcp_protocol::socket_init();
            tcp_server::SetNonBlock(listen_fd);
            auto listen_socket = connection::create(listen_fd, _listen_addr, _listen_port,
                                                    shared_from_this(), "监听套接字");
            tcp_protocol::bind_port(listen_socket, listen_sockaddr);
            tcp_protocol::listen(listen_socket);

            listen_socket->set_register_call(
                std::bind(tcp_server::connect_register, std::placeholders::_1,
                          Epoller::EpollerItem::inreadable | Epoller::EpollerItem::edge_trigger));

            listen_socket->set_unregister_call(tcp_server::connect_unregister);

            listen_socket->set_in_call(tcp_server::accept_connect);

            listen_socket->register_call();
            ::signal(SIGPIPE, SIG_IGN);
        } catch (std::system_error& e) {
            BOOST_LOG_TRIVIAL(fatal) << std::format("服务初始化发生致命错误: {}", e.what());
            ::exit(e.code().value());
        }
    }

    template <class LogExpr>
    inline static void connect_common(connect_weak_ptr connect, LogExpr&& log_expr) {
        auto socket = connect.lock();
        if (!socket) throw std::runtime_error("连接生命周期过早结束");

        auto server = socket->server().lock();
        if (!server) throw std::runtime_error("服务端会话层生命周期过早结束");

        // 自定义日志
        log_expr(*socket);

        // 查找并删除
        auto it = server->_connects.find(socket.get());
        if (it == server->_connects.end()) throw std::runtime_error("连接移除时找不到句柄");

        server->_epoll->del(socket);
        server->_connects.erase(it);
        // 3: dispatch 一个, 文件错误对端关闭转化为 IO 问题, recv/send lock 一个,
        // 这个内联函数lock一个
        if (socket.use_count() > 3) {
            BOOST_LOG_TRIVIAL(error) << "针对连接存在疑似内存泄露现象";
        }

        socket->close();
        socket->is_register() = false;
    }

    void dispatch(int timeout) {
        // 注意 HUP/EOF 实际上也算作可读事件, 只不过划分更细了
        auto items = _epoll->wait(timeout);
        // BOOST_LOG_TRIVIAL(debug) << std::format("待处理的连接数: {}", items.size());
        for (auto& item : items) {
            // 简化逻辑, 将文件错误和对端关闭统一转换成 IO 问题
            // BOOST_LOG_TRIVIAL(debug) << std::format("一个{}准备好了{}", item.connect()->remark(),
            //                                         EpollerItem::event_name(item.events()));
            if (item.events() & Epoller::EpollerItem::error) {
                item.events() |=
                    (Epoller::EpollerItem::inreadable | Epoller::EpollerItem::outwritable);
            }
            if (item.events() & Epoller::EpollerItem::hangup) {
                item.events() |=
                    (Epoller::EpollerItem::inreadable | Epoller::EpollerItem::outwritable);
            }

            auto connect = item.connect()->shared_from_this();

            if (item.events() & Epoller::EpollerItem::inreadable) {
                connect->in_call();
            }

            // 这个连接实际上已经失效, 不需要继续判断
            if (connect->is_closed()) continue;
            // 这个连接在会话层已经被注销, 不需要继续判断
            if (!connect->is_register()) continue;

            if (item.events() & Epoller::EpollerItem::outwritable) {
                connect->out_call();
            }
        }
    }

    self_weak_ptr weak_from_this() {
        return shared_from_this();
    }

    // 在 epoll 模型中, EpollerItem::error 和 EpollerItem::hangup 是"强制触发型"事件
    // 即使我们没有显式关心, wait 的时候仍旧可以等到
    static void connect_register(connect_weak_ptr connect, uint32_t events) {
        auto socket = connect.lock();
        if (!socket) throw std::runtime_error("连接生命周期过早结束");
        auto server = socket->server().lock();
        if (!server) throw std::runtime_error("服务端会话层生命周期过早结束");

        // 当 add 抛出异常后, 因为还没有注册到会话层, 所以会话层不用做对应处理
        // 也就是说这两句的顺序是不能颠倒的, 如果颠倒, 我们就需要在外界注销连接
        // 但连接本身引用计数归零, 我们找不到控制它的句柄, 就无法注销, 从而形成内存泄露
        server->_epoll->add(socket, events);
        server->_connects.emplace(socket.get(), socket);

        // 三个: 调用注册回调的, 回调里的lock, 注册到map里的
        // BOOST_LOG_TRIVIAL(debug) << std::format("当前引用计数为: {}", socket.use_count());
    }

    static void connect_unregister(connect_weak_ptr connect) {
        connect_common(connect, [&](auto& s) {
            BOOST_LOG_TRIVIAL(info) << std::format("会话(\"{}:{}\")注销开始", s.addr(), s.port());
        });
    }

    static void connect_hangup(connect_weak_ptr connect) {
        connect_common(connect, [&](auto& s) {
            BOOST_LOG_TRIVIAL(info)
                << std::format("连接(\"{}:{}\")已经完全关闭, 将执行注销回调", s.addr(), s.port());
        });
    }

    static void connect_rdhangup(connect_weak_ptr connect) {
        connect_common(connect, [&](auto& s) {
            BOOST_LOG_TRIVIAL(info)
                << std::format("对端(\"{}:{}\")关闭了连接, 将执行注销回调", s.addr(), s.port());
        });
    }

    static void connect_error(connect_weak_ptr connect) {
        connect_common(connect, [&](auto& s) {
            BOOST_LOG_TRIVIAL(info)
                << std::format("对端(\"{}:{}\")错误, 将执行注销回调", s.addr(), s.port());
        });
    }

    static void accept_connect(connect_weak_ptr connect) {
        auto listen_socket = connect.lock();
        if (!listen_socket) throw std::runtime_error("监听连接生命周期过早结束");
        auto server = listen_socket->server().lock();
        if (!server) throw std::runtime_error("服务端会话层生命周期过早结束");
        sockaddr user_sockaddr;
        while (true) {
            tcp_protocol::bzero(user_sockaddr);
            int fd = tcp_protocol::accept(listen_socket, user_sockaddr);
            if (fd >= 0) {
                try {
                    // 若 new 抛出异常, 就不用考虑了, 那时候应该在意系统
                    auto socket = connection::create(fd, tcp_protocol::inet_ntop(user_sockaddr),
                                                     user_sockaddr.sin_port, server, "普通套接字");
                    // 若它抛出异常, 会话层不需要进行任何移除行为
                    tcp_server::SetNonBlock(fd);

                    socket->set_register_call(
                        std::bind(tcp_server::connect_register, std::placeholders::_1,
                                  EpollerItem::inreadable | EpollerItem::edge_trigger));

                    socket->set_unregister_call(tcp_server::connect_unregister);

                    socket->set_rdh_call(tcp_server::connect_rdhangup);
                    socket->set_hup_call(tcp_server::connect_hangup);
                    socket->set_err_call(tcp_server::connect_error);
                    socket->set_in_call(tcp_server::recv_connect);
                    socket->set_out_call(tcp_server::send_connect);

                    // 保证如果抛出异常, 则不会注册到会话层中
                    // 而 class socket 内部析构会关闭文件描述符
                    socket->register_call();

                } catch (std::exception& e) {
                    BOOST_LOG_TRIVIAL(warning)
                        << std::format("处理新连接的时候出现了异常: {}", e.what());
                    // 这里的异常不需要做额外后续处理
                }
            } else {
                // 彻底读完了
                if (errno == EWOULDBLOCK) return;
                // 被信号打断
                else if (errno == EINTR)
                    continue;
                // 出错
                else {
                    listen_socket->err_call();
                    return;
                }
            }
        }
    }

    static void recv_connect(connect_weak_ptr connect) {
        auto socket = connect.lock();
        if (!socket) throw std::runtime_error("连接生命周期过早结束");
        auto server = socket->server().lock();
        if (!server) throw std::runtime_error("服务端会话层生命周期过早结束");
        while (true) {
            char buffer[1024];
            std::fill(buffer, buffer + sizeof(buffer), 0);
            ssize_t n = ::recv(socket->file(), buffer, sizeof(buffer), 0);
            if (n > 0) {
                socket->inBuff().append(buffer, n);
            } else if (n == 0) {
                // 尽管对端已经开启了四次挥手, 连接处于半关闭状态
                // 但对方的读端不一定关闭
                // 闲着也是闲着, 把可能存在的数据都给他发过去
                socket->out_call();
                // 这个连接实际上已经失效, 不需要继续判断
                if (socket->is_closed()) continue;
                // 这个连接在会话层已经被注销, 不需要继续判断
                if (!socket->is_register()) continue;
                socket->rdh_call();
                return;
            } else {
                if (errno == EWOULDBLOCK) {
                    break;
                } else if (errno == EINTR) {
                    continue;
                } else {
                    socket->unregister_call();
                    return;
                }
            }
        }

        try {
            std::string temp;
            do {
                temp = server->_call_back(socket->inBuff());
                socket->outBuff() += temp;
            } while (!temp.empty() && !socket->inBuff().empty());
            // 新增了写关心
            if (!socket->outBuff().empty()) {
                server->_epoll->mod(socket, EpollerItem::outwritable | EpollerItem::inreadable |
                                                EpollerItem::edge_trigger);
            }
        } catch (std::exception& e) {
            BOOST_LOG_TRIVIAL(error)
                << std::format("会话层之上出现了无法处理的问题, 将注销该会话: {}", e.what());
            socket->unregister_call();
        }
    }

    static void send_connect(connect_weak_ptr connect) {
        auto socket = connect.lock();
        if (!socket) throw std::runtime_error("连接生命周期过早结束");
        auto server = socket->server().lock();
        if (!server) throw std::runtime_error("服务端会话层生命周期过早结束");

        while (!socket->outBuff().empty()) {
            ssize_t n =
                ::send(socket->file(), socket->outBuff().c_str(), socket->outBuff().size(), 0);
            if (n > 0)
                socket->outBuff().erase(0, n);
            else if (n == 0)
                // 发完了出去
                break;
            else {
                // 传输层缓冲区满了
                if (errno == EWOULDBLOCK)
                    break;
                else if (errno == EINTR)
                    continue;
                else {
                    socket->err_call();
                    return;
                }
            }
        }
        // 之前 add 或者 mod 关心过写事件, 则现在去除
        if (socket->need_write_interest() && socket->outBuff().empty()) {
            // 去除写关心
            server->_epoll->mod(socket, EpollerItem::inreadable | EpollerItem::edge_trigger);
        }
    }

    static void SetNonBlock(int fd) {
        int f = fcntl(fd, F_GETFL);
        if (f < 0) {
            throw std::system_error(
                errno, std::generic_category(),
                std::format("对于文件描述符({})的连接在设置为非阻塞读文件属性时失败: ", fd));
        }

        int r = fcntl(fd, F_SETFL, f | O_NONBLOCK);
        if (r < 0) {
            throw std::system_error(
                errno, std::generic_category(),
                std::format("对于文件描述符({})的连接在设置为非阻塞写文件属性时失败: ", fd));
        }
    }

   private:
    connects _connects;
    on_message_call_t _call_back;
    addr _listen_addr;
    port _listen_port;
    std::unique_ptr<Epoller> _epoll;
};
