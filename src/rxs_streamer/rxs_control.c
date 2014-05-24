#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rxs_streamer/rxs_control.h>

/* ----------------------------------------------------------------------------- */

static void control_write_u16(uint8_t* buffer, uint16_t v);
static void control_write_command(uint8_t* buffer, uint8_t command, uint8_t count);
static uint16_t control_read_u16(uint8_t* buffer);
static int control_read_command(uint8_t* buffer, uint8_t* command, uint8_t* count);
static int control_sender_send(rxs_control_sender* send);
static void control_sender_send_cb(uv_udp_send_t* req, int status);

/* ----------------------------------------------------------------------------- */

int rxs_control_sender_init(rxs_control_sender* s, const char* ip, int port) {

  int r = 0;

  if (!s) { return -1; } 

  s->loop = uv_default_loop();
  if (!s->loop) {
    printf("Error: cannot get uv loop for control sender.\n");
    return -2;
  }

  r = uv_udp_init(s->loop, &s->sock);
  if (r != 0) {
    printf("Error: cannot initialize the udp sock for control sender: %s\n", uv_strerror(r));
    return -3;
  }

  r = uv_ip4_addr(ip, port, &s->saddr);
  if (r != 0) {
    printf("Error: cannot initialize the ip4 saddr for control sender: %s\n", uv_strerror(r));
    return -4;
  }

  s->nr = 0;
  s->dx = 0;

  return 0;
}

void rxs_control_sender_update(rxs_control_sender* send) {
  uv_run(send->loop, UV_RUN_NOWAIT);
}

int rxs_control_sender_request_keyframe(rxs_control_sender* send) {

  if (!send) { return -1; } 

  /* create command */
  send->dx = 0;

  control_write_u16(send->buffer, send->nr);
  send->dx += 2;

  control_write_command(send->buffer + send->dx, RXS_CONTROL_COMMAND_KEY_FRAME, 0);
  send->dx++;

  /* send */
  if (control_sender_send(send) < 0) {
    printf("Error: couldn't send control packet.\n");
    return -2;
  }
  
  /* increment our packet counter */
  send->nr++;
  return 0;
}

int rxs_control_sender_request_packets(rxs_control_sender* send, uint16_t* seqnums, int num) {

  int i = 0;

  if (!send) { return -1; } 
  if (!num) { return -2; } 

  if (num >= RXS_CONTROL_MAX_SEQNUM) {
    printf("Error: asked to request %d packets, but we can only request: %d.\n", num, RXS_CONTROL_MAX_SEQNUM);
    return -3;
  }

  send->dx = 0;

  /* store the number */
  control_write_u16(send->buffer, send->nr);
  send->dx += 2;

  /* command */
  control_write_command(send->buffer + send->dx, RXS_CONTROL_COMMAND_RESEND, num);
  send->dx++;

  for (i = 0; i < num; ++i) {
    control_write_u16(send->buffer + send->dx, seqnums[i]);
    send->dx += 2;
  }

  /* and send */
  if (control_sender_send(send) < 0) {
    printf("Error: something went wrong while trying to send the packet resend request.\n");
    return -3;
  }

  send->nr++;
  return 0;
}

/* ----------------------------------------------------------------------------- */

/* @todo - the control_sender_send() function allocate tiny bits of memory 
           every time it needs to send something; it will be a performance
           improvement to use e.g. a ringbuffer
*/
static int control_sender_send(rxs_control_sender* send) {

  int r;
  uint8_t* data;
  uv_buf_t buf;
  uv_udp_send_t* req;

  if (!send) { return -1; } 
  if (send->dx <= 0) { return -2; } 
  

  /* create buffer */
  data = (uint8_t*)malloc(send->dx);
  if (!data) {
    printf("Error: cannot allocate tmp buffer in controller; out of mem.\n");
    return -3;
  }

  memcpy(data, send->buffer, send->dx);
  buf = uv_buf_init(data, send->dx);
  req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
  if (!req) {
    printf("Error: cannot allocate a send request in the controller.\n");
    return -4;
  }

  req->data = (void*) data;

  /* send */
  r = uv_udp_send(req, 
                  &send->sock, 
                  &buf, 1, 
                  (const struct sockaddr*)&send->saddr, 
                  control_sender_send_cb);

  if (r != 0) {
    printf("Error: failed to send control packet: %s\n", uv_strerror(r));
    free(req);
    req = NULL;
    return -5;
  }

  return 0;
}


static void control_sender_send_cb(uv_udp_send_t* req, int status) {

  /* free buffer */
  uint8_t* data = (uint8_t*) req->data;
  free(data);
  data = NULL;

  /* free request */
  free(req);
  req = NULL;
}

/* ----------------------------------------------------------------------------- */

static void receiver_alloc_cb(uv_handle_t* handle, size_t nsize, uv_buf_t* buf);
static void receiver_recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags);

/* ----------------------------------------------------------------------------- */

int rxs_control_receiver_init(rxs_control_receiver* rec, int port) {

  int r = 0;

  if (!rec) { return -1; } 

  rec->loop = uv_default_loop();
  if (!rec->loop) {
    printf("Error: canont get the default uv loop for the control receiver.\n");
    return -2;
  }

  r = uv_ip4_addr("0.0.0.0", port, &rec->saddr);
  if (r != 0) {
    printf("Error: cannot init the ip4 of the control receiver: %s\n", uv_strerror(r));
    return -3;
  }

  r = uv_udp_init(rec->loop, &rec->sock);
  if (r != 0) {
    printf("Error: cannot init the sock for the control receiver: %s\n", uv_strerror(r));
    return -4;
  }

  r = uv_udp_bind(&rec->sock, (const struct sockaddr*)&rec->saddr, 0);
  if (r != 0) {
    printf("Error: cannot bind udp sock for control receiver: %s\n", uv_strerror(r));
    return -5;
  }

  rec->sock.data = (void*)rec;

  r = uv_udp_recv_start(&rec->sock, receiver_alloc_cb,  receiver_recv_cb);
  if (r != 0) {
    printf("Error: cannot start the control udp receiver: %s\n", uv_strerror(r));
    return -6;
  }

  return 0;
}

void rxs_control_receiver_update(rxs_control_receiver* rec) {
  uv_run(rec->loop, UV_RUN_NOWAIT);
}

/* ----------------------------------------------------------------------------- */

static void receiver_alloc_cb(uv_handle_t* handle, 
                              size_t nsize, 
                              uv_buf_t* buf) 
{
  static char slab[65536];

  if (nsize > sizeof(slab)) {
    printf("Error: requested control receiver size to large. @todo - this is just a quick implementation.\n");
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
  int i;
  int dx = 0;
  rxs_control_receiver* rec = (rxs_control_receiver*) handle->data;
  uint8_t* buffer = buf->base;

  if (nread == 0) {
    /* @todo - how to we handle this ? */
    return ;
  }

  if (nread < 3) {
    printf("Error: controller read invalid packet; should be at least 3 bytes.\n");
    return ;
  }

  rec->nr = control_read_u16(buffer);
  dx += 2;
  
  if (control_read_command(buffer + dx, &rec->command, &rec->count)) {
    printf("Error: cannot parse the controller command.\n");
    return;
  }
  dx++;

#if 0
  printf("Received: %u bytes, nr: %d, command: %d, count: %d\n", 
         nread,
         rec->nr,
         rec->command,
         rec->count
         
  );
#endif

  switch (rec->command) {
    case RXS_CONTROL_COMMAND_RESEND: {
      for (i = 0; i < rec->count; ++i) {
        rec->seqnums[i] = control_read_u16(buffer + dx);
        dx += 2;
      }
      break;
    }
    default: {
      break;
    }
  }

  /* call the set callback */
  if (rec->on_command) {
    rec->on_command(rec);
  }

}

/* ----------------------------------------------------------------------------- */

static void control_write_u16(uint8_t* buffer, uint16_t v) {
  uint8_t* p = (uint8_t*)&v;
  buffer[0] = p[1];
  buffer[1] = p[0];
}

static uint16_t control_read_u16(uint8_t* buffer) {
  uint16_t result = 0;
  uint8_t* dest = (uint8_t*)&result;
  dest[0] = buffer[1];
  dest[1] = buffer[0];
  return result;
}

static void control_write_command(uint8_t* buffer, uint8_t command, uint8_t count) {
  buffer[0]  = (command & 0x0F) << 4;
  buffer[0] |= (count   & 0x0F) << 0;
}

static int control_read_command(uint8_t* buffer, uint8_t* command, uint8_t* count) {
  if (!buffer) { return -1; } 
  if (!command) { return -2; } 
  if (!count) { return -3; } 

  *command = (buffer[0] & 0xF0) >> 4;
  *count   = (buffer[0] & 0x0F) >> 0;
  return 0;
}
