/*
  
  rxs_reconstruct
  ---------------
  
  rxs_reconstruct takes care of merging rxs_packets with 
  RTP VP8 data and detecting missing sequence numbers. 

*/

#ifndef RXS_RECONSTRUCT_H
#define RXS_RECONSTRUCT_H

#include <rxs_streamer/rxs_packets.h>

typedef struct rxs_reconstruct rxs_reconstruct;
typedef void(*rxs_reconstruct_seqnum_callback)(rxs_reconstruct* rec, uint16_t* seqnums, int num);    /* gets called when a packet is added that an invalid sequence number. */
typedef void(*rxs_reconstruct_frame_callback)(rxs_reconstruct* rec, uint8_t* data, uint32_t nbytes); /* gets called when we've merged a frame */

struct rxs_reconstruct {

  /* buffer + checking */
  rxs_packets packets;
  uint16_t prev_seqnum;
  uint16_t checked_seqnum;
  uint8_t* buffer;
  uint32_t capacity;
  uint8_t found_keyframe;

  /* callbacks */
  void* user;
  rxs_reconstruct_seqnum_callback on_missing_seqnum;
  rxs_reconstruct_frame_callback on_frame;
};

int rxs_reconstruct_init(rxs_reconstruct* recon);                                   /* initialize and allocate necessary memory */
int rxs_reconstruct_clear(rxs_reconstruct* recon);                                  /* frees all allocated memory and goes back to initial state. */
int rxs_reconstruct_add_packet(rxs_reconstruct* recon, rxs_packet* pkt);            /* add packet */
int rxs_reconstruct_check_seqnum(rxs_reconstruct* recon, uint16_t seqnum);          /* checks if the given seqnum is valid; e.g. monotonically incrementing */
int rxs_reconstruct_merge_packets(rxs_reconstruct* recon, uint64_t timestamp);      /* will try to merge packets for the given timestamp */
int rxs_reconstruct_is_frame_complete(rxs_packet** packets, int npackets);          /* checks if the given array contains all packets that are necessary to create a frame. */

#endif
