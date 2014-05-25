#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <rxs_streamer/rxs_types.h>
#include <rxs_streamer/rxs_reconstruct.h>

/* ----------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------------- */

int rxs_reconstruct_init(rxs_reconstruct* rc) { 

  if (!rc) { return -1; } 

  /* create our internal buffer */
  if (rxs_packets_init(&rc->packets, 24, 1024 * 8) < 0) {
    printf("Error: cannot initialize the packets buffer in reconstruct.\n");
    return -2;
  }


  /* allocate some space for that we use to merge the packets */
  rc->capacity = 1024 * 1024 * 4;
  rc->buffer = (uint8_t*) malloc(rc->capacity); 
  if (!rc->buffer) {
    printf("Error: cannot allocate the buffer into which we write frames in reconstruct.\n");
    return -3;
  }

  rc->prev_seqnum = 0;

  return 0;
}

int rxs_reconstruct_clear(rxs_reconstruct* rc) {
  if (!rc) { return -1; }
  
  rc->prev_seqnum = 0;

  return rxs_packets_clear(&rc->packets);
}


/*
  Add a packet which we can use to reconstruct a frame.
  Make sure that at least the following members of the packet
  are set so we can actually reconstruct packet.

  - marker
  - timestamp
  - seqnum
  - nbytes
  - nonref
  - data

 */
int rxs_reconstruct_add_packet(rxs_reconstruct* rc, rxs_packet* pkt) {

  rxs_packet* free_pkt = NULL;
  
  if (!rc) { return -1; } 
  if (!pkt) { return -2; } 

  /* find a free packet */
  free_pkt = rxs_packets_next(&rc->packets);
  if (!free_pkt) {
    printf("Error: fetching next packet failed in reconstruct.\n");
    return -3;
  }

  if (pkt->nbytes > free_pkt->capacity) {
    printf("Error: size of the incoming packet it too big for the reconstruct buffer: %d > %d.\n", pkt->nbytes, free_pkt->capacity);
    return -4;
  }

  /* copy the packet info */
  free_pkt->marker = pkt->marker;
  free_pkt->timestamp = ((uint64_t)(pkt->timestamp)* ( (double)(1.0/90000) )) * 1000 * 1000 * 1000 ;
  free_pkt->seqnum = pkt->seqnum;
  free_pkt->nbytes = pkt->nbytes;
  free_pkt->nonref = pkt->nonref;

  memcpy((char*)free_pkt->data, (void*)pkt->data, pkt->nbytes);

  return 0;
}

int rxs_reconstruct_check_seqnum(rxs_reconstruct* rc, uint16_t seqnum) {

  static uint16_t missing_seqnums[RXS_MAX_MISSING_PACKETS]; 
  int i;
  uint32_t nmissing = 0;

  /* early check for missing packets */
  if (rc->prev_seqnum && seqnum != (rc->prev_seqnum + 1)) {
    for (i = rc->prev_seqnum + 1; i < seqnum; ++i) {
      printf("Missing? %d\n", i);
      missing_seqnums[nmissing] = i;
      nmissing++;
    }
    /* @todo add missing packet callback in reconstruct */

    if (nmissing > 0 && rc->on_missing_seqnum) {
      rc->on_missing_seqnum(rc, missing_seqnums, nmissing);
    }
  }

  if (seqnum > rc->prev_seqnum) {
    rc->prev_seqnum = seqnum;
  }

  return 0;
}

int rxs_reconstruct_merge_packets(rxs_reconstruct* rc, uint64_t timestamp) {
  rxs_packet* pkt = NULL;
  int i = 0;
  int j = 0;
  uint32_t pos = 0;
  rxs_packet* packets[RXS_MAX_SPLIT_PACKETS];

  if (!rc) { return -1; } 

  
  return 0;
}
