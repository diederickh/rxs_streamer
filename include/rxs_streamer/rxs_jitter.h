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


   Approach:
   ---------

   The jitter buffer keeps N-packets and splits the packet list in three partitions,
   the minimal partition should be corret (all in order, no missing packetes), the 
   validating buffer is where we check for lost and or out-of-order packets. When 
   a packet is lost in this buffer, we drop all packets until we find a keyframe,


   +------------------------------------+
   | timestamp     |  sequence number   |
   +------------------------------------+ -+
   |    1          |    500             |  |
   |    2          |    501             |  |
   |    3          |    502             |  | = minimal buffer
   |    4          |    503             |  |
   |    5          |    504             |  |
   |    6          |    505             | -+  
   +---------------+--------------------+ -+
   |    7          |    508             |  |
   |    8          |    507             |  | = validating buffer
   |    9          |    506             |  | 
   |   10          |    509             |  |
   +---------------+--------------------+ -+
   |   11          |    510             | -+ 
   |   12          |    511             |  |
   |   13          |    515             |  | = accumulating buffer
   |   14          |    512             |  |                            
   |   15          |    514             |  |
   +---------------+--------------------+ -+

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

#include <rxs_streamer/rxs_packets.h>

#define RXS_JIT_MODE_NONE 0     /* default we don't do anything */
#define RXS_JIT_MODE_RECOVER 1  /* this simply means that when we detect a missing packet, we try to find the next keyframe and start there */
#define RXS_JIT_MODE_RESEND 2   /* NOT IMPLEMENTED ask the sender to resend a packet when it was lost; .. NOT IMPLEMENTED */

typedef struct rxs_jitter rxs_jitter;
typedef void(*rxs_jitter_callback)(rxs_jitter* jit);
typedef void(*rxs_jitter_seqnum_callback)(rxs_jitter* jit, uint16_t* seqnums, int num); /* gets called we're missing some sequence number */
typedef void(*rxs_jitter_packet_callback)(rxs_jitter* jit, rxs_packet* pkt);
typedef void(*rxs_jitter_frame_callback)(rxs_jitter* jit, uint8_t* data, uint32_t nbytes); 

struct rxs_jitter {
  rxs_packets packets;
  uint8_t* buffer;                            /* used to merge packets for a frame */
  uint32_t capacity;                          /* capacity of the rxs_jitter.buffer */
  uint32_t pos;
  uint32_t missing_seqnum;                     /* will be set to the missing sequence number when we call the on_missing_seqnum function */
  uint32_t prev_seqnum;                        /* the previous added sequence number, when out of order we call on_missing_packet */
  uint32_t checked_seqnum;                     /* the previous checked sequence number; used to detect sequence number order */
  uint64_t npackets;                           /* number of added packets */
  uint32_t correct_dx;                         /* all packets from index 0 till correct_dx should be in order and "correct" */
  uint32_t check_dx;                           /* from correct_dx till check_dx we check for missing packets */

  void* user;
  rxs_jitter_seqnum_callback on_missing_seqnum;
  rxs_jitter_frame_callback on_frame;          /* gets called when we have a frame that needs to be decoded and displayed */
  rxs_jitter_packet_callback on_packet;

  uint64_t timeout;                            /* when we should show the next frame */
  uint64_t time_start;
  uint64_t timestamp_start;
  uint32_t next_dx; /* tmp */
  uint32_t curr_dx; /* tmp */
  uint32_t packet_dx; 
  
  rxs_packet* curr_pkt;

  uint32_t write_dx;
  uint8_t found_keyframe;                     /* we will only start calling on_frame when we received a keyframe, else the vpx decoder will crashe on linux */
  uint8_t mode;
};

int rxs_jitter_init(rxs_jitter* jit);
int rxs_jitter_add_packet(rxs_jitter* jit, rxs_packet* pkt);
int rxs_jitter_reset(rxs_jitter* jit);                          /* resets the jitter buffer */
void rxs_jitter_update(rxs_jitter* jit);                        /* must be called often so we can check if there missing frames */

#endif
