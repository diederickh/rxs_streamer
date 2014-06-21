#ifndef RXS_DECODER_H
#define RXS_DECODER_H

#define VPX_CODEC_DISABLE_COMPAT 1
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>
#include <stdint.h>
#define vpx_dx_interface (vpx_codec_vp8_dx())

typedef struct rxs_decoder rxs_decoder;
typedef void(*rxs_decoder_callback)(rxs_decoder* dec, vpx_image_t* img);

struct rxs_decoder {
  vpx_codec_ctx_t ctx;
  vpx_image_t* img;
  void* user;
  rxs_decoder_callback on_image;
};

int rxs_decoder_init(rxs_decoder* dec);
int rxs_decoder_clear(rxs_decoder* dec);
int rxs_decoder_decode(rxs_decoder* dec, uint8_t* buffer, uint32_t nbytes);

#endif
