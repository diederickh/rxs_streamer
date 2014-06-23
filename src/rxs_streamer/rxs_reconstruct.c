#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <rxs_streamer/rxs_types.h>
#include <rxs_streamer/rxs_reconstruct.h>

/* ----------------------------------------------------------------------------- */

static int reconstruct_check_sequence_order(rxs_reconstruct* rc, rxs_packet** packets, int npackets); /* this will check if the sequence number in the given collection is correct */

/* ----------------------------------------------------------------------------- */

int rxs_reconstruct_init(rxs_reconstruct* rc) { 

  if (!rc) { return -1; } 

  /* create our internal buffer */
  /* @todo the buffer size should be optional in reconstruct !! */
  /* @todo the size of the packets should be optional too */
  if (rxs_packets_init(&rc->packets, 128, 1024 * 32) < 0) {
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
  rc->checked_seqnum = 0;
  rc->found_keyframe = 0;

  return 0;
}

int rxs_reconstruct_clear(rxs_reconstruct* rc) {
  if (!rc) { return -1; }
  
  rc->prev_seqnum = 0;
  rc->checked_seqnum = 0;
  rc->capacity = 0;
  rc->found_keyframe = 0;
  
  if (rc->buffer) { 
    free(rc->buffer);
    rc->buffer = NULL;
  }

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

  /* @todo - we use a different timestamp internally (see free_pkt->timestamp 
             a couple of lines above. When you want to use this together with 
             rxs_reconstruct_merge_packets using pkt->timestamp, it would never
             work because we're using diferent timestamps... 

             This is ugly design an we shouldn't change the timestamp internally
             I tink. 

  */
  pkt->timestamp = free_pkt->timestamp;

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

  int i = 0;
  uint32_t pos = 0;
  rxs_packet* pkt = NULL;
  rxs_packet* packets[RXS_MAX_SPLIT_PACKETS];
  int npackets = 0; /* number of found packets for given timestamp */

  if (!rc) { return -1; } 

  /* get packets for timestamp */
  npackets = rxs_packets_find_timestamp(&rc->packets, timestamp, packets, RXS_MAX_SPLIT_PACKETS);
  if (npackets <= 0) {
    return -2; 
  }

  /* sort on sequence number */
  if (rxs_packets_sort_seqnum(packets, npackets) < 0) {
    return -4;
  };
  
  if (reconstruct_check_sequence_order(rc, packets, npackets) < 0) {
    printf("Warning: packet sequence numbers are invalid in reconstruct.\n");
    /* @todo - should we "abort" here, or continue */
  }

  /* check if the found packets can create a complete frame */
  if (rxs_reconstruct_is_frame_complete(packets, npackets) < 0) {
    return -5;
  }

  /* merge rtp vp8 partitions/packets */
  for (i = 0; i < npackets; ++i) {

    pkt = packets[i];
    if ( (pos + pkt->nbytes) > rc->capacity) {
      printf("Error: the rxs_reconstruct.buffer member doesn't have enough capacity to store a frame.\n");
      return -6;
    }

    memcpy(rc->buffer + pos, pkt->data, pkt->nbytes);
    pos += pkt->nbytes;

    if (!rc->found_keyframe && pkt->nonref == 0) {
      rc->found_keyframe = 1;
    }
  }

  /* call the frame callback */
  if (rc->found_keyframe && rc->on_frame) {
    rc->on_frame(rc, rc->buffer, pos);
  }

  return 0;
}

/* 
   This function will check if the packets in the given 
   array can be used to create a complete VP8 frame. The
   logic is simple: the last last packet should have the
   marker bit set. We assume that the sequence numbers are 
   correctly sorted and no packets are missing. 

   It will return 0 when the frame is complete else < 0

   @todo see 4.5.  Frame reconstruction algorithm at
         http://tools.ietf.org/html/draft-ietf-payload-vp8-11

*/
int rxs_reconstruct_is_frame_complete(rxs_packet** packets, int npackets) {
#if !defined(NDEBUG)
  if (!packets) { return -1; } 
  if (!npackets || npackets < 1) { return -2; } 
#endif

  return (packets[npackets - 1]->marker == 1) ? 0 : -3;
}


/* ----------------------------------------------------------------------------- */

/*
  This will check if the sequence order in the given array is correct. 

  IMPORTANT: make sure that the packets are already sorted by sequence order.

  @todo maybe add a check for unsorted arrays?

 */
static int reconstruct_check_sequence_order(rxs_reconstruct* rc, rxs_packet** packets, int npackets) {
  uint32_t i;
  int r = 0;

#if !defined(NDEBUG)
  if (!rc) { return -1; } 
  if (!npackets) { return -2; } 
  if (!packets) { return -3; } 
#endif


  for (i = 0; i < npackets; ++i) {
    if (rc->checked_seqnum && (rc->checked_seqnum + 1) != packets[i]->seqnum) {
      r = -1;
      //printf("~~ missing: %d\n", jit->checked_seqnum +1 );

      /* @todo  this is where a packet is really lost and we
                should probably ask the sender for a new 
                keyframe. 
      */
    }
    rc->checked_seqnum = packets[i]->seqnum;
  }

  return 0;
}



  

