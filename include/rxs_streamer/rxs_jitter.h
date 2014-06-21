/* 

   rxs_jitter
   ----------

   In short: rxs_jitter creates a delayed playback which gives the user
             some time to request the sender to resend lost packets.

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

   Simulate packet loss
   --------------------
   On linux you can use `tc` to simulate packet loss:

   ````sh

      # add rule
      $ tc qdisc add dev enp6s0 root netem loss 1%
   
      $ remove rule
      $ tc qdisc del dev enp6s0 root netem loss 1%

   ````


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

typedef struct rxs_jitter rxs_jitter;
typedef void(*rxs_jitter_seqnum_callback)(rxs_jitter* jit, uint16_t* seqnums, int num);     /* gets called we're missing some sequence numbers */
typedef void(*rxs_jitter_frame_callback)(rxs_jitter* jit, uint8_t* data, uint32_t nbytes);  /* gets called when we've constructed a raw VP8 frame with encoded data */

struct rxs_jitter {

  /* packet management */
  rxs_reconstruct reconstruct;                                   /* used to collect packets, detect missing ones and reconstruct vp8 frames */
  rxs_packets* packets;                                          /* pointer to the rxs_packets of the reconstructor to keep some buffer with packets */
  uint64_t npackets;                                             /* number of added packets */
  
  /* callbacks */
  void* user;                                                    /* can be set to somether the user of this code wants */ 
  rxs_jitter_seqnum_callback on_missing_seqnum;                  /* gets called directly when we notice a missed packed. */ 
  rxs_jitter_frame_callback on_frame;                            /* gets called when we have a frame that needs to be decoded and displayed */

  /* playback / packet loss */
  uint64_t timeout;                                              /* when we should show the next frame */
  uint64_t time_start;                                           /* the wall time when we started handling our first packet. */
  uint64_t timestamp_start;                                      /* the timestamp since we started handling packets */
  rxs_packet* curr_pkt;                                          /* the currently handled packet; is used to check e.g. what next timestamp we need to handle */
};

int rxs_jitter_init(rxs_jitter* jit);                            /* initialize the jitter handler */
int rxs_jitter_add_packet(rxs_jitter* jit, rxs_packet* pkt);     /* add a new packet, see the code to check what members which must be set. */
void rxs_jitter_update(rxs_jitter* jit);                         /* must be called often so we can check if there missing frames */

#endif
