#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rxs_streamer/rxs_sender.h>

/* ----------------------------------------------------------------------------- */

static void send_cb(uv_udp_send_t* req, int status);

/* ----------------------------------------------------------------------------- */

int rxs_sender_init(rxs_sender* net, const char* ip, int port) {
  int r;

  if (!net) { return -1; } 

  /* get default loop */
  net->loop = uv_default_loop();
  if (!net->loop) {
    printf("Error: cannot get default loop in sender.\n");
    return -2;
  }

  /* init sock */
  r = uv_udp_init(net->loop, &net->send_sock);
  if (r != 0) {
    printf("Error: cannot initialize uv_udp_t in sender.\n");
    return -3;
  }

  /* create ip4 addr */
  r = uv_ip4_addr(ip, port, &net->saddr);
  if (r != 0) {
    printf("Error: cannot create addr struct for sender: %s\n", uv_strerror(r));
    return -4;
  } 

  return 0;
}

int rxs_sender_send(rxs_sender* net, uint8_t* buffer, uint32_t nbytes) {
  
  char* tmp = NULL;
  int r;
  uv_buf_t b;
  uv_udp_send_t* req = NULL;

  if (!net) { return -1; } 
  if (!buffer) { return -2; } 
  if (!nbytes) { return -3; } 

  /* we should copy the buf here... */
  tmp = (char*)malloc(nbytes);
  memcpy(tmp, buffer, nbytes);

  /* @todo - the tests of libuv use this, but I'm pretty sure we want to allocate on the heap here */

  b = uv_buf_init((char*)tmp, nbytes);
  req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
  req->data = tmp;
  r = uv_udp_send(req, &net->send_sock, &b, 1, (const struct sockaddr*)&net->saddr, send_cb);

  if (r != 0) {
    printf("Error: cannot send udp: %s\n", uv_strerror(r));
    /* @todo -> shouldn't we free the allocated buffer here? */
    return -1;
  }
  return 0;
}

void rxs_sender_update(rxs_sender* net){
  uv_run(net->loop, UV_RUN_NOWAIT);
  //uv_run(net->loop, UV_RUN_DEFAULT);
}

/* ----------------------------------------------------------------------------- */

static void send_cb(uv_udp_send_t* req, int status) {

  char* data = (char*)req->data;
  free(data);
  data = NULL;

  free(req);
  req = NULL;
}
