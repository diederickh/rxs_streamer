#ifndef RXS_PACKETS_H
#define RXS_PACKETS_H

#include <stdint.h>

typedef struct rxs_packet rxs_packet;
typedef struct rxs_packets rxs_packets;

struct rxs_packet {
  uint8_t* data;
  uint32_t nbytes;
  int is_free;
};

struct rxs_packets {
  uint8_t* buffer;
  rxs_packet* packets;
  int npackets;
};


int rxs_packet_init(rxs_packet* pkt);    /* initialize a packet */
int rxs_packet_clear(rxs_packet* pkt);   /* clears packets; deallocs if necessary */
int rxs_packets_init(rxs_packets* ps, int num, uint32_t nframebytes);   /* initialize all the packets */
int rxs_packets_clear(rxs_packets* ps);  /* clears/resets the packets */
rxs_packet* rxs_packets_find_free(rxs_packets* ps);

#endif
