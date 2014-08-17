#ifndef STREAMER_SOCKET_H
#define STREAMER_SOCKET_H

#include <stdint.h>

#define SOCKET_TYPE_NONE 0x00
#define SOCKET_TYPE_UDP 0x01
#define SOCKET_TYPE_TCP 0x02

class Loop;
class Socket;
class ReadRequest;
class WriteRequest;
typedef void(*socket_on_read)(ReadRequest* req);
typedef void(*socket_on_write)(WriteRequest* req);

class Socket {
 public:
  Socket(Loop* loop, size_t buffersize = 8192);            /* Create a new socket with with a buffer of size `buffersize` into which we will write all incoming bytes; it will automatically grow if necessary but you need to make sure to reset the in_dx whenever you've read something, so make sure to set the on_read callback! */
  virtual ~Socket();                                       /* Cleans up all allocs. */
  virtual int getSocketDescriptor() = 0;                   /* Get the socket descriptor that is used for e.g. kqueue */
  bool growInputBuffer();                                  /* This will expand the input buffer to twice it's current size; */

 public:
  uint8_t type;
  Loop* loop;                                              /* the loop handler, necessary for handling i/o events. */
  uint8_t* in_buffer;                                      /* when there is data aviable on the socket we read into this buffer, wdx will be set to the end index of the write. */
  uint32_t in_dx;                                          /* write index into in_buffer. */
  uint32_t in_capacity;                                    /* the number of bytes that can be written into in_buffer. */
  socket_on_read on_read;                                  /* will be called whenever we've read some data */
  void* user;                                              /* user data */
};

#endif
