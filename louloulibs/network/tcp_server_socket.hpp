#pragma once

#include <network/socket_handler.hpp>
#include <network/resolver.hpp>
#include <network/poller.hpp>

#include <string>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <logger/logger.hpp>

#include <cstring>
#include <cassert>

using namespace std::string_literals;

template <typename RemoteSocketType>
class TcpSocketServer: public SocketHandler
{
 public:
  TcpSocketServer(std::shared_ptr<Poller> poller, const uint16_t port):
      SocketHandler(poller, -1)
  {
    if ((this->socket = ::socket(AF_INET6, SOCK_STREAM, 0)) == -1)
      throw std::runtime_error("Could not create socket: "s + std::strerror(errno));

    int opt = 1;
    if (::setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
      throw std::runtime_error("Failed to set socket option: "s + std::strerror(errno));

    struct sockaddr_in6 addr{};
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = IN6ADDR_ANY_INIT;
    if ((::bind(this->socket, (const struct sockaddr*)&addr, sizeof(addr))) == -1)
      { // If we can’t listen on this port, we just give up, but this is not fatal.
        log_warning("Failed to bind on port ", std::to_string(port), ": ", std::strerror(errno));
        return;
      }

    if ((::listen(this->socket, 10)) == -1)
      throw std::runtime_error("listen() failed");

    this->accept();
  }
  ~TcpSocketServer() = default;

  void on_recv() override
  {
    // Accept a RemoteSocketType
    int socket = ::accept(this->socket, nullptr, nullptr);

    auto client = std::make_unique<RemoteSocketType>(poller, socket, *this);
    this->poller->add_socket_handler(client.get());
    this->sockets.push_back(std::move(client));
  }

 protected:
  std::vector<std::unique_ptr<RemoteSocketType>> sockets;

 private:
  void accept()
  {
    this->poller->add_socket_handler(this);
  }
  void on_send() override
  {
    assert(false);
  }
  void connect() override
  {
    assert(false);
  }
  bool is_connected() const override
  {
    return true;
  }
  bool is_connecting() const override
  {
    return false;
  }
};
