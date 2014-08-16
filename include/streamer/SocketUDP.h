#ifndef STREAMER_SOCKET_UDP_H
#define STREAMER_SOCKET_UDP_H

#include <netdb.h>
#include <stdint.h>
#include <string>
#include <streamer/Socket.h>
#include <streamer/Loop.h>

class SocketUDP : public Socket {

 public:
  SocketUDP(Loop* loop);
  ~SocketUDP();
  int bind(std::string ip, uint16_t port);
  int getSocketDescriptor();
  int sendTo(std::string ip, uint16_t port, const uint8_t* data, uint32_t nbytes, loop_on_written onwritten, void* udata);

 public:
  struct sockaddr_in addr;
  int sock;
  std::string ip;
  uint16_t port;
};

inline int SocketUDP::getSocketDescriptor() {
  return sock;
}

#endif
