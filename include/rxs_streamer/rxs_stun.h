/*

  rxs_stun
  --------

  Experimental stun code to test router hole punching. At this moment
  we're only focussing on the bare minimum implementation of STUN to 
  make rxs_streamer work easily while testing.


  References:
  -----------
  - STUN rfc http://tools.ietf.org/html/rfc5389
  - info on XOR obfuscating which is used with a XOR-MAPPED-ADDRESS: http://blog.malwarebytes.org/intelligence/2013/05/nowhere-to-hide-three-methods-of-xor-obfuscation/
  - javascript stun/turn implemenation: https://github.com/davidrivera/stutter.js

 */

#ifndef RXS_STUN_H
#define RXS_STUN_H

/* networking */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define RXS_STUN_MAGIC_COOKIE 0x2112A442
#define RXS_STUN_BIND_REQUEST 0x0001
#define RXS_STUN_BIND_RESPONSE 0x0101
#define RXS_STUN_MAPPED_ADDRESS 0x0001
#define RXS_STUN_XOR_MAPPED_ADDRESS 0x0020
#define RXS_STUN_STATE_NONE 0
#define RXS_STUN_STATE_BIND_REQUEST 1

typedef struct rxs_stun rxs_stun;
typedef void(*rxs_stun_callback)(rxs_stun* s, uint8_t* data, uint32_t nbytes);
/*
typedef struct rxs_stun_msg srx_stunc_msg;
typedef struct rxs_stun_attr srx_stun_attr;
typedef struct rxs_stun_mem src_stun_mem;
*/

struct rxs_stun {

  /* buffer and state */
  uint8_t buffer[4096];                    /* buffer to which we append incoming data and we use to parse */
  uint32_t dx;                             /* read position in the buffer */
  int state;                               /* state of stun; handled internally, @todo - check if we really need state */

  /* callback */
  void* user;
  rxs_stun_callback on_send;               /* gets called whenever the user needs to send some data. */
};

int rxs_stun_init(rxs_stun* st);           /* starts the stun process */
int rxs_stun_start(rxs_stun* st);   
void rxs_stun_update(rxs_stun* st);
int rxs_stun_clear(rxs_stun* st);

#endif
