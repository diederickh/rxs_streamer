/*

  rxs_stun
  --------

  Experimental stun code to test router hole punching. At this moment
  we're only focussing on the bare minimum implementation of STUN to 
  make rxs_streamer work easily while testing.

  With stun you send some packets to a STUN server who replies with information
  about the IP:PORT from which the packets were received and detects the type of NAT 
  device trhough which the packets were sent. The STUN client uses this information
  so that external contacts can reach them withouth the need of manual port 
  forwarding. This doesn't work with all NATs; only will so called "full-cone"
  NATs.

  Testing:
  --------

  Once you got a reply from the stun server you test if the port is 
  correctly forwarded by sending some test UDP packets using netcat: 

     echo -n "hello" | nc -4u -w1 home.roxlu.com 19302

  Keep Alive
  ----------

  From RFC 5245, Section 10:

          If STUN is being used for keepalives, a STUN Binding Indication is
          used [RFC5389].  The Indication MUST NOT utilize any authentication
          mechanism.  It SHOULD contain the FINGERPRINT attribute to aid in
          demultiplexing, but SHOULD NOT contain any other attributes.  It is
          used solely to keep the NAT bindings alive.  The Binding Indication
          is sent using the same local and remote candidates that are being
          used for media.  Though Binding Indications are used for keepalives,
          an agent MUST be prepared to receive a connectivity check as well.
          If a connectivity check is received, a response is generated as
          discussed in [RFC5389], but there is no impact on ICE processing
          otherwise.

  Search for "Seconds" in the RFC 5389 (40 seconds)

  References:
  -----------
  - STUN rfc http://tools.ietf.org/html/rfc5389
  - info on XOR obfuscating which is used with a XOR-MAPPED-ADDRESS: http://blog.malwarebytes.org/intelligence/2013/05/nowhere-to-hide-three-methods-of-xor-obfuscation/
  - javascript stun/turn implemenation: https://github.com/davidrivera/stutter.js
  - Solving the Firewall/NAT Traversal Issue of SIP, Ingate Systems (clear info): https://www.google.nl/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0CC8QFjAA&url=http%3A%2F%2Fwww.ingate.com%2Ffiles%2FSolving_Firewall-NAT_Traversal.pdf&ei=w4uHU5yQFMKmPeaegLgB&usg=AFQjCNE8ln0x-QNr4MWQE_jXIZ09LwbkKw&sig2=PvGsH9MTW-tBZOs09IdYXw&bvm=bv.67720277,d.ZWU
  - Making your computer accessible from the public internet, http://www.nch.com.au/kb/networking-stun.html
  - Keep Alive messages (1), http://tools.ietf.org/html/rfc5245#section-10
  - Keep Alive messages (2), http://tools.ietf.org/html/draft-ietf-rtcweb-stun-consent-freshness-00
  - Simple implementation: https://github.com/gregnietsky/stun-c/blob/master/stun.c

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

#define RXS_STUN_MAGIC_COOKIE         0x2112A442
#define RXS_STUN_BIND_REQUEST         0x0001
#define RXS_STUN_BIND_RESPONSE        0x0101
#define RXS_STUN_MAPPED_ADDRESS       0x0001
#define RXS_STUN_XOR_MAPPED_ADDRESS   0x0020

typedef struct rxs_stun rxs_stun;
typedef struct rxs_stun_attr rxs_stun_attr;
typedef void(*rxs_stun_callback)(rxs_stun* s, uint8_t* data, uint32_t nbytes);

struct rxs_stun_attr {
  int type;                                                            
  struct sockaddr_in address;
};

struct rxs_stun {

  /* buffer and state */
  uint8_t buffer[4096];                                                /* buffer to which we append incoming data and we use to parse */
  uint32_t pos;                                                        /* read position in the buffer */
  uint32_t capacity;                                                   /* how many bytes buffer can handle */
  uint32_t transid[3];                                                 /* our transaction id, used in all messages */

  /* callback */
  void* user;
  rxs_stun_callback on_send;                                           /* gets called whenever the user needs to send some data. */
};

int rxs_stun_init(rxs_stun* st);                                       /* sets up members */
int rxs_stun_start(rxs_stun* st);                                      /* starts the stun process */ 
int rxs_stun_clear(rxs_stun* st);                                      /* clears up any used memory and resets members */
int rxs_stun_process(rxs_stun* st, uint8_t* data, uint32_t nbytes);    /* whenever you receive some data, pass it into this function so we can process it */

#endif
