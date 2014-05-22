#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rxs_streamer/rxs_packets.h>

/* ----------------------------------------------------------------------------- */

static int packets_sort_seqnum(const void* a, const void* b);

/* ----------------------------------------------------------------------------- */

int rxs_packet_init(rxs_packet* pkt) {
  if (!pkt) { return -1; } 

  pkt->is_free = 1;
  pkt->data = NULL;
  pkt->nbytes = 0;
  pkt->capacity = 0;
  pkt->marker = 0;

  return 0;
}

int rxs_packet_clear(rxs_packet* pkt) {
  if (!pkt) { return -1; } 

  pkt->is_free = 1;
  pkt->data = NULL;
  pkt->nbytes = 0;
  pkt->capacity = 0;
  pkt->seqnum = 0;
  pkt->marker = 0;
  pkt->timestamp = 0;

  return 0;
}

int rxs_packet_write(rxs_packet* pkt, uint8_t* data, uint32_t nbytes) {

  if (!pkt) { return -1; } 
  if (!data) { return -2; } 
  if (!nbytes) { return -3; } 

  if (pkt->capacity < nbytes) {
    printf("Error: cannot write data into packet because we don't have enough space.\n");
    return -4;
  }

  memcpy(pkt->data, data, nbytes);

  pkt->nbytes = nbytes;
  pkt->is_free = 0;

  return 0;
}

/* ----------------------------------------------------------------------------- */

int rxs_packets_init(rxs_packets* ps, int num, uint32_t nframebytes) {

  int i;
  uint64_t nbytes;

  if (!ps) { return -1; } 

  /* allocat the packets */
  ps->npackets = num;
  ps->packets = (rxs_packet*)malloc(sizeof(rxs_packet) * num);
  if (!ps->packets) {
    printf("Error: cannot allocate the packets. Out of mem?\n");
    return -2;
  }

  /* allocate our ringbuffer */
  nbytes = nframebytes * num;
  ps->buffer = (uint8_t*)malloc(nbytes);
  if (!ps->buffer) {
    printf("Error: cannot allocate internal storage for packets.\n");
    return -4;
  }

  
  /* initialize the packets */
  for (i = 0; i < num; ++i) {
    if (rxs_packet_init(&ps->packets[i]) < 0) {
      printf("Error: cannot initialize a packet.\n");
      return -3;
    }

    /* set the write pointer */
    ps->packets[i].data = ps->buffer + (i * nframebytes);
    ps->packets[i].capacity = nframebytes;
  }

  return 0;
}

int rxs_packets_clear(rxs_packets* ps) {

  int i;

  if (!ps) { return -1; } 
  if (!ps->npackets) { return -2; }
  if (ps->packets == NULL) { return -3; } 

  for (i = 0; i < ps->npackets; ++i) {
    if (rxs_packet_clear(&ps->packets[i]) < 0) {
      return -4;
    }
  }

  free(ps->packets);
  ps->packets = NULL;
  ps->npackets = 0;

  return 0;
}

/* find a free packet, returns NULL when we don't have any new packets.*/
rxs_packet* rxs_packets_find_free(rxs_packets* ps) { 

  int i;

  for(i = 0; i < ps->npackets; ++i) {
    if (ps->packets[i].is_free)  {
      return &ps->packets[i];
    }
  }

  return NULL;
}

int rxs_packets_sort_seqnum(rxs_packets* ps) {
  if (!ps) { return -1; } 
  
  qsort(ps->packets, ps->npackets, sizeof(rxs_packet), packets_sort_seqnum) ;

  return 0;
}

/* ----------------------------------------------------------------------------- */

static int packets_sort_seqnum(const void* a, const void* b) {

  const rxs_packet* aa = (const rxs_packet*)a;
  const rxs_packet* bb = (const rxs_packet*)b;

#if !defined(NDEBUG)
  /* sequence numbers are supposed to increase! */
  if (aa->seqnum == bb->seqnum) {
    return 0;
  }
#endif

  if (aa->seqnum > bb->seqnum) {
    return 1;
  }
  else {
    return -1;
  }



}
