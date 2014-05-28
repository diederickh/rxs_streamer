#ifndef RXS_STUN_IO_H
#define RXS_STUN_IO_H

#include <uv.h>
#include <rxs_streamer/rxs_stun.h>

#define RXS_STUN_IO_NUM_MEM_BLOCKS 16
#define RXS_STUN_IO_MEM_BLOCK_SIZE (16 * 1024)

#define RXS_SIO_STATE_NONE 0                          /* default state, will kickoff resolving when rxs_stun_io_init() is called. */
#define RXS_SIO_STATE_RESOLVED 1                      /* IP of stun server is resolved. */

typedef struct rxs_stun_io rxs_stun_io;
typedef struct rxs_stun_mem rxs_stun_mem;

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
};

int rxs_stun_io_init(rxs_stun_io* io, const char* server, const char* port);
void rxs_stun_io_update(rxs_stun_io* io);
int rxs_stun_io_clear(rxs_stun_io* io);

#endif
