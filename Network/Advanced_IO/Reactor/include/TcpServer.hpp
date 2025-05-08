#pragma once

#include "Buffer.hpp"
#include "Epoller.hpp"
#include "NonCopy.hpp"
#include "TcpSocket.hpp"
#include "log.hpp"
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

class TcpServer;
class Connection;

static const int ip_buff_size = 20;
static const int io_buff_size = 1024;
static const int MAXEVENTS = 64;
static const int default_timeout = 1000;

// 使用边缘触发模式
static const uint32_t EVENT_IN = EPOLLIN | EPOLLET;
static const uint32_t EVENT_OUT = EPOLLOUT | EPOLLET;

// 非要有这个参数是为了TcpServer可以找到那个特定的连接
using func_t = std::function<void(std::weak_ptr<Connection>)>;

class Connection : public NonCopy {
public:
  Connection(int sock, std::weak_ptr<TcpServer> tcp_server_ptr, const char *ip,
             uint16_t port)
      : _sock(sock), _tcp_server_ptr(tcp_server_ptr), _ip(ip), _port(port) {}

  void SetCallBack(func_t recv_cb, func_t send_cb, func_t except_cb) {
    _recv_cb = recv_cb;
    _send_cb = send_cb;
    _except_cb = except_cb;
  }

  int sock_fd() { return _sock; }

private:
  int _sock;

public:
  std::weak_ptr<TcpServer> _tcp_server_ptr;

  std::string _ip;
  uint16_t _port;

  std::string
      _inbuffer; // 缓冲区不应该用string, 那样读不了二进制文件,
                 // 因为二进制文件里可能会有很多\0, 这样就会被视为终止符,
                 // 但为了方便起见, 这里依旧使用string
  std::string _outbuffer;

public:
  // 专门给TcpServer检测有效性而暴露的接口
  func_t _recv_cb;
  func_t _send_cb;
  func_t _except_cb;
};

// 为了registerConnection能得到自己的shared_ptr而继承enable_shared_from_this
// 不写public默认私有, 会出问题, 所以一定要指明是public,
// 特别是enable_shared_from_this
// 私有化继承会导致shared_from_this抛出异常bad_weak_ptr
class TcpServer : public NonCopy,
                  public std::enable_shared_from_this<TcpServer> {
private:
  // 构造函数私有化, 让TcpServer只能由shared_ptr形式创建
  TcpServer(uint16_t port, func_t OnMessage)
      : _running(false), _port(port), _epoll_ptr(new Epoller()),
        _OnMessage(OnMessage), _listen_sock_ptr(new TcpSocket()) {
    _listen_sock_ptr->socket_create_();
    SetNonBlock(_listen_sock_ptr->socket_fd_());
    _listen_sock_ptr->socket_bind_(_port);
    _listen_sock_ptr->socket_reuse_port_address_();
    _listen_sock_ptr->socket_listen_();
  }

public:
  static std::shared_ptr<TcpServer> create(uint16_t port, func_t OnMessage) {
    // 不能用make_shared, 因为这个函数里面的new突破不了我的类域
    return std::shared_ptr<TcpServer>(new TcpServer(port, OnMessage));
  }

  // 智能指针控制生命周期
  ~TcpServer() {}

  void init() {}

  void registerConnection(int sock, uint32_t event, const char *ip,
                          uint16_t port, func_t recv_cb = nullptr,
                          func_t send_cb = nullptr,
                          func_t except_cb = nullptr) {
    // 注册一个连接分为两步
    // 一, 将连接交付给_connections统一调度
    // 获取自身的shared_ptr对象, 前提是必须继承enable_shared_from_this,
    // 和TcpServer由shared_ptr形式创建,
    // 直接使用shared_ptr<TcpServer>(this)会重复析构
    auto me = shared_from_this();
    std::shared_ptr<Connection> connection = std::make_shared<Connection>(
        sock, me, ip, port); // 相当于new Connection(sock),
                             // 注意weak_ptr只能用shared_ptr来初始化
    connection->SetCallBack(recv_cb, send_cb, except_cb);
    _connections.emplace(sock, connection);
    // 二, 将连接添加到epoll模型中
    _epoll_ptr->epoll_add_(sock, event);

    _log(Debug, "已将该文件成功注册:%d", sock);
  }

  void Accept(std::weak_ptr<Connection> connection) {
    // weak_ptr没有原始指针内函数的使用权, 需要先转化为shared_ptr
    std::shared_ptr<Connection> me = connection.lock();

    // 使劲读, 直到读出EWOULDBLOCK
    while (true) {
      struct sockaddr_in sock_in;
      socklen_t len = static_cast<socklen_t>(sizeof(sock_in));

      // 因为我们现在只有文件描述符, 所以只能用原始系统接口
      int sock = ::accept(me->sock_fd(),
                          reinterpret_cast<struct sockaddr *>(&sock_in), &len);
      // 0也是有可能的, 特别是守护进程话之后
      if (sock >= 0) {
        // 成功获取一个连接
        uint16_t peerport = ntohs(
            sock_in.sin_port); // peer意为对端, 对方, 常用于去中心化,
                               // 分布式网络环境中, 不过在这里似乎不太合适,
                               // 因为我们是传统cs模式
        char peerip[ip_buff_size];
        inet_ntop(AF_INET, &sock_in.sin_addr, peerip, sizeof(peerip));

        _log(Debug, "收到一个新的连接[%s:%d], sockfd:%d", peerip, peerport,
             sock);

        SetNonBlock(sock);
        registerConnection(
            sock, EVENT_IN, peerip, peerport,
            std::bind(&TcpServer::Recver, this, std::placeholders::_1),
            std::bind(&TcpServer::Sender, this, std::placeholders::_1),
            std::bind(&TcpServer::Excepter, this, std::placeholders::_1));
      } else {
        // 出错了
        if (errno == EWOULDBLOCK) {
          // 彻底读完了
          break;
        } else if (errno == EINTR) {
          // 被信号打断了
          // 继续读
          continue;
        } else {
          // 真的出错了
        }
      }
    }
  }

  void Recver(std::weak_ptr<Connection> connection) {
    std::shared_ptr<Connection> me = connection.lock();

    // _log(Debug, "ha");

    // 服务器主框架只关心把全部数据读上来
    // 数据解析不是本层应该做的事

    int sock = me->sock_fd();
    while (true) {
      char buff[io_buff_size];
      memset(buff, 0, sizeof(buff));
      ssize_t n =
          recv(sock, buff, sizeof(buff), 0); // flags 0不进行任何标记位设置
      if (n > 0) {
        me->_inbuffer += buff;
        _log(Debug, "%s", me->_inbuffer.c_str());

      } else if (n == 0) {
        _log(Info, "用户(%d)<%s:%d>退出了连接...", sock, me->_ip.c_str(),
             me->_port);

        // 回调异常处理逻辑
        me->_except_cb(me);
        return;

      } else {
        // 出错了
        if (errno == EWOULDBLOCK) {
          // 彻底读完了
          break;
        } else if (errno == EINTR) {
          // 被信号打断了
          // 继续读
          continue;
        } else {
          // 真的出错了
          // 回调异常处理逻辑
          _log(Warning, "用户(%d)<%s:%d>连接出错", sock, me->_ip.c_str(),
               me->_port);
          me->_except_cb(me);
          return;
        }
      }
    }

    // 我只是服务器框架
    // 应用层交由上层解析
    _OnMessage(me);
  }

  void Sender(std::weak_ptr<Connection> connection) {
    std::shared_ptr<Connection> me = connection.lock();

    while (true) {
      ssize_t n =
          send(me->sock_fd(), me->_outbuffer.c_str(), me->_outbuffer.size(), 0);
      if (n > 0) {
        me->_outbuffer.erase(0, n);

        // 发完了, 出去
        if (me->_outbuffer.empty())
          break;
      } else if (n == 0) {
        return;
      } else {
        if (errno == EWOULDBLOCK) {
          break;
        } else if (errno == EINTR) {
          continue;
        } else {
          _log(Warning, "连接<%s:%d>发送失败: %s", me->_ip.c_str(), me->_port,
               strerror(errno));
          me->_except_cb(me);
          return;
        }
      }

      if (me->_outbuffer.empty()) {
        // 发完了, 为了避免写关心引发效率问题, 取消写关心
        AdjustEventMask(me->sock_fd(), true, false);
      } else {
        // 没发完, 如果传输层缓冲区又能写了, 那我就触发事件, 继续来写.
        AdjustEventMask(me->sock_fd(), true, true);
      }
    }
  }

  void AdjustEventMask(int sockfd, bool wantRead, bool wantWrite) {
    uint32_t event = 0;
    event |= ((wantRead ? EPOLLIN : 0) | (wantWrite ? EPOLLOUT : 0) | EPOLLET);
    _epoll_ptr->epoll_mod_(sockfd, event);
  }

  void Excepter(std::weak_ptr<Connection> connection) {
    std::shared_ptr<Connection> me = connection.lock();

    _log(Warning, "用户(%d)<%s:%d>连接关闭", me->sock_fd(), me->_ip.c_str(),
         me->_port);

    // 将连接从epoll中去除
    _epoll_ptr->epoll_del_(me->sock_fd());
    // AdjustEventMask(me->sock_fd(), false, false);

    // 将其从_connections中去除
    _connections.erase(me->sock_fd());

    // 关闭文件   或者将文件生命周期交由connection
    close(me->sock_fd());
  }

  // loop比start更合适,loop常用来表示不断循环的逻辑
  void loop() {
    _running = true;

    registerConnection(
        _listen_sock_ptr->socket_fd_(), EVENT_IN, "0.0.0.0", _port,
        std::bind(&TcpServer::Accept, this, std::placeholders::_1));

    // 调试: 为了避免listen_sock的Connection中的buff为空而导致异常,
    // 对其追加终止符, PrintConnection已经做了一定处理, 本代码舍弃
    // _connections[_listen_sock_ptr->socket_fd_()]->_inbuffer.append("", 1);
    // _connections[_listen_sock_ptr->socket_fd_()]->_outbuffer.append("", 1);

    while (_running) {

      dispatch(default_timeout);
      //   PrintConnection();
    }
  }

  // 检测文件是否被注册
  // 防止为_connections添加非法文件
  bool isRegister(int fd) {
    auto it = _connections.find(fd);
    return !(it == _connections.end());
  }

  // 事件派发器
  void dispatch(int timeout) {
    int n = _epoll_ptr->epoll_wait_(_readys, MAXEVENTS, timeout);
    if (n == 0)
      _log(Debug, "未等到新的连接");
    // 暂不考虑差错处理
    for (int i = 0; i < n; ++i) {

      int sock = _readys[i].data.fd;
      uint32_t event = _readys[i].events;

      // 由于可能同时就绪多个事件, 所以应该用多个if, 而不是if else
      // 写回的event只会标记就绪的事件,
      // 所以和我们的EVENT_IN并不相同,此时只能使用按位与

      // 将错误异常统一转换成读写问题进行处理, 防止问题扩散,
      // 交由专门(读写)的组件进行处理
      if (event & EPOLLERR)
        event |= (EPOLLIN | EPOLLOUT);
      if (event & EPOLLHUP)
        event |= (EPOLLIN | EPOLLOUT);

      if ((event & EPOLLIN) && isRegister(sock)) {
        auto connection_ptr = _connections[sock];
        // 如果调用方法是有效的
        if (connection_ptr->_recv_cb)
          connection_ptr->_recv_cb(connection_ptr);
      }

      if ((event & EPOLLOUT) && isRegister(sock)) {
        auto connection_ptr = _connections[sock];
        // 如果调用方法是有效的
        if (connection_ptr->_send_cb)
          connection_ptr->_send_cb(connection_ptr);
      }
    }
  }

  // void PrintConnection() {
  //   std::cout << "连接展示: \n";
  //   for (const auto &e : _connections) {
  //     // 缓冲区不是字符串, 大小为零连'\0'都没有
  //     if(e.second->_inbuffer.size() == 0) continue;
  //     printf("[fd: %d]<%s:%d>#", e.first, e.second->_ip.c_str(),
  //     e.second->_port); printf("%s\n", e.second->_inbuffer.data());
  //   }
  //   std::cout << std::endl<<std::endl;
  // }

  // 为兼容以往代码, 已将缓冲区改为string, 以下注释失效
  // 不安全, _inbuffer不一定是字符串化的
  void PrintConnection() {
    std::cout << "当前活跃连接：" << std::endl;
    for (const auto &e : _connections) {
      const auto &conn = e.second;
      // 缓冲区不是字符串, 大小为零连'\0'都没有
      if (conn->_inbuffer.empty())
        continue;

      std::cout << "----------------------------------------" << std::endl;
      std::cout << "fd        : " << e.first << std::endl;
      std::cout << "地址      : " << conn->_ip << ":" << conn->_port
                << std::endl;
      std::cout << "接收数据  : " << conn->_inbuffer.c_str() << std::endl;
    }
    std::cout << "========================================" << std::endl
              << std::endl;
  }

private:
  // 为防止阻塞设置失败而卡死进程, 失败时直接退出
  void SetNonBlock(int fd) {
    int f1 = fcntl(fd, F_GETFL);
    if (f1 < 0) {
      _log(Fatal, "文件%d阻塞状态获取失败:%s", strerror(errno));
      exit(1);
    }

    int r = fcntl(fd, F_SETFL, f1 | O_NONBLOCK);
    if (r < 0) {
      _log(Fatal, "文件%d阻塞状态设置失败:%s", strerror(errno));
      exit(1);
    }
  }

private:
  bool _running;
  uint16_t _port;
  func_t _OnMessage;
  struct epoll_event _readys[MAXEVENTS];
  std::shared_ptr<Epoller> _epoll_ptr;
  wind::Log &_log = wind::Log::getInstance();
  std::shared_ptr<TcpSocket> _listen_sock_ptr;
  std::unordered_map<int, std::shared_ptr<Connection>> _connections;
};