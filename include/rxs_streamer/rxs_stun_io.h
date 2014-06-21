#ifndef RXS_STUN_IO_H
#define RXS_STUN_IO_H

#include <uv.h>
#include <rxs_streamer/rxs_stun.h>

#define RXS_STUN_IO_NUM_MEM_BLOCKS 16
#define RXS_STUN_IO_MEM_BLOCK_SIZE (64 * 1024)

#define RXS_SIO_STATE_NONE 0                          /* default state, will kickoff resolving when rxs_stun_io_init() is called. */
#define RXS_SIO_STATE_RESOLVED 1                      /* IP of stun server is resolved. */

typedef struct rxs_stun_io rxs_stun_io;
typedef struct rxs_stun_mem rxs_stun_mem;

typedef void(*rxs_stun_io_address_cb)(rxs_stun_io* io, struct sockaddr_in* addr); /* gets called when we got a reply from the stun server and we know our public IP:PORT */

struct rxs_stun_mem {
  char data[RXS_STUN_IO_MEM_BLOCK_SIZE];              /* used to store the data */
  int is_free;                                        /* is set to 1 when this memory block isn't used */
  uint32_t nbytes;                                    /* number of bytes stored, is set in on_alloc*/
  rxs_stun_io* io;                                    /* pointer to our rxs_stun_io */
};

struct rxs_stun_io {
  struct sockaddr_in saddr;                           /* address to which we send udp data */
  uv_udp_t sock;
  uv_getaddrinfo_t resolver;
  uv_loop_t* loop;
  rxs_stun stun;
  uint16_t port;
  char ip[17];                                         /* ip of stun server */
  rxs_stun_mem mem[RXS_STUN_IO_NUM_MEM_BLOCKS];        /* very basic memory mangement for now. */
  int state;                                           /* used to keep state; */
  uint64_t keepalive_timeout;                          /* every keepalive_delay nanos we send a binding indication. */
  uint64_t keepalive_delay;                            /* delay in ns. between each keep alive message */

  /* callback */
  void* user;
  rxs_stun_io_address_cb on_address;                   /* simply dispatches the on_attr call from rxs_stun. */
};

int rxs_stun_io_init(rxs_stun_io* io, const char* server, const char* port);
void rxs_stun_io_update(rxs_stun_io* io);
int rxs_stun_io_clear(rxs_stun_io* io);

#endif
