#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <rxs_streamer/rxs_jitter.h>
#include <uv.h>

/* ----------------------------------------------------------------------------- */

static int jitter_check_dropped_packets(rxs_jitter* jit);

/* ----------------------------------------------------------------------------- */

int rxs_jitter_init(rxs_jitter* jit) {

  if (!jit) { return -1; } 

  if (rxs_packets_init(&jit->packets, 50, 1024 * 8) < 0) {
    printf("Error: cannot initialize the packets buffer for the jitter.\n");
    return -2;
  }

  jit->pos = 0;
  jit->missing_seqnum = 0;
  jit->prev_seqnum = 0;

  return 0;
}


int rxs_jitter_add_packet(rxs_jitter* jit, rxs_packet* pkt) {

  rxs_packet* free_pkt = NULL;

  if (!jit) { return -1; }
  if (!pkt) { return -2; } 

  /* find a free packet */
  free_pkt = rxs_packets_find_free(&jit->packets);
  if (!free_pkt) {
    /* when no free packet found, we reusing the first one, which is always the oldest because we sort on seqnum. */
    free_pkt = &jit->packets.packets[0];
  }

  if (pkt->nbytes > free_pkt->capacity) {
    printf("Error: size of the incoming packet it too big for the jitter buffer: %d > %d.\n", pkt->nbytes, free_pkt->capacity);
    return -3;
  }

  /* copy the packet info */
  free_pkt->is_free = 0;
  free_pkt->marker = pkt->marker;
  free_pkt->timestamp = pkt->timestamp;
  free_pkt->seqnum = pkt->seqnum;

  memcpy((char*)free_pkt->data, (void*)pkt->data, pkt->nbytes);

  /* make sure we're sorted */
  /* @todo - we should implement insertion sort here */
  if (rxs_packets_sort_seqnum(&jit->packets) < 0) {
    printf("Error: could not sort the packets.\n");
    return -5;
  }

  /* tmp */
#if 0 
  int i = 0;
  for (i = 0; i < jit->packets.npackets; ++i) {
    rxs_packet* pp = &jit->packets.packets[i];
    printf("- seqnum: %d, timestamp: %d\n", pp->seqnum, pp->timestamp);
  }
#endif
  /* end */

  /* 
  if (jit->prev_seqnum && pkt->seqnum != (jit->prev_seqnum + 1)) {
    printf("oops missed a packet.\n");
    exit(1);
  }
  jit->prev_seqnum = pkt->seqnum;
  */

  return 0;
}

void rxs_jitter_update(rxs_jitter* jit) {

  /* check if there are losts packets */
  //jitter_check_dropped_packets(jit);
  
  /* check if we need to construct a frame and call the callback */
}

/* this function simple make all packets free again. will be done when the remote stream is restarted */
int rxs_jitter_reset(rxs_jitter* jit){

  int i = 0;

  if (!jit) { return -1; } 
  
  for (i = 0; i < jit->packets.npackets; ++i) {
    jit->packets.packets[i].is_free = 1;
  }
}

/* ----------------------------------------------------------------------------- */

/* This function assumes the packets are already sorted on seqnum */
/* @todo should we keep track if the packets are sorted? */
static int jitter_check_dropped_packets(rxs_jitter* jit) {

  int i = 0;
  uint32_t j = 0;
  uint32_t prev = 0;
  uint32_t prev_ts = 0;
  int64_t gap = 0;
  double inv_ts = 1.0 / 90000.0;
  double time_diff = 0.0;
  rxs_packet* p = NULL;

  if (!jit) { return -1; } 
  if (!jit->on_missing_packet) { return 0; } 

  for(i = 0; i < jit->packets.npackets; ++i) {

    p = &jit->packets.packets[i];
    if (p->is_free) { 
      continue;
    }

    printf("--\n");
    /* check if the sequence numbers aren't monotonically incrementing */
    if (prev && p->seqnum != (prev + 1)) {

      if (p->timestamp < prev_ts) {
        /* detect time difference between previous and current packet (e.g. the remote stream could have been restarted.) */
        printf("Warning: stream restarted (1).\n");
        return rxs_jitter_reset(jit);
      }
      else if ((p->seqnum - prev) > (jit->packets.npackets / 2)) {
        /* gap in sequence number if too large */
        printf("Warning: stream restarted (2).\n");
        return rxs_jitter_reset(jit);
      }
      else if(p->seqnum == prev) {
        /* duplicate packets ? */
        printf("Warning: stream restarted (3).\n");
        return rxs_jitter_reset(jit);
      }
      else {
        /* time different too big? -- @todo it seems we never get to this point? */
        gap = p->timestamp - prev_ts;
        time_diff = gap * inv_ts;
        if (time_diff > 1.0) {
          printf("Time difference too large.\n");
          return rxs_jitter_reset(jit);
        }
      }

      printf("ERROR: previous sequence number was: %d and current: %d which means we're missing some. - %d, time: %f\n", prev, p->seqnum, gap, time_diff);
      
      for(j = (prev + 1); j < p->seqnum; ++j) {
        jit->missing_seqnum = j;
        jit->on_missing_packet(jit);
      }
      /* @todo, do we really want to return here? */
      return 0;
    }

    prev = p->seqnum;
    prev_ts = p->timestamp;

  }

  return 0;
}
