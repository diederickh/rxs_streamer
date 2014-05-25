/*

  rxs_encoder
  -----------
  The rxs_encoder encoder raw I420 frames using VP8 and calls the set on_packet
  callback when it has some encoded data. At the moment of writing each packet 
  that is passed into the on_packet callback will contain one VP8 partition which 
  makes it easy to encapsulate in a RTP VP8 packet.

 */
#ifndef RXS_ENCODER_H
#define RXS_ENCODER_H

#define VPX_CODEC_DISABLE_COMPAT 1
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
#include <stdint.h>
#define vpx_cx_interface (vpx_codec_vp8_cx())

typedef struct rxs_encoder rxs_encoder;
typedef struct rxs_encoder_config rxs_encoder_config;
typedef void (*rxs_pkt_callback)(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);  /* the `on_packet` callback which is called when the encoder outputs an packet. */

struct rxs_encoder_config {
  uint32_t width;                       /* height of the image buffer you pass into rxs_encoder_encode, as I420 */
  uint32_t height;                      /* height of the image buffer you pass into rxs_encoder_encode, as I420 */
  int fps_num;                          /* FPS numerator, e.g. 1 */
  int fps_den;                          /* FPS denumerator, e.g. 25 for 25fps */
};

struct rxs_encoder {
  vpx_image_t img;
  vpx_codec_enc_cfg_t cfg;
  vpx_codec_ctx_t ctx;  
  int width;
  int height;
  int fps_num;
  int fps_den;
  unsigned long frame_duration;
  int flags;                            /* encoder flags, e.g. used by rxs_encoder_request_keyframe */

  /* callbacks */
  void* user;
  rxs_pkt_callback on_packet;
};

int rxs_encoder_init(rxs_encoder* enc, rxs_encoder_config* cfg);               /* initialize the encoder */
int rxs_encoder_encode(rxs_encoder* enc, unsigned char* yuv420, int64_t pts);  /* encode the given buffer */
int rxs_encoder_request_keyframe(rxs_encoder* enc);                            /* will ask the encoder to return a keyframe */

#endif
