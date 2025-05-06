#pragma once

#include<fcntl.h>
#include"log.hpp"
#include<vector>
#include<memory>
#include<cstring>
#include<iostream>
#include"NonCopy.hpp"
#include"Epoller.hpp"
#include<functional>
#include"TcpSocket.hpp"
#include<unordered_map>


class TcpServer;
class Connection;

static const int MAXEVENTS = 64;
static const int default_timeout = 3000;

// 使用边缘触发模式
static const uint32_t EVENT_IN = EPOLLIN | EPOLLET;
static const uint32_t EVENT_OUT = EPOLLOUT | EPOLLET ;

// 非要有这个参数是为了TcpServer可以找到那个特定的连接
using func_t = std::function<void(std::weak_ptr<Connection>)>;

class Connection : public NonCopy
{
    public:
    Connection(int sock, std::weak_ptr<TcpServer> tcp_server_ptr) : _sock(sock) , _tcp_server_ptr(tcp_server_ptr){}

    void SetCallBack(func_t recv_cb, func_t send_cb, func_t except_cb)
    {
        _recv_cb = recv_cb;
        _send_cb = send_cb;
        _except_cb = except_cb;
    }

    int sock_fd() {return _sock;}

    private:
    int _sock;
    std::vector<char> _inbuffer;   // 缓冲区不应该用string, 那样读不了二进制文件
    std::vector<char> _outbuffer;
    std::weak_ptr<TcpServer> _tcp_server_ptr;

    public:
    // 专门给TcpServer检测有效性而暴露的接口
    func_t _recv_cb;
    func_t _send_cb;
    func_t _except_cb;
};

// 为了registerConnection能得到自己的shared_ptr而继承enable_shared_from_this
// 不写public默认私有, 会出问题, 所以一定要指明是public, 特别是enable_shared_from_this
// 私有化继承会导致shared_from_this抛出异常bad_weak_ptr
class TcpServer : public NonCopy , public std::enable_shared_from_this<TcpServer>
{
    private:
    // 构造函数私有化, 让TcpServer只能由shared_ptr形式创建
    TcpServer(uint16_t port)
        : _running(false)
        , _port(port)
        , _epoll_ptr(new Epoller())
        , _listen_sock_ptr(new TcpSocket())
    {
        _listen_sock_ptr->socket_create_();
        SetNonBlock(_listen_sock_ptr->socket_fd_());
        _listen_sock_ptr->socket_bind_(_port);
        _listen_sock_ptr->socket_reuse_port_address_();
        _listen_sock_ptr->socket_listen_();
    }

    public:
    static std::shared_ptr<TcpServer> create(uint16_t port)
    {
        // 不能用make_shared, 因为这个函数里面的new突破不了我的类域
        return std::shared_ptr<TcpServer>(new TcpServer(port));
    }

    // 智能指针控制生命周期
    ~TcpServer(){}

    void init(){}

    void registerConnection(int sock, uint32_t event, func_t recv_cb = nullptr, func_t send_cb = nullptr, func_t except_cb = nullptr)
    {
        // 注册一个连接分为两步
        // 一, 将连接交付给_connections统一调度
        // 获取自身的shared_ptr对象, 前提是必须继承enable_shared_from_this, 和TcpServer由shared_ptr形式创建, 直接使用shared_ptr<TcpServer>(this)会重复析构
        auto me = shared_from_this();
        std::shared_ptr<Connection> connection = std::make_shared<Connection>(sock, me);  // 相当于new Connection(sock), 注意weak_ptr只能用shared_ptr来初始化
        connection->SetCallBack(recv_cb, send_cb, except_cb);
        _connections.emplace(sock, connection);
        // 二, 将连接添加到epoll模型中
        _epoll_ptr->epoll_add_(sock, event);
    }

    void Accept(std::weak_ptr<Connection> connection)
    {
        // weak_ptr没有原始指针内函数的使用权, 需要先转化为shared_ptr
        std::shared_ptr<Connection> me = connection.lock();

        // 使劲读, 直到读出EWOULDBLOCK
        while(true)
        {
            struct sockaddr_in sock_in;
            socklen_t len = static_cast<socklen_t>(sizeof(sock_in));

            // 因为我们现在只有文件描述符, 所以只能用原始系统接口
            int sock = ::accept(me->sock_fd(), reinterpret_cast<struct sockaddr*>(&sock_in), &len);
            // 0也是有可能的, 特别是守护进程话之后
            if(sock >= 0)
            {
                // 成功获取一个连接
                SetNonBlock(sock);
                registerConnection(sock, EVENT_IN);
            }
            else
            {
                // 出错了
                if(errno == EWOULDBLOCK)
                {
                    // 彻底读完了
                    break;
                }
                else if(errno == EINTR)
                {
                    // 被信号打断了
                    // 继续读
                    continue;
                }
                else
                {
                    // 真的出错了

                }
            }
        }
    }

    // loop比start更合适,loop常用来表示不断循环的逻辑
    void loop(){
        _running = true;

        registerConnection(_listen_sock_ptr->socket_fd_(), EVENT_IN, std::bind(&TcpServer::Accept, this, std::placeholders::_1));


        while(_running)
        {
            dispatch(default_timeout);
            PrintConnection();
        }
    }

    // 检测文件是否被注册
    // 防止为_connections添加非法文件
    bool isRegister(int fd)
    {
        auto it = _connections.find(fd);
        return !(it == _connections.end());
    }

    // 事件派发器
    void dispatch(int timeout)
    {
        int n = _epoll_ptr->epoll_wait_(_readys, MAXEVENTS, timeout);
        // 暂不考虑差错处理
        for(int i = 0; i < n; ++i)
        {
            int sock = _readys[i].data.fd;
            uint32_t event = _readys[i].events;

            // 由于可能同时就绪多个事件, 所以应该用多个if, 而不是if else
            // 写回的event只会标记就绪的事件, 所以和我们的EVENT_IN并不相同,此时只能使用按位与

            // 将错误异常统一转换成读写问题进行处理, 防止问题扩散, 交由专门(读写)的组件进行处理
            if(event & EPOLLERR) event |= (EPOLLIN | EPOLLOUT);
            if(event & EPOLLHUP) event |= (EPOLLIN | EPOLLOUT);

            if((event & EPOLLIN) && isRegister(sock))
            {
                auto connection_ptr = _connections[sock];
                // 如果调用方法是有效的
                if(connection_ptr->_recv_cb) connection_ptr->_recv_cb(connection_ptr);
            }

            if((event & EPOLLOUT) && isRegister(sock))
            {
                auto connection_ptr = _connections[sock];
                // 如果调用方法是有效的
                if(connection_ptr->_send_cb) connection_ptr->_send_cb(connection_ptr);
            }
        }
    }

    void PrintConnection()
    {
        std::cout<<"sock fd list: ";
        for(const auto& connection : _connections)
        {
            std::cout<<connection.first<<" ";
        }
        std::cout << std::endl;
    }

    private:

    // 为防止阻塞设置失败而卡死进程, 失败时直接退出
    void SetNonBlock(int fd)
    {
        int f1 = fcntl(fd, F_GETFL);
        if(f1 < 0)
        {
            _log(Fatal, "文件%d阻塞状态获取失败:%s", strerror(errno));
            exit(1);
        }

        int r =  fcntl(fd, F_SETFL, f1 | O_NONBLOCK);
        if(r < 0)
        {
            _log(Fatal, "文件%d阻塞状态设置失败:%s", strerror(errno));
            exit(1);
        }
    }

    private:
    bool _running;
    uint16_t _port;
    struct epoll_event _readys[MAXEVENTS];
    std::shared_ptr<Epoller> _epoll_ptr;
    wind::Log &_log = wind::Log::getInstance();
    std::shared_ptr<TcpSocket> _listen_sock_ptr;
    std::unordered_map<int, std::shared_ptr<Connection>> _connections;
};