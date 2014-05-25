/* 

   rxs_jitter
   ----------

   This code implements a very simple jitter buffer. Goal of this jitter buffer 
   is to keep N-packets of video frames in a buffer before we're giving it back 
   to the user to display. This means the the video will be delayed by N-packets
   but this also means that we can re-order, re-send, etc.. packets which 
   are invalid. 

   Also the jitter buffer will handle timings and frame construction too. Normally
   a VP8 frame will be sent using a couple (mostly 2), RTP packets and we need to 
   construct a complete frame when we receive a packet with an end marker for the 
   same timestamp so it can be fed into the decoder.  Internally we use the 
   `buffer` member to store a complete frame and pass this to the callback whenever
   you need to display a frame. 

   Whenever a packet is added to the "jitter buffer" by calling `rxs_jitter_add_packet()`
   we check if the new sequence number is correct. When not, we will collect the
   sequence numbers and call the set `on_missing_seqnum()` callback. You should
   request the sender to resend these packets for you.

   Whenever we have constructured a new VP8 frame we will call the `on_frame()` 
   callback. The data you get passed into ths function can be passed directly 
   into the vpx decoder. 

   Modes:
   ------
   
   RXS_JITTER_MODE_NONE
          When RXS_JITTER_MODE_NONE is used, the rxs_jitter can still be used to 
          reconstruct 

   <example>

     rxs_jitter jit;
     
     rxs_jitter_init(&jit);
     
     jit.on_missing_seqnum = my_seqnum_callback;
     jit.on_frame = on_frame_callback;
  
     // add packet 
     // --------------------
     rxs_packet p;
     p.marker = d->marker;
     p.timestamp = d->timestamp;
     p.seqnum = d->seqnum; 
     p.data = buffer;
     p.nbytes = nbytes;
     p.nonref = d->N;
  
     rxs_jitter_add_packet(&jit, pkt);

     // update as often as possible (in your event/app loop)
     rxs_jitter_update(&jit);

   </example>


   References:
   -----------

   http://tools.ietf.org/html/rfc4585
         Extended RTP Profile for Real-time Transport Control Protocol (RTCP)-Based Feedback (RTP/AVPF)


   http://tools.ietf.org/html/rfc5104
          RTP Audio/Video Profile with Feedback

   RTCP-FB
   RTCP-FB PLI/FIR
   RTCP PLI       

*/
#ifndef RXS_JITTER_H
#define RXS_JITTER_H

#include <rxs_streamer/rxs_reconstruct.h>
#include <rxs_streamer/rxs_packets.h>

#define JITTER_MODE_NONE 0      /* This will not create an internal buffer so all incoming packets will be directly merged into a VP8 frame if possible, but when we're missing a packet the callback will be called. */
#define JITTER_MODE_BUFFER 1    /* When using a buffer mode we will first buffer N-packets and delay playback so you have some time to request dropped frames again. */

typedef struct rxs_jitter rxs_jitter;
typedef void(*rxs_jitter_seqnum_callback)(rxs_jitter* jit, uint16_t* seqnums, int num);     /* gets called we're missing some sequence numbers */
typedef void(*rxs_jitter_frame_callback)(rxs_jitter* jit, uint8_t* data, uint32_t nbytes);  /* gets called when we've constructed a raw VP8 frame with encoded data */

struct rxs_jitter {

  rxs_reconstruct reconstruct;                                   /* used to collect packets, detect missing ones and reconstruct vp8 frames */

  /* buffer management */
  uint64_t npackets;                                             /* number of added packets */
  rxs_packets packets;                                           /* we use a rxs_packets to keep some buffer with packets */
  uint8_t* buffer;                                               /* used to merge packets for a frame, this will hold a complete VP8 frame that one can decode. */
  uint32_t capacity;                                             /* capacity of the rxs_jitter.buffer */
  
  /* callbacks */
  void* user;                                                    /* can be set to somether the user of this code wants */ 
  rxs_jitter_seqnum_callback on_missing_seqnum;                  /* gets called directly when we notice a missed packed. */ 
  rxs_jitter_frame_callback on_frame;                            /* gets called when we have a frame that needs to be decoded and displayed */

  /* playback / packet loss */
  uint64_t timeout;                                              /* when we should show the next frame */
  uint64_t time_start;                                           /* the wall time when we started handling our first packet. */
  uint64_t timestamp_start;                                      /* the timestamp since we started handling packets */
  uint32_t prev_seqnum;                                          /* the previous added sequence number, when out of order we call on_missing_packet */
  uint32_t checked_seqnum;                                       /* the previous checked sequence number; used to detect sequence number order */
  rxs_packet* curr_pkt;                                          /* the currently handled packet; is used to check e.g. what next timestamp we need to handle */
  uint8_t found_keyframe;                                        /* we will only start calling on_frame when we received a keyframe, else the vpx decoder will crashe on linux */
};

int rxs_jitter_init(rxs_jitter* jit);                            /* initialize the jitter handler */
int rxs_jitter_add_packet(rxs_jitter* jit, rxs_packet* pkt);     /* add a new packet, see the code to check what members which must be set. */
void rxs_jitter_update(rxs_jitter* jit);                         /* must be called often so we can check if there missing frames */

#endif
