#ifndef RXS_DEPACKETIZER_H
#define RXS_DEPACKETIZER_H

#include <stdint.h>

#define RXS_MAX_FRAME_SIZE (1024 * 1024) /* we expect that the payload data is not more then this */

typedef struct rxs_depacketizer rxs_depacketizer;

typedef void (*rxs_depacketizer_callback)(rxs_depacketizer* dep, uint8_t* buffer, uint32_t nbytes);

struct rxs_depacketizer {

  /* VP8: header info */
  uint8_t version;
  uint8_t padding;
  uint8_t extension;
  uint8_t csrc_count;
  uint8_t marker;                        /* Set for the very last packet of each encoded frame in line with the normal use of the M bit in video formats. For VP8 this will be set to 1 when the last packet for a frame is received. */
  uint8_t pt;                            /* Payload */
  uint16_t seqnum;                       /* Sequence number */
  uint32_t timestamp;
  uint32_t ssrc;

  /* VP8: require header */
  uint8_t X;                            /* extended controlbits present */
  uint8_t N;                            /* (non-reference frame)  when set to 1 this frame can be discarded */
  uint8_t S;                            /* start of VP8 partition */
  uint8_t PID;                          /* partition index */

  /* VP8: 2nd second row Payload Descriptor (is optional) */
  uint8_t I;                            /* 1 if PictureID is present */
  uint8_t L;                            /* 1 if TL0PICIDX is present */
  uint8_t T;                            /* 1 if TID is present */ 
  uint8_t K;                            /* 1 if KEYIDX is present */
  uint16_t PictureID;                    /* 8 or 16 bits, picture ID */
  uint8_t TL0PICIDX;                    /* 8 bits temporal level zero index */

  /* state, internal */
  int64_t len;                          /* number of bytes left in buf */
  uint32_t pos;                         /* number of bytes in `rxs_depacketizer.buffer` */
  uint8_t* buf;                         /* current read pointer */
  uint8_t buffer[RXS_MAX_FRAME_SIZE];
  uint8_t received_keyframe;            /* is set to 1 when we received a keyframe. only when we received a keyframe we will start calling the on_frame callback */

  /* callback */
  void* user;
  // rxs_depacketizer_callback on_frame;  /* gets called when we've decoded/extracted an complete frame from the stream */
  rxs_depacketizer_callback on_packet;  /* gets called when we've decoded a rtp vp8 packet */
};

int rxs_depacketizer_init(rxs_depacketizer* dep);
int rxs_depacketizer_reset(rxs_depacketizer* dep); /* just resets the members */
int rxs_depacketizer_unwrap(rxs_depacketizer* dep, uint8_t* buffer, int64_t nbytes);
void rxs_depacketizer_print(rxs_depacketizer* dep);

#endif
