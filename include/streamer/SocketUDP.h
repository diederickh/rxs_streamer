#ifndef STREAMER_SOCKET_UDP_H
#define STREAMER_SOCKET_UDP_H

#include <netdb.h>
#include <stdint.h>
#include <string>
#include <streamer/Socket.h>
#include <streamer/Loop.h>

namespace rxs {

  class SocketUDP : public Socket {

  public:
    SocketUDP(Loop* loop, size_t buffersize = 8192);
    ~SocketUDP();
    int bind(std::string ip, uint16_t port);
    int getSocketDescriptor();
    int sendTo(std::string ip, uint16_t port, const uint8_t* data, uint32_t nbytes, socket_on_write onwritten, void* udata);

  public:
    struct sockaddr_in addr;
    int sock;
    std::string ip;
    uint16_t port;
  };

  inline int SocketUDP::getSocketDescriptor() {
    return sock;
  }

} /* namespace rxs */

#endif
