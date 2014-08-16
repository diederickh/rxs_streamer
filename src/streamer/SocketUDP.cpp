#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <streamer/SocketUDP.h>

SocketUDP::SocketUDP(Loop* loop) 
  :sock(-1)
  ,Socket(loop)
  ,port(0)
{
}

SocketUDP::~SocketUDP() {

  if (-1 != sock) {
    ::close(sock);
  }

  sock = -1;
}

int SocketUDP::bind(std::string host, uint16_t p) {
  int r;

  if (-1 != sock) { return -1; } 
  if (0 == host.size()) { return -2; } 

  ip = host;
  port = p;

  /* create sock */
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (-1 == sock) {
    printf("Error: cannot create udp socket.\n");
    return -3;
  }

  /* setup our address hints */
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(p);
  
  /* bind */
  r = ::bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr));
  if (0 != r) {
    printf("Error: cannot bind the udp socket: %d.\n", r);
    return -4;
  }

  /* make sure we get events when there is data available */
  if (0 != loop->notifyRead(this)) {
    printf("Error: cannot listen to read events.\n");
    return -5;
  }

  return 0;
}

int SocketUDP::sendTo(std::string tip, uint16_t tport, const uint8_t* data, uint32_t nbytes, loop_on_written onwritten, void* udata) {
  if (-1 == sock) { return -1; } 
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(tip.c_str());
  addr.sin_port = htons(tport);
  return loop->sendTo(this, addr, data, nbytes, onwritten, udata);
}
