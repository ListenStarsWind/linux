#include "Epoller.hpp"
#include "NonCopy.hpp"
#include "TcpSocket.hpp"
#include "log.hpp"
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

static const uint32_t EVENT_IN = EPOLLIN | EPOLLET;
static const uint32_t EVENT_OUT = EPOLLOUT | EPOLLET;

static const int MAXEVENTS = 64;
static const int TIMEOUT = 1000;
static const int IPBUFFSIZE = 64;
static const int IOBUFFSIZE = 64;

class Connection;
class LoopServer;

using func_t = std::function<void(std::weak_ptr<Connection>)>;

struct Connection {

  Connection(int sock, std::weak_ptr<LoopServer> server_ptr)
      : _sock(sock), _server_ptr(server_ptr) {}

  int _sock;
  std::string _inbuff;
  std::string _outbuff;

  func_t _recv_cb;
  func_t _send_cb;
  func_t _except_cb;

  uint16_t _port;
  std::string _ip;

  std::weak_ptr<LoopServer> _server_ptr;
};

class LoopServer : public std::enable_shared_from_this<LoopServer>,
                   public NonCopy {

  LoopServer(uint16_t port, func_t OnMessage)
      : _running(false), _port(port), _epoll_ptr(new Epoller()),
        _listen_ptr(new TcpSocket()), _OnMessage(OnMessage) {}

public:
  static std::shared_ptr<LoopServer> create(uint16_t port, func_t OnMessage) {
    return std::shared_ptr<LoopServer>(new LoopServer(port, OnMessage));
  }

  void init() {
    _listen_ptr->socket_create_();
    SetNonBlock(_listen_ptr->socket_fd_());
    _listen_ptr->socket_reuse_port_address_();
    _listen_ptr->socket_bind_(_port);
    _listen_ptr->socket_listen_();

    registerConnection(
        _listen_ptr->socket_fd_(), EVENT_IN, "0.0.0.0", _port,
        std::bind(&LoopServer::Accept, this, std::placeholders::_1));
  }

  void registerConnection(int sock, uint32_t event, const char *ip,
                          uint16_t port, func_t recv_cb = nullptr,
                          func_t send_cb = nullptr,
                          func_t except_cb = nullptr) {
    // 注册一个连接分为两步

    // 1. 将连接交付给_connections统一调度
    // 获取自身(LoopServer)的shared_ptr
    auto me = shared_from_this();
    std::shared_ptr<Connection> connection =
        std::make_shared<Connection>(sock, me);
    connection->_ip = ip;
    connection->_port = port;
    connection->_recv_cb = recv_cb;
    connection->_send_cb = send_cb;
    connection->_except_cb = except_cb;
    _connections.emplace(sock, connection);

    // 2. 将连接添加到epoll对象中
    _epoll_ptr->epoll_add_(sock, event);

    _log(Debug, "成功注册文件: %d", sock);
  }

  void Accept(std::weak_ptr<Connection> listen) {
    auto me = listen.lock();
    while (true) {
      struct sockaddr_in peer;
      socklen_t len = static_cast<socklen_t>(sizeof(peer));
      int sock =
          accept(me->_sock, reinterpret_cast<struct sockaddr *>(&peer), &len);

      // 尽管不太可能, 但我们还是加上等于号
      if (sock >= 0) {
        uint16_t port = ntohs(peer.sin_port);
        char ip[IPBUFFSIZE];
        inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));

        _log(Info, "一个新的连接, 描述符:%d, 地址:%s:%d", sock, ip, port);

        SetNonBlock(sock);
        registerConnection(
            sock, EVENT_IN, ip, port,
            std::bind(&LoopServer::Recver, this, std::placeholders::_1),
            std::bind(&LoopServer::Sender, this, std::placeholders::_1),
            std::bind(&LoopServer::Excepter, this, std::placeholders::_1));
      } else {
        // 读完了
        if (errno == EWOULDBLOCK)
          return;
        // 被信号打断
        else if (errno == EINTR)
          continue;
        // 错误
        else {
          _log(Warning, "::accept 错误: %s", strerror(errno));
          return;
        }
      }
    }
  }

  void Recver(std::weak_ptr<Connection> connection) {
    auto me = connection.lock();
    int sock = me->_sock;
    while (true) {
      char buff[IOBUFFSIZE];
      std::fill(buff, buff + IOBUFFSIZE, 0);
      ssize_t n = recv(sock, buff, sizeof(buff), 0);
      if (n > 0) {
        me->_inbuff += buff;
      } else if (n == 0) {
        _log(Info, "用户[%s:%d]退出了连接, 来自fd(%d)的消息...",
             me->_ip.c_str(), me->_port, sock);
        me->_except_cb(me);
        return;
      } else {
        if (errno == EWOULDBLOCK)
          break;
        else if (errno == EINTR)
          continue;
        else {
          _log(Info, "连接[%s:%d]出错, 来自fd(%d)的消息", me->_ip.c_str(),
               me->_port, sock);
          me->_except_cb(me);
          return;
        }
      }
    }

    _OnMessage(me);
  }

  void Sender(std::weak_ptr<Connection> connection) {
    auto me = connection.lock();

    while (true) {
      ssize_t n = send(me->_sock, me->_outbuff.c_str(), me->_outbuff.size(), 0);
      if (n > 0)
        me->_outbuff.erase(0, n);
      else if (n == 0)
        break; // 发完了出去
      else {
        if (errno == EWOULDBLOCK)
          break;
        else if (errno == EINTR)
          continue;
        else {
          _log(Warning, "连接[$s:%d]出错, 来自fd(%d)的消息:%s", me->_ip.c_str(),
               me->_port, me->_sock, strerror(errno));
          me->_except_cb(me);
          return;
        }
      }
    }

    if (me->_outbuff.empty()) {
      // 取消写关心
      AdjustEventMask(me->_sock, true, false);
    } else {
      // 维持写关心
      AdjustEventMask(me->_sock, true, true);
    }
  }

  void AdjustEventMask(int sockfd, bool wantRead, bool wantWrite) {
    uint32_t event = 0;
    event |= ((wantRead ? EPOLLIN : 0) | (wantWrite ? EPOLLOUT : 0) | EPOLLET);
    _epoll_ptr->epoll_mod_(sockfd, event);
  }

  void Excepter(std::weak_ptr<Connection> connection) {
    auto me = connection.lock();
    _log(Info, "关闭连接<%d>[%s:%d]", me->_sock, me->_ip.c_str(), me->_port);

    // 1. 把连接从epoll里删除
    _epoll_ptr->epoll_del_(me->_sock);

    // 2. 把连接从_connections里取出
    _connections.erase(me->_sock);

    // 3. 告诉传输层关闭连接  或者可以让Connection析构时自动关闭
    close(me->_sock);
  }

  void loop() {
    _running = true;

    while (_running) {
      dispatch(TIMEOUT);
      RunIdleTasks();
    }
  }

  void dispatch(int timeout) {
    int n = _epoll_ptr->epoll_wait_(_reads, MAXEVENTS, timeout);
    if (n == 0)
      _log(Debug, "未收到任何连接");

    // 为了方便, 就不进行差错处理了
    for (int i = 0; i < n; ++i) {
      int sock = _reads[i].data.fd;
      uint32_t event = _reads[i].events;

      if (event & EPOLLERR) {
        event |= (EPOLLIN | EPOLLOUT);
      }
      if (event & EPOLLHUP)
        event |= (EPOLLIN | EPOLLOUT);

      if ((event & EPOLLIN) && isRegister(sock)) {
        auto connect_ptr = _connections[sock];
        // 如果调用方法是存在的
        if (connect_ptr->_recv_cb)
          connect_ptr->_recv_cb(connect_ptr);
      }

      if ((event & EPOLLOUT) && isRegister(sock)) {
        auto connect_ptr = _connections[sock];
        if (connect_ptr->_send_cb)
          connect_ptr->_send_cb(connect_ptr);
      }
    }
  }

  bool isRegister(int fd) {
    auto it = _connections.find(fd);
    return it != _connections.end();
  }

  void RunIdleTasks() {
    // PrintConnection();
  }

  void PrintConnection() {
    std::cout << "当前活跃连接：" << std::endl;
    for (const auto &e : _connections) {
      const auto &conn = e.second;
      if (conn->_inbuff.empty())
        continue;

      std::cout << "----------------------------------------" << std::endl;
      std::cout << "fd        : " << e.first << std::endl;
      std::cout << "地址      : " << conn->_ip << ":" << conn->_port
                << std::endl;
      std::cout << "接收数据  : " << conn->_inbuff << std::endl;
    }
    std::cout << "========================================" << std::endl
              << std::endl;
  }

  void SetNonBlock(int fd) {
    int f = fcntl(fd, F_GETFL);
    if (f < 0) {
      _log(Fatal, "文件(%d)阻塞状态获取失败:%s", strerror(errno));
      exit(1);
    }

    int r = fcntl(fd, F_SETFL, f | O_NONBLOCK);
    if (r < 0) {
      _log(Fatal, "文件(%d)阻塞状态设置失败:%s", strerror(errno));
      exit(1);
    }
  }

private:
  bool _running;
  uint16_t _port;
  struct epoll_event _reads[MAXEVENTS];
  std::shared_ptr<Epoller> _epoll_ptr;
  wind::Log &_log = wind::Log::getInstance();
  std::shared_ptr<TcpSocket> _listen_ptr;
  std::unordered_map<int, std::shared_ptr<Connection>> _connections;

  func_t _OnMessage;
};