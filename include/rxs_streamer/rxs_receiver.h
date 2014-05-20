/* 

   rxs_receiver
   ------------

   Used to receive UDP RTP packets that contain VP8. 

 */
#ifndef RXS_RECEIVER_H
#define RXS_RECEIVER_H

#include <uv.h>

typedef struct rxs_receiver rxs_receiver;
typedef void(*rxs_receiver_callback)(rxs_receiver* rec, uint8_t* buf, uint32_t nbytes);

struct rxs_receiver {

  /* networking */
  struct sockaddr_in saddr;
  uv_udp_t recv_sock;
  uv_loop_t* loop;

  /* callback */
  void* user;
  rxs_receiver_callback on_data;  /* will be called when we receive data */
};

int rxs_receiver_init(rxs_receiver* r, int port);
void rxs_receiver_update(rxs_receiver* r);

#endif
