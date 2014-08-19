#ifndef STREAMER_RECEIVER_H
#define STREAMER_RECEIVER_H

#include <streamer/Loop.h>
#include <streamer/SocketUDP.h>
#include <pthread.h>

namespace rxs { 

  /* The receiver callback which is called whenever we read some data on the 
     socket. You need to make sure to call `resetBuffer()` whenever you have
     handled the incoming data. All incoming data is stored in the `sock.in_buffer` 
     and will grow until you're out of memory. Therefore it's important to call 
     `resetBuffer` when  you're done with the data. 
     
     The `data` and `nbytes` give you info about how many bytes we've stored
     in TOTAL in the `sock.in_buffer`. 

  */
  class Receiver;
  typedef void(*receiver_on_data)(Receiver* recv, uint8_t* data, uint32_t nbytes);

  class Receiver {

  public:
    Receiver(std::string lip, uint16_t lport, std::string rip, uint16_t rport);
    ~Receiver();
    int init();
    int shutdown();
    void resetBuffer();

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

  inline void Receiver::resetBuffer() {
    sock.in_dx = 0;
  }

} /* namespace rxs */

#endif
