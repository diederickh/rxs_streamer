/* 

   rxs_packets
   ------------

   rxs_packets implements a very simplistic "ring" buffer. You 
   initiliase the buffer by calling `rxs_packets_init()` where you 
   specify the maximum allowed 'data size'. When you have some data
   for a specific packet you find a free packet using `rxs_packets_find_free()`
   and the write data into by using `rxs_packet_write()`.

   rxs_packets is used as the base to implement a simplistic jitter
   buffer. Implementing a jitter buffer means that both the sender
   and receiver need to keep track of N-packets. The sender needs this
   so it can resend lost packets and the receiver needs it to 
   sort and validate incoming packets. 

   The data the sender and receiver store in the packets doesn't have
   be the same. E.g. the sender can keep track of the raw RTP packets 
   while the receiver might only want to store the raw VP8 partitions
   or payload data. 

   <example>
           #include <stdint.h>
           #include <stdio.h>
           #include <stdlib.h>
           #include <string.h>
           #include <rxs_streamer/rxs_packets.h>
           
           #define FAKE_SIZE (1024 * 1024)
           
           int main() {
             printf("\n\ntest_packets.\n\n");
           
             int i;
             rxs_packet* pkt;
             rxs_packets ps;
             uint8_t* fake_data;
           
             if (!rxs_packets_init(&ps, 10, FAKE_SIZE) < 0) {
               printf("Error: cannot create the packets buffer.\n");
               exit(1);
             }
           
             fake_data = (uint8_t*)malloc(FAKE_SIZE);
             if (!fake_data) {
               printf("Error: out of mem, cannot alloc tmp buffer.\n");
               exit(1);
             }
           
             for (i = 0; i < 15; ++i) {
               pkt = rxs_packets_find_free(&ps);
               if (!pkt) {
                 printf("Warning: cannot find a free packet for: %d\n", i);
               }
               else {
                 printf("Writing into a packet: %p, %zu bytes\n", pkt->data, FAKE_SIZE);
                 if (rxs_packet_write(pkt, fake_data, sizeof(fake_data)) < 0) {
                   printf("Error: cannot write to packet.\n");
                 }
                 else {
                   printf("Wrote some fake data.\n");
                 }
               }
               printf("-\n");
             }
           
             free(fake_data);
             fake_data = NULL;

             if (rxs_packets_clear(&ps) < 0) {
               printf("Error: cannot clear the packet buffer. Leaking here.\n");
               exit(1);
             }

             printf("\n\n");
             return 0;
           }
           
   </example>

 */
#ifndef RXS_PACKETS_H
#define RXS_PACKETS_H

#include <stdint.h>

typedef struct rxs_packet rxs_packet;
typedef struct rxs_packets rxs_packets;

struct rxs_packet {
  uint8_t marker;        /* copy of the the RTP m flag */
  uint32_t timestamp;    /* can be used by user to playback */
  uint32_t seqnum;       /* sequence number that can be used to detect dropped or out-of-order packets */
  uint8_t is_free;       /* indicates if this packet is still free and can be used to write some data into */
  uint8_t* data;         /* the actual packet data; can be e.g. a raw RTP packet, or a raw VP8 frame */
  uint32_t nbytes;       /* number of bytes written into data, can't be more then capacity */
  uint32_t capacity;     /* capacity that you can write into this packet. same as nframebytes when initialising with rxs_packets_init() */
};

struct rxs_packets {
  uint8_t* buffer;
  rxs_packet* packets;
  int npackets;
  uint32_t max_frame_nbytes;
};

int rxs_packet_init(rxs_packet* pkt);                                                       /* initialize a packet */
int rxs_packet_clear(rxs_packet* pkt);                                                      /* clears packets; deallocs if necessary */
int rxs_packet_write(rxs_packet* pkt, uint8_t* data, uint32_t nbytes);                      /* write the given data into the ringbuffer and set the members of rxs_packet */
int rxs_packets_init(rxs_packets* ps, int num, uint32_t nframebytes);                       /* initialize all the packets */
int rxs_packets_clear(rxs_packets* ps);                                                     /* clears/resets the packets */
int rxs_packets_sort_seqnum(rxs_packets* ps);                                               /* sort on seq num */

rxs_packet* rxs_packets_find_free(rxs_packets* ps);

#endif
