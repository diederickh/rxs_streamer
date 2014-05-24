#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <rxs_streamer/rxs_jitter.h>
#include <uv.h>

/* ----------------------------------------------------------------------------- */

static rxs_packet* jitter_next_packet(rxs_jitter* jit);
static int jitter_merge_packets(rxs_jitter* jit, uint64_t timestamp);
static int jitter_sort_seqnum(const void* a, const void* b);
static int jitter_get_packets_for_timestamp(rxs_jitter* jit, uint64_t timestamp, rxs_packet** result, int maxPackets); /* fills the given `result` parameter with packets with the same timestamp */
static int jitter_check_sequence_order(rxs_jitter* jit, rxs_packet** packets, int npackets); /* detects missing sequence number. when an error occurs or when it detects a missing sequence it returns a value < 0. */
static int jitter_is_frame_complete(rxs_packet** packets, int npackets);                     /* checks if the given packets can form a complete frame. */

/* ----------------------------------------------------------------------------- */

int rxs_jitter_init(rxs_jitter* jit) {

  int nsize = 24;

  if (!jit) { return -1; } 


  if (rxs_packets_init(&jit->packets, nsize, 1024 * 8) < 0) {
    printf("Error: cannot initialize the packets buffer for the jitter.\n");
    return -2;
  }

  jit->npackets = 0;
  jit->prev_seqnum = 0;
  jit->checked_seqnum = 0;
  jit->timeout = 0;
  jit->time_start = 0;
  jit->timestamp_start = 0;
  jit->found_keyframe = 0;
  jit->curr_pkt = NULL;


  /* allocate some space for that we use to merge the packets */
  jit->capacity = 1024 * 1024 * 4;
  jit->buffer = (uint8_t*) malloc(jit->capacity); 
  if (!jit->buffer) {
    printf("Error: cannot allocate the buffer into which we write frames.\n");
    return -3;
  }
                                                     
  return 0;
}

int rxs_jitter_add_packet(rxs_jitter* jit, rxs_packet* pkt) {

  static uint16_t missing_seqnums[15]; /* we need to store this max somewhere; should be same as RXS_CONTROL_MAX_SEQNUM */
  uint32_t nmissing = 0;
  uint32_t i;
  rxs_packet* free_pkt = NULL;

  if (!jit) { return -1; }
  if (!pkt) { return -2; } 

  /* find a free packet */
  free_pkt = rxs_packets_next(&jit->packets);
  if (!free_pkt) {
    printf("Error: fetching next packet failed.\n");
    return -3;
  }

  if (pkt->nbytes > free_pkt->capacity) {
    printf("Error: size of the incoming packet it too big for the jitter buffer: %d > %d.\n", pkt->nbytes, free_pkt->capacity);
    return -4;
  }

  /* copy the packet info */
  free_pkt->marker = pkt->marker;
  free_pkt->timestamp = ((uint64_t)(pkt->timestamp)* ( (double)(1.0/90000) )) * 1000 * 1000 * 1000 ;
  free_pkt->seqnum = pkt->seqnum;
  free_pkt->nbytes = pkt->nbytes;
  free_pkt->nonref = pkt->nonref;

  memcpy((char*)free_pkt->data, (void*)pkt->data, pkt->nbytes);

  printf("+ %d\n", pkt->seqnum);

  /* early check for missing packets */
  if (jit->prev_seqnum && pkt->seqnum != (jit->prev_seqnum + 1)) {
    for (i = jit->prev_seqnum + 1; i < pkt->seqnum; ++i) {
      printf("Missing? %d\n", i);
      missing_seqnums[nmissing] = i;
      nmissing++;
    }
    if (nmissing > 0 && jit->on_missing_seqnum) {
      jit->on_missing_seqnum(jit, missing_seqnums, nmissing);
    }
  }

  jit->npackets++;

  /* pkt could be a packet the was resend, so only update when it's higher. */
  if (pkt->seqnum > jit->prev_seqnum) {
    jit->prev_seqnum = pkt->seqnum;
  }

  return 0;
}

void rxs_jitter_update(rxs_jitter* jit) {

  uint64_t now  = ((uv_hrtime() - jit->time_start));
  rxs_packet* pkt = NULL;  

  /* only when our buffer is filled */
  if (jit->npackets < (jit->packets.npackets / 2)) {
    return ;
  }

  /* check if there is a packet which needs to be shown */
  if (jit->timeout == 0) {
    jit->timestamp_start = jit->packets.packets[0].timestamp;
    jit->time_start = uv_hrtime();
    jit->timeout = now;
    jit->curr_pkt = &jit->packets.packets[0];
    printf("First timeout set: %llu, first seqnum: %lld\n", jit->timeout, jit->curr_pkt->seqnum);
  }

  if (now < jit->timeout) {
    return;
  }

  if (!jit->curr_pkt) {
    printf("Error: cannot find curr pkt.\n");
    return;
  }

  /* construct a packet */
  jitter_merge_packets(jit, jit->curr_pkt->timestamp);

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

  for (i = 0; i < jit->packets.npackets; ++i) {

    /* when the timestamp is less then the current one continue */
    timestamp = jit->packets.packets[i].timestamp;
    if (timestamp <= jit->curr_pkt->timestamp) {
      continue;
    }

    /* when this timestamp is closer to the current one select it */
    if(best_match == 0 || timestamp < best_match) {
      best_match = timestamp;
      pkt = &jit->packets.packets[i];
    }
  }

  if (!pkt) {
    printf("nothing found.\n");
  }
  return pkt;
}

static int jitter_sort_seqnum(const void* a, const void* b) {

  rxs_packet* aa = *(rxs_packet**)a;
  rxs_packet* bb = *(rxs_packet**)b;

  if (aa->seqnum > bb->seqnum) {
    return 1;
  }
  else {
    return -1;
  }
}

#define RXS_MAX_SPLIT_PACKETS 8

static int jitter_merge_packets(rxs_jitter* jit, uint64_t timestamp) { 

  rxs_packet* pkt = NULL;
  int i = 0;
  int j = 0;
  uint32_t pos = 0;
  rxs_packet* packets[RXS_MAX_SPLIT_PACKETS];
  
  if (!jit) { return -1; } 

  /* collect all packets with the same timestamp */
  j = jitter_get_packets_for_timestamp(jit, timestamp, packets, RXS_MAX_SPLIT_PACKETS);
  if (j <= 0) {
    return -2;/* shouldn't happen */
  }

  /* check if there are missing packets */
  if (jitter_check_sequence_order(jit, packets, j) < 0) {
    return -3;
  }

  /* check if the found packets can create a complete frame */
  if (jitter_is_frame_complete(packets, j) < 0) {
    return -4;
  }

  /* merge rtp vp8 partitions/packets */
  for (i = 0; i < j; ++i) {

    pkt = packets[i];
    if ( (pos + pkt->nbytes) > jit->capacity) {
      printf("Error: the rxs_jitter.buffer member doesn't have enough capacity to store a frame.\n");
      return -5;
    }

    memcpy(jit->buffer + pos, pkt->data, pkt->nbytes);
    pos += pkt->nbytes;

    if (!jit->found_keyframe && pkt->nonref == 0) {
      jit->found_keyframe = 1;
    }
  }

  /* call the frame callback */
  if (jit->found_keyframe && jit->on_frame) {
    jit->on_frame(jit, jit->buffer, pos);
  }

  return 0;
}

static int jitter_check_sequence_order(rxs_jitter* jit, 
                                       rxs_packet** packets, 
                                       int npackets) 
{
  uint32_t i;

#if !defined(NDEBUG)
  if (!jit) { return -1; } 
  if (!npackets) { return -2; } 
  if (!packets) { return -3; } 
#endif

  /* sort on sequence number */
  qsort(packets, npackets, sizeof(rxs_packet*), jitter_sort_seqnum);

  for(i = 0; i < npackets; ++i) {
    if ( jit->checked_seqnum && (jit->checked_seqnum + 1) != packets[i]->seqnum) {
      //printf("~~ missing: %d\n", jit->checked_seqnum +1 );

      /* @todo  this is where a packet is really lost and we
                should probably ask the sender for a new 
                keyframe. 
      */
    }
    jit->checked_seqnum = packets[i]->seqnum;
  }

  return 0;
}

static int jitter_get_packets_for_timestamp(rxs_jitter* jit, 
                                            uint64_t timestamp, 
                                            rxs_packet** result, 
                                            int maxPackets)
{
  rxs_packet* pkt = NULL;
  uint32_t i = 0;
  uint32_t j = 0;

#if !defined(NDEBUG)
  if (!jit) { return -1; } 
  if (maxPackets == 0) { return -2; } 
  if (!result) { return -3; } 
#endif

  /* collect packets with same timestamp */
  for (i = 0; i < jit->packets.npackets; ++i) {

    pkt = &jit->packets.packets[i];
    if (pkt->timestamp != timestamp) {
      continue;
    }
     
    result[j++] = pkt;

    if (j >= RXS_MAX_SPLIT_PACKETS) {
      break;
    }
  }

  return j;
}

/* 
   This function will check if the packets in the given 
   array can be used to creat a complete VP8 frame. The
   logic is simple: the last last packet should have the
   marker bit set. We assume that the sequence numbers are 
   correctly sorted and no packets are missing. 

   It will return 0 when the frame is complete else < 0

   @todo see 4.5.  Frame reconstruction algorithm at
         http://tools.ietf.org/html/draft-ietf-payload-vp8-11

*/
static int jitter_is_frame_complete(rxs_packet** packets, int npacket) {
#if !defined(NDEBUG)
  if (!packets) { return -1; } 
  if (!npacket || packet < 1) { return -2; } 
#endif

  return (packets[npacket - 1]->marker == 1) ? 0 : -3;
}
