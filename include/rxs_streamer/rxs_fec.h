/*

  rxs_fec
  ---------------

  rxs_fec handles FEC encoding & decoding RTP/VP8 packets
  for rxs_streamer through the use of Jerasure 2.0.

  https://bitbucket.org/jimplank/jerasure

  (Necessary files included within the extern directory)

  ----------------
  FEC header specification:
    https://tools.ietf.org/id/draft-galanos-fecframe-rtp-reedsolomon-02.txt

*/

#ifndef RXS_FEC_H
#define RXS_FEC_H

#include <rxs_streamer/rxs_packets.h>
#include <jerasure.h>
#include <rxs_streamer/rxs_types.h>

typedef struct rxs_fec rxs_fec;

struct rxs_fec {

  int mode;      /*0 for standalone, 1 for webRTC compatible mode*/
  int index;     /*index for packets into buffer/num frame packets in buffer*/
  uint8_t numCoding; /*Number of coded packets to output*/

  uint32_t ssrc;      /* our identifier */
  uint16_t seqnum;    /* current packet sequence number, increments for eaach packet */
  uint32_t timestamp; /* timestamp at the rate that vp8 wants */
  uint16_t SN_Base;   /*Lowest seqnum of any given batch of FEC packets for a frame*/
  uint16_t pkt_span;  /*Number of source packets for encoding*/

                 /*Jerasure coding requires... */
  char** data;   /* (1): data array containing the packets*/
  char** coding; /* (2): coding array for output of encoded packets*/
  unsigned char** buffer; /*pads packets to a multiple of 8 (1024) for matrix encoding*/
  uint8_t** FECbuf;

};

/*initialize and allocate necessary memory*/
int rxs_fec_init(rxs_fec* fec, int mode);

/*encode the packets in buffer*/
int rxs_fec_encode(rxs_fec* fec);

/*Add a packet to the FEC buffer for encoding*/
int rxs_fec_add_packet(rxs_fec* fec, uint8_t* buffer, uint32_t nbytes);

/*shut-down the buffer */
int rxs_fec_clear_buf(rxs_fec* fec);

/*Wrap the RTP/FEC header*/
int rxs_fec_wrap(rxs_fec* fec);

/*Unwrap the RTP/FEC header*/
int rxs_fec_unwrap(rxs_fec* fec, uint8_t* buffer, uint32_t nbytes);

/*reset data & coding arrays for Jerasure*/
int rxs_fec_reset_bufs(rxs_fec* fec);
#endif
