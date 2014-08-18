#ifndef STREAMER_RECEIVER_H
#define STREAMER_RECEIVER_H

#include <streamer/Loop.h>
#include <streamer/SocketUDP.h>

namespace rxs { 

  class Receiver {

  public:
    Receiver(std::string ip, uint16_t port);
    ~Receiver();
    int start();
    int stop();

  public:
    Loop loop;
    SocketUDP sock;
    std::string rip;          /* remote IP */
    uint16_t rport;           /* remote port */ 
  };

} /* namespace rxs */

#endif
