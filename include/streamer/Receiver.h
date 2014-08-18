#ifndef STREAMER_RECEIVER_H
#define STREAMER_RECEIVER_H

#include <streamer/Loop.h>
#include <streamer/SocketUDP.h>
#include <pthread.h>

namespace rxs { 

  class Receiver;
  typedef void(*receiver_on_data)(Receiver* recv, uint8_t* data, uint32_t nbytes);

  class Receiver {

  public:
    Receiver(std::string lip, uint16_t lport, std::string rip, uint16_t rport);
    ~Receiver();
    int init();
    int shutdown();

  public:
    Loop loop;
    SocketUDP sock;
    std::string lip;          /* local IP */
    std::string rip;          /* remote IP */
    uint16_t lport;           /* local port */ 
    uint16_t rport;           /* remote port */
    bool must_stop;
    bool is_running;
    pthread_t thread;
    
    /* callback */
    receiver_on_data on_data;
    void* user;
  };

} /* namespace rxs */

#endif
