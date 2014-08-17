/*
  
  Loop
  -----

  The Loop is the main context that processes the events. All socket read/writes 
  are done asynchronously whenever the OS tells us when some data is available 
  on the socket or when the socket is ready to be written too. 


 */
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

#define READ_REQ_NONE 0x00
#define READ_REQ_UDP_RECVMSG 0x01

/* ------------------------------------------------------------- */

class WriteRequest  {

 public:
  WriteRequest();                                                  /* C'tor, inits all members */
  ~WriteRequest();                                                 /* D'tor, calls reset() and sets is_free = false */
  void reset();                                                    /* Reset all the member data + sets is_free to true so it can be reused. This must be called in e.g. you `socket_on_write` callback you pass ot the send*() functions. */
                                                                   
 public:                                                           
  const uint8_t* data;                                             /* The data we're going to write, you should manage the buffers you want to send yourself. We don't allocate or free any data. */
  uint32_t nbytes;                                                 /* The number of bytes to write. */
  bool is_free;                                                    /* We use Loop::getFreeWriteRequest(), which returns a free request when available, else it will create a new one. */
  struct sockaddr_in addr;                                         /* Used when the type is e.g. WRITE_REQ_UDP_SENDTO, so we know where we need to send data to. */ 
  Socket* socket;
  socket_on_write on_write;                                        /* Gets called when this write request has been executed. */
  void* user;                                                      /* Is set to the user data you pass to e.g. sendTo(). */
};

/* ------------------------------------------------------------- */
class ReadRequest {
 public:
  ReadRequest();
  ~ReadRequest();
  void reset();

 public:
  bool is_free;                                                     /* Whether this request is in use of not. */
  Socket* socket;                                                   /* The socket for which we want to handle reads */
  struct sockaddr_storage addr;                                     /* Used with UDP reads, is set to the addr form which we received data. */
  socket_on_read on_read;                                           /* Is called whenever we read something. */
  void* user;                                                       /* User specific data. */
};

/* ------------------------------------------------------------- */

class Loop {

 public:
  Loop();                                                          /* C'tor, inits everything; creats a queue handler.*/
  ~Loop();                                                         /* D'tor cleans up all requests */  
  int notifyRead(Socket* s);                                       /* Call the on_read function of this socket whenever we've read something */
  int notifyWrite(Socket* s);                                      /* Start listening for write-available events for this socket; when it becomes available we will write any outstanding data. */
  void update();                                                   /* Call this whenever you want to process the event queue */
  WriteRequest* getFreeWriteRequest();                             /* We reuse write requests so we don't need to allocate them all the time. */
  ReadRequest* getFreeReadRequest();

  /* The send() function is used internally to send data using the
     correct function. When we have an UDP socket we use sendto
     or with TCP just send. */
  ssize_t send(struct kevent& kev);

  /* Send data form `sender` to `addr. This is used for UDP sockets. 
     When the send is ready we call the socket_on_write callback function
     passing it the user data (`udata`). In the callback make sure 
     that you call reset() on the given WriteRequest* param.
  */
  int sendTo(Socket* sender, struct sockaddr_in addr, 
             const uint8_t* data, uint32_t nbytes, 
             socket_on_write onwritten, void* udata);

  /* Read data for the given struct kevent. The kevent.udata member is 
     set to the ReadRequest. Because an ReadRequest has a pointer to the
     socket we can figure out what kind of socket we have (UDP/TCP) and 
     therefore know how to read this socket. */
  ssize_t read(struct kevent& kev);

  /* recvmsg is used when a UDP socket reads data. We set the member 
     ReadRequest.addr to the address from which we read. */
  ssize_t recvmsg(ReadRequest* req);

 public:
  int kq;                                     /* our queue/event handle. */
  void* user;                                 /* can be set to any user data. */
  std::vector<struct kevent> read_list;       /* listen for read events */
  std::vector<struct kevent> write_list;      /* listen for write-ready events */
  std::vector<struct kevent> trigger_list;    /* passed into kevent(), gets the kevent's that triggered */
  std::vector<struct kevent> monitor_list;    /* merged version of the read and write list. writes are removed after they're used. */
  std::vector<WriteRequest*> write_requests;  /* our (reusable) collection of write requests. */
  std::vector<ReadRequest*> read_requests;    /* our (reusable) collection of read requests. */
};

#endif
