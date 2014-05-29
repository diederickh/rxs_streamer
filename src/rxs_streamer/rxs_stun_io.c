#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <rxs_streamer/rxs_stun_io.h>

/* --------------------------------------------------------------------------- */
static void on_send_ready(uv_udp_send_t* req, int status);                                                              /* gets called by uv when data has been sent */      
static void on_stun_send(rxs_stun* stun, uint8_t* data, uint32_t nbytes);                                               /* gets called by rxs_stun when we need to send some data */       
static void on_resolved(uv_getaddrinfo_t* resolver, int status, struct addrinfo* addr);                                 /* gets called when the stun server domain has been resolved. */
static void on_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags); /* gets called whenever there is something on the socket that we need to process/read. */
static void on_alloc(uv_handle_t* handle, size_t nbytes, uv_buf_t* buf);                                                /* alloc data, called by uv */
static rxs_stun_mem* find_free_mem_block(rxs_stun_io* io);                                                              /* finds a memory block in io that we can use to store some data on the receiving side */
static rxs_stun_mem* find_mem_block(rxs_stun_io* io, char* dataptr);                                                    /* finds the memory block for which the data member has the given dataptr address */
static int stun_send(rxs_stun_io* io, uint8_t* data, uint32_t nbytes);

/* --------------------------------------------------------------------------- */

int rxs_stun_io_init(rxs_stun_io* io, const char* server, const char* port) {

  int r;
  int i;
  struct addrinfo hints;

  if (!io) { return -1; } 

  if (rxs_stun_init(&io->stun) < 0) {
    printf("Error: cannot init stun.\n");
    return -2;
  }

  io->loop = uv_default_loop();
  if (!io->loop) {
    printf("Error: cannot get uv loop.\n");
    return -3;
  }

  /* init the memory blocks */
  for(i = 0; i < RXS_STUN_IO_NUM_MEM_BLOCKS; ++i) {
    io->mem[i].is_free = 1;
    io->mem[i].io = io;
  }

  /* hints for resolver */
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  hints.ai_flags = 0;

  /* set user data */
  io->resolver.data = (void*)io;
  io->port = atoi(port);
  io->state = RXS_SIO_STATE_NONE;
  io->sock.data = io;
  io->stun.user = (void*)io;
  io->stun.on_send = on_stun_send;

  r = uv_getaddrinfo(io->loop, &io->resolver, 
                     on_resolved, server, port, &hints);
  
  if (r != 0){
    printf("Error: cannot start resolving stun server. %s\n", uv_strerror(r));
    return -4;
  }

  return 0;
}

void rxs_stun_io_update(rxs_stun_io* io) {

#if !defined(NDEBUG)
  if (!io) { 
    printf("Error: rxs_stun_io, cannot update; invalid io.\n");
    exit(1);
  }
#endif

  uv_run(io->loop, UV_RUN_NOWAIT);

}

int rxs_stun_io_clear(rxs_stun_io* io) {
  if (!io) { return -1; }

  if (rxs_stun_clear(&io->stun) < 0) {
    printf("Error: cannot clear rxs_stun in rxs_stun_io.\n");
    return -2;
  }

  return 0;
}

/* --------------------------------------------------------------------------- */

/* 
   Finds a free memory block in rxs_stun_io. We're using a very 
   basic memory system for now. We just have a big array with block
   of some kb. 
 */

static rxs_stun_mem* find_free_mem_block(rxs_stun_io* io) {

  int i;

  if (!io) { return NULL; } 
  
  for (i = 0; i < RXS_STUN_IO_NUM_MEM_BLOCKS; ++i) {
    if (io->mem[i].is_free == 1) {
      return &io->mem[i];
    }
    else {
      printf(">> %d\n", io->mem[i].is_free);
    }
  }

  return NULL;
}

static rxs_stun_mem* find_mem_block(rxs_stun_io* io, char* dataptr) {

  int i = 0;

  if (!io) { return NULL; } 
  for (i = 0; i < RXS_STUN_IO_NUM_MEM_BLOCKS; ++i) {
    if (io->mem[i].data == dataptr) {
      return &io->mem[i];
    }
  }

  return NULL;
}

/* --------------------------------------------------------------------------- */

static int stun_send(rxs_stun_io* io, uint8_t* data, uint32_t nbytes) {

  rxs_stun_mem* mem; 
  uv_udp_send_t* req;
  uv_buf_t buf;
  int r;

  if (!io) { return -1; }

  if (nbytes > RXS_STUN_IO_MEM_BLOCK_SIZE) {
    printf("Error: trying to send data in rxs_stun_io, but the buffer size is to small.\n");
    return -2;
  }

  req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
  if(!req) {
    return -3;
  }

  mem = find_free_mem_block(io);
  if (!mem) {
    printf("Error: cannot find a free memory block in rxs_stun_io. We should create more.\n");
    return -5;
  }

  req->data = (void*)mem;

  memcpy(mem->data, data, nbytes);

  buf.base = mem->data;
  buf.len = nbytes;

  r = uv_udp_send(req, &io->sock, &buf, 1, (const struct sockaddr*)&io->saddr, on_send_ready);
  if (r != 0) {
    printf("Error: error while trying to send some stun data: %s\n", uv_strerror(r));
    return -6;
  }
  
  return 0;
}

/* --------------------------------------------------------------------------- */

static void on_resolved(uv_getaddrinfo_t* resolver, int status, struct addrinfo* addr) {
  
  int r = 0;
  rxs_stun_io* io;

  if(status < 0) {
    printf("Error: Something went wrong when resolving the host in rxs_stun_io.\n");
    exit(1);
  }

  io = (rxs_stun_io*) resolver->data;

  /* get IP */
  uv_ip4_name((struct sockaddr_in*)addr->ai_addr, io->ip, 16);
  io->ip[16] = '\0';
    
  /* create our send sockaddr */
  r = uv_ip4_addr(io->ip, io->port, &io->saddr);
  if (r != 0) {
    printf("Error: cannot create ip4 addr struct. %s\n", uv_strerror(r));
    exit(1);
  }
  printf("Resolved host: %s\n", io->ip);

  /* init UDP sock */
  r = uv_udp_init(io->loop, &io->sock);
  if(r != 0) {
    printf("Error: cannot create sock for stun io: %s\n", uv_strerror(r));
    exit(1);
  }

  /* create our receiver sockaddr; this will receive data */
  printf("Binding on port; %d\n", io->port);
  struct sockaddr_in raddr;
  r = uv_ip4_addr("0.0.0.0", io->port, &raddr);
  if (r != 0) {
    printf("Error: cannot init ip4 addr.\n");
    exit(1);
  }

  r = uv_udp_bind(&io->sock, (const struct sockaddr*)&raddr, 0);
  if(r < 0) {
    printf("Error: cannot bind: %s, in rxs_stun_io\n", uv_strerror(r));
    exit(1);
  }

  /* start receiving data. */
  r = uv_udp_recv_start(&io->sock, on_alloc, on_read);
  if(r < 0) {
    printf("Error: cannot start recieving: %s\n", uv_strerror(r));
    exit(1);
  }
  
  /* @todo --> ready to kickoff stun */
  io->state = RXS_SIO_STATE_RESOLVED;
  
  if (rxs_stun_start(&io->stun) < 0) {
    printf("Error: cannot start the stun process.\n");
    exit(1);
  }
}

static void on_alloc(uv_handle_t* handle, size_t nbytes, uv_buf_t* buf) {

  printf("rxs_stun_io, on alloc.\n");

  rxs_stun_io* io = (rxs_stun_io*) handle->data;

  if (nbytes > RXS_STUN_IO_MEM_BLOCK_SIZE) {
    printf("Error: rxs_stun_io needs a memory block bigger then the anticipated on: %d > %d.\n", nbytes, RXS_STUN_IO_MEM_BLOCK_SIZE);
    exit(1);
  }
  
  rxs_stun_mem* mem = find_free_mem_block(io);
  if (!mem) {
    printf("Error: cannot find any new memory block for rxs_stun_io. Need to allocate more!\n");
    exit(1);
  }

  buf->base = (char*)mem->data;
  buf->len = nbytes;

  mem->nbytes = nbytes;
  mem->is_free = 0;
}

static void on_send_ready(uv_udp_send_t* req, int status) {

  rxs_stun_mem* mem = (rxs_stun_mem*)(req->data);
  mem->is_free = 1;

  printf("Send some data: %d\n", status);

  if(status < 0) {
    printf("Error: while sending some data in rxs_stun_io. %s\n", uv_strerror(status));
    exit(1);
  }

  free(req);
  req = NULL;
}

static void on_read(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {

  printf("Read some data: %ld\n", nread);

  rxs_stun_io* io = (rxs_stun_io*)(handle->data);
  rxs_stun_mem* mem = find_mem_block(io, buf->base);

  /* pass data into the stun parser. */
  rxs_stun_process(&io->stun, buf->base, nread);

  /* set memory block free */
  if (!mem) {
    printf("Error: Cannot set the memory block free in rxs_stun_io. Cannot find it.\n");
    exit(1);
  }
  mem->is_free = 1;

}

/* STUN CALLBACKS                                                              */
/* --------------------------------------------------------------------------- */

static void on_stun_send(rxs_stun* stun, uint8_t* data, uint32_t nbytes) {

  rxs_stun_io* io = (rxs_stun_io*)stun->user;
  
  printf("We need to send some stun data %d bytes..\n", nbytes);

  if (stun_send(io, data, nbytes) < 0) {
    printf("Error: failed sending stun data.\n");
  }
}
