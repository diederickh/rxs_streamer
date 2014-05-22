/* 
   rxs_jitter
   ----------

   This code implements a very simple jitter buffer. Goal of this jitter buffer 
   is to keep N-millis of video frames in a buffer before we're giving it back 
   to the user to display. This means the the video will be delayed by N-millis,
   but this also means that we can re-order, re-send, etc.. packets which 
   are invalid. 

   Also the jitter buffer will handle timings and frame construction too. Normally
   a VP8 frame will be sent using a couple (mostly 2), RTP packets and we need to 
   construct a complete frame when we receive a packet with an end marker for the 
   same timestamp so it can be fed into the decoder.  Internally we use the 
   `buffer` member to store a complete frame and pass this to the callback whenever
   you need to display a frame. 

   
 */

#ifndef RXS_JITTER_H
#define RXS_JITTER_H

#include <rxs_streamer/rxs_packets.h>

typedef struct rxs_jitter rxs_jitter;
typedef void(*rxs_jitter_callback)(rxs_jitter* jit);

struct rxs_jitter {
  rxs_packets packets;
  uint8_t* buffer;
  uint32_t pos;
  uint32_t missing_seqnum; /* will be set to the missing sequence number when we call the on_missing_packet function */
  uint32_t prev_seqnum; 
  void* user;
  rxs_jitter_callback on_missing_packet;
};

int rxs_jitter_init(rxs_jitter* jit);
int rxs_jitter_add_packet(rxs_jitter* jit, rxs_packet* pkt);
int rxs_jitter_reset(rxs_jitter* jit);                          /* resets the jitter buffer */
void rxs_jitter_update(rxs_jitter* jit);                        /* must be called often so we can check if there missing frames */

#endif
