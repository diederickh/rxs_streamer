#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <rxs_streamer/rxs_types.h>
#include <rxs_streamer/rxs_jitter.h>
#include <uv.h>

/* ----------------------------------------------------------------------------- */

static rxs_packet* jitter_next_packet(rxs_jitter* jit);
static void jitter_on_missing_seqnums(rxs_reconstruct* rc, uint16_t* seqnums, int num);          /* gets called by the reconstructor when it's missing sequence numbers */
static void jitter_on_frame(rxs_reconstruct* rc, uint8_t* data, uint32_t nbytes);                /* gets called by the reconstructor when it could merge packets into a frame */

/* ----------------------------------------------------------------------------- */

int rxs_jitter_init(rxs_jitter* jit) {

  if (!jit) { return -1; } 

  if (rxs_reconstruct_init(&jit->reconstruct) < 0) {
    return -2;
  }

  jit->reconstruct.user = (void*)jit;
  jit->reconstruct.on_missing_seqnum = jitter_on_missing_seqnums;
  jit->reconstruct.on_frame = jitter_on_frame;
  jit->packets = &jit->reconstruct.packets;

  jit->npackets = 0;
  jit->timeout = 0;
  jit->time_start = 0;
  jit->timestamp_start = 0;
  jit->curr_pkt = NULL;
                                                     
  return 0;
}

int rxs_jitter_add_packet(rxs_jitter* jit, rxs_packet* pkt) {

  if (!jit) { return -1; }
  if (!pkt) { return -2; }

  if (rxs_reconstruct_add_packet(&jit->reconstruct, pkt) < 0) {
    return -3;
  }

  if (rxs_reconstruct_check_seqnum(&jit->reconstruct, pkt->seqnum) < 0) {
    return -4;
  }

  jit->npackets++;

  return 0;
}

void rxs_jitter_update(rxs_jitter* jit) {

  uint64_t now  = ((uv_hrtime() - jit->time_start));
  rxs_packet* pkt = NULL;  

  /* only when our buffer is filled */
  if (jit->npackets < (jit->packets->npackets / 2)) {
    return ;
  }

  /* check if there is a packet which needs to be shown */
  if (jit->timeout == 0) {
    jit->timestamp_start = jit->packets->packets[0].timestamp;
    jit->time_start = uv_hrtime();
    jit->timeout = now;
    jit->curr_pkt = &jit->packets->packets[0];
    printf("First timeout set: %llu, first seqnum: %u\n", jit->timeout, jit->curr_pkt->seqnum);
  }

  if (now < jit->timeout) {
    return;
  }

  if (!jit->curr_pkt) {
    printf("Error: cannot find curr pkt.\n");
    return;
  }

  /* construct a packet, @todo - not sure if we need to check the return value here...  */
  rxs_reconstruct_merge_packets(&jit->reconstruct, jit->curr_pkt->timestamp);

  /* find the next timeout */
  pkt = jitter_next_packet(jit);
  if (!pkt) {
    printf("Error: no next packet found. This should not happen (unless sender stopped)!\n");
    exit(0);
  }
  else {
    jit->timeout = pkt->timestamp - jit->timestamp_start;
    jit->curr_pkt = pkt;
    return ;
  }

  printf("no next packet found\n");
  exit(0);
  return ;
}

/* ----------------------------------------------------------------------------- */

/* 
   This function is used to select the next packet that we need to 
   handle. It will find the packet with the next timestamp which is 
   closest to the current one. We don't assume the list to be sorted. 
   We set the next timestamp based on the found packet.
*/
static rxs_packet* jitter_next_packet(rxs_jitter* jit) {
  
  uint64_t best_match = 0;
  uint64_t timestamp = 0; 
  int i = 0;
  rxs_packet* pkt = NULL;

  if (!jit) { return NULL; }
  if (!jit->curr_pkt) { return NULL; } 

  for (i = 0; i < jit->packets->npackets; ++i) {

    /* when the timestamp is less then the current one continue */
    timestamp = jit->packets->packets[i].timestamp;
    if (timestamp <= jit->curr_pkt->timestamp) {
      continue;
    }

    /* when this timestamp is closer to the current one select it */
    if(best_match == 0 || timestamp < best_match) {
      best_match = timestamp;
      pkt = &jit->packets->packets[i];
    }
  }

  if (!pkt) {
    printf("nothing found.\n");
  }
  return pkt;
}

static void jitter_on_missing_seqnums(rxs_reconstruct* rc, uint16_t* seqnums, int num) {
  rxs_jitter* jit = (rxs_jitter*)rc->user;
  if (jit->on_missing_seqnum) {
    jit->on_missing_seqnum(jit, seqnums, num);
  }
}

static void jitter_on_frame(rxs_reconstruct* rc, uint8_t* data, uint32_t nbytes) {
  rxs_jitter* jit = (rxs_jitter*)rc->user;
  if (jit->on_frame) {
    jit->on_frame(jit, data, nbytes);
  }
}
