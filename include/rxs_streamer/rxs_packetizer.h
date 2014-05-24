#ifndef RXS_PACKETIZER_H
#define RXS_PACKETIZER_H

#include <stdint.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
#include <rxs_streamer/rxs_types.h>

#define RXS_PACK_BUFFER_SIZE (1024 * 1024)

typedef struct rxs_packetizer rxs_packetizer;
typedef void (*rxs_packetizer_callback)(rxs_packetizer* rxs, uint8_t* buffer, uint32_t nbytes); /* is called when we have a rtp packet ready. */

struct rxs_packetizer {
  
  /* RTP header */
  uint8_t version;                             /* the version, 2 */
  uint8_t padding;                             /* padding bit in rtp header */
  uint8_t extension;                           /* extension bit in rtp header */
  uint8_t cc;                                  /* number of CSRC identifiers */
  uint8_t marker;                              /* the rtp m bit, must be set for the last packet of a frame */
  uint8_t payload_type;                        /* payload type */
  uint32_t ssrc;                               /* our identifier */
  uint16_t seqnum;                             /* current packet sequence number, increments for eaach packet */
  uint32_t timestamp;                          /* timestamp at the rate that vp8 wants */

  /* VP8 RTP */
  uint8_t extended;                            /* should we add extension header */
  uint8_t pstart;                              /* start of vp8 partition */
  uint8_t nonref;
  uint8_t pid;                                 /* partition id */
  int16_t picture_id;                          /* PictureID field */
  
  uint32_t dx;                                 /* write index into rxs_packetiser.dx */
  uint32_t frame_dx;                           /* index into the vpx_codec_cx_pkt_t.data.frame.buf */
  int64_t frame_len;                           /* number of read bytes from the frame/vpx packet */
  int64_t frame_size;                          /* payload size for the current packet */
  
  uint8_t buffer[RXS_PACK_BUFFER_SIZE];        /* large buffer into which we store the current rtp packet */

  /* callback */
  void* user;
  rxs_packetizer_callback on_packet;
};

int rxs_packetizer_init(rxs_packetizer* vpx);
int rxs_packetizer_wrap(rxs_packetizer* vpx, const vpx_codec_cx_pkt_t* pkt);
int rxs_packetizer_reset(rxs_packetizer* vpx);  /* resets all members to defaults, but not the one which needs to be set (like ssrc)*/
void rxs_packetizer_print(rxs_packetizer* vpx); /* print some debug info */

#endif
