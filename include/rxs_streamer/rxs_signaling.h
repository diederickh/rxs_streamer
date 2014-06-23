/*

  signaling
  ---------
  
  Simple signaling client/server model that can be used together 
  with rxs_stun to exchange the information about the port mappings
  which have been made.  This file contains both a server and client 
  implementation on top of picomsg. 

  The user who wants to receive video, needs to make sure that 
  the ports are correctly mapped using STUN. Once there are some
  mapped ports, the receiver will notify the signal server about what
  port and IP address can be used by the sender. 

 */
#ifndef RXS_SIGNALING_H
#define RXS_SIGNALING_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <hiredis.h> /* redis */
#include <async.h> /* redis */
#include <adapters/libuv.h>  /* redis */

typedef struct rxs_signal rxs_signal;

typedef void(*rxs_signal_cb)(rxs_signal* s, char* ip, uint16_t port);

struct rxs_signal {
  redisAsyncContext* redis;
  uv_loop_t* loop;
  int slot;
  void* user;
  rxs_signal_cb on_address;
  int connected;
};

int rxs_signal_init(rxs_signal* s, const char* ip, uint16_t port);
int rxs_signal_subscribe(rxs_signal* s, int slot);
int rxs_signal_update(rxs_signal* s);
int rxs_signal_store_address(rxs_signal* s, int slot, const char* ip, uint16_t port);
int rxs_signal_retrieve_address(rxs_signal* s, int slot);


#endif
