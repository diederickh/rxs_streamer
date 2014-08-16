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
typedef void(*loop_on_written)(WriteRequest* req);

class WriteRequest  {

 public:
  WriteRequest();
  ~WriteRequest();
  void reset();

 public:
  uint8_t type;         /* e.g. WRITE_REQ_UDP_SENDTO */
  const uint8_t* data;
  uint32_t nbytes;
  bool is_free;
  struct sockaddr_in addr;
  Socket* sender;
  loop_on_written on_written;
  void* user;
  
};

/* ------------------------------------------------------------- */

class Loop {

 public:
  Loop();
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
