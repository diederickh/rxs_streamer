#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <rxs_streamer/rxs_decoder.h>

int rxs_decoder_init(rxs_decoder* dec) {

  vpx_codec_err_t err;

  if (!dec) { return -1; } 

  err = vpx_codec_dec_init(&dec->ctx, vpx_dx_interface, NULL, 0);
  if (err) {
    printf("Error: cannot initialize the decoder: %s.\n", vpx_codec_err_to_string(err));
    return -2; 
  }

  dec->img = NULL;

  return 0;
}

/*
 
  Decode the given data. Note that on linux libvpx 
  will crash at this moment when the first partition 
  you pass into this function is not a key frame. 

 */
int rxs_decoder_decode(rxs_decoder* dec, uint8_t* buffer, uint32_t nbytes) {

  vpx_codec_iter_t iter = NULL;
  vpx_codec_err_t err;

  if (!dec) { return -1; } 
  if (!buffer) { return -2; } 
  if (!nbytes) { return -3; } 

  dec->img = NULL;

  err = vpx_codec_decode(&dec->ctx, buffer, nbytes, NULL, 0);
  if (err) {
    printf("Error: cannot decode buffer: %s\n", vpx_codec_err_to_string(err));
    return -4;
  }

  while ( (dec->img = vpx_codec_get_frame(&dec->ctx, &iter)) ) {
    dec->on_image(dec, dec->img);
  }

  return 0;
}

int rxs_decoder_clear(rxs_decoder* dec) {

  if (!dec) { return -1; } 

  if (vpx_codec_destroy(&dec->ctx)) {
    printf("Error: cannot destroy the decoder.\n");
    return -2; 
  }

  return 0;
}
