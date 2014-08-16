#ifndef STREAMER_LOOP_H
#define STREAMER_LOOP_H

#include <stdint.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <vector>
#include <map>
#include <string>
#include <streamer/Socket.h>

/* ------------------------------------------------------------- */

#define WRITE_REQ_NONE 0x00
#define WRITE_REQ_UDP_SENDTO 0x01

/* ------------------------------------------------------------- */

class WriteRequest;
typedef void(*loop_on_written)(WriteRequest* req);                 /* This callback is used when a write request is ready. */

class WriteRequest  {

 public:
  WriteRequest();                                                  /* C'tor, inits all members */
  ~WriteRequest();                                                 /* D'tor, calls reset() and sets is_free = false */
  void reset();                                                    /* Reset all the member data + sets is_free to true so it can be reused. This must be called in e.g. you `loop_on_written` callback you pass ot the send*() functions. */
                                                                   
 public:                                                           
  uint8_t type;                                                    /* The type of write request, TCP, UDP. E.g. WRITE_REQ_UDP_SENDTO */
  const uint8_t* data;                                             /* The data we're going to write, you should manage the buffers you want to send yourself. We don't allocate or free any data. */
  uint32_t nbytes;                                                 /* The number of bytes to write. */
  bool is_free;                                                    /* We use Loop::getFreeWriteRequest(), which returns a free request when available, else it will create a new one. */
  struct sockaddr_in addr;                                         /* Used when the type is e.g. WRITE_REQ_UDP_SENDTO, so we know where we need to send data to. */ 
  Socket* sender;                                                  /* Points to the socket that initiated the write. */
  loop_on_written on_written;                                      /* Gets called when this write request has been executed. */
  void* user;                                                      /* Is set to the user data you pass to e.g. sendTo(). */
  
};

/* ------------------------------------------------------------- */

class Loop {

 public:
  Loop();                                                          /* C'tor, inits everything; creats a queue handler.*/
  ~Loop();
  int notifyRead(Socket* s);
  int notifyWrite(Socket* s);
  void update();
  WriteRequest* getFreeWriteRequest();
  int sendTo(Socket* sender, struct sockaddr_in addr, const uint8_t* data, uint32_t nbytes, loop_on_written onwritten, void* udata);

 public:
  int kq;
  void* user;
  std::vector<struct kevent> read_list;       /* event list we want to monitor */
  std::vector<struct kevent> write_list;      /* event list we want to monitor */
  std::vector<struct kevent> trigger_list;    /* event list we want to monitor */
  std::vector<struct kevent> monitor_list;    /* event list we want to monitor */
  std::vector<WriteRequest*> write_requests;
};

#endif
