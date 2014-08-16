#ifndef STREAMER_SOCKET_H
#define STREAMER_SOCKET_H

#define SOCKET_BUFFER_SIZE 4096
#include <stdint.h>

class Loop;
class Socket;
typedef void(*socket_on_read)(Socket* s);

class Socket {
 public:
  Socket(Loop* loop);
  virtual ~Socket();
  virtual int getSocketDescriptor() = 0;

 public:
  Loop* loop;              /* the loop handler, necessary for handling i/o events. */
  //  uint8_t* out_buffer;     /* when you want to write data this is where it's stored. */
  //  uint32_t out_dx;         /* the number of bytes in the output buffer. */
  uint8_t* in_buffer;      /* when there is data aviable on the socket we read into this buffer, wdx will be set to the end index of the write. */
  uint32_t in_dx;          /* write index into in_buffer. */
  socket_on_read on_read;  /* will be called whenever we've read some data */
  void* user;              /* user data */
};

#endif
