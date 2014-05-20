#include <stdio.h>
#include <stdlib.h>
#include <rxs_streamer/rxs_receiver.h>

/* ----------------------------------------------------------------------------- */

static void receiver_alloc_cb(uv_handle_t* handle, size_t nsize, uv_buf_t* buf);
static void receiver_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags);

/* ----------------------------------------------------------------------------- */

int rxs_receiver_init(rxs_receiver* rec, int port) {

  int r = 0;

  if (!rec) { return -1; } 

  /* get the loop handler. */
  rec->loop = uv_default_loop();
  if (!rec->loop) {
    printf("Error: cannot get the default uv loop.\n");
    return -2;
  }

  /* create sockaddr */
  //r = uv_ip4_addr("0.0.0.0", port, &rec->saddr);
  r = uv_ip4_addr("0.0.0.0", port, &rec->saddr);
  if (r != 0) {
    printf("Error: cannot init the sock struct.\n");
    return -3;
  }

  /* init receiver socket. */
  r = uv_udp_init(rec->loop, &rec->recv_sock);
  if (r != 0) {
    printf("Error: cannot initialize the receive sock handle. %s\n", uv_strerror(r));
    return -4;
  }

  /* set the user data */
  rec->recv_sock.data = (void*)rec;

  /* let our sock handle 'listen' on the given port */
  r = uv_udp_bind(&rec->recv_sock, (const struct sockaddr*)&rec->saddr, 0);
  if (r != 0) {
    printf("Error: cannot bind receiver socket %s\n", uv_strerror(r));
    return -5;
  }

  /* start receiving. */
  r = uv_udp_recv_start(&rec->recv_sock, receiver_alloc_cb, receiver_recv_cb);
  if (r != 0) {
    printf("Error: cannot start the udp receiver: %s\n", uv_strerror(r));
    return -6;
  }
  
  return 0;
}

void rxs_receiver_update(rxs_receiver* r) {
  uv_run(r->loop, UV_RUN_NOWAIT);
}


/* ----------------------------------------------------------------------------- */

static void receiver_alloc_cb(uv_handle_t* handle, 
                              size_t nsize, 
                              uv_buf_t* buf) 
{
  static char slab[65536];

  if (nsize > sizeof(slab)) {
    printf("Error: requested receiver size to large. @todo - this is just a quick implementation.\n");
    exit(1);
  }
  
  buf->base = slab;
  buf->len = sizeof(slab);
}

static void receiver_recv_cb(uv_udp_t* handle, 
                             ssize_t nread, 
                             const uv_buf_t* buf, 
                             const struct sockaddr* addr, 
                             unsigned int flags)
{
  rxs_receiver* rec = (rxs_receiver*) handle->data;

  if (nread == 0) {
    /* @todo - how to we handle this ? */
    return ;
  }

  rec->on_data(rec, (uint8_t*)buf->base, nread);
}
