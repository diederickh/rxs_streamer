/*

  rxs_ivf
  -------
  Based on http://www.webmproject.org/docs/vp8-sdk/example__simple__encoder.html the
  rxs_ivf code is used to store VPX frames into a very simple file format that can
  be used with avconv to mux it into a playable format. Created this for testing the 
  RTP-VP8 stream.

  You can use avconv to mux the ivf into a webm file:

  ````sh
  ./avconv -f ivf -i test.ivf -vcodec copy out.webm
  ````

  Example:
  
  ````C

      // create the ivf writer 
      if (rxs_ivf_init(&ivf) < 0) {
        printf("Error: cannot create the ivf.\n");
        exit(1);
      }
    
      ivf.width = WIDTH;
      ivf.height = HEIGHT;
      ivf.timebase_num = 1;
      ivf.timebase_den = FPS;
      
      if (rxs_ivf_create(&ivf, "output.ivf") < 0) {
        printf("Error: cannot create the ivf file.\n");
        exit(1);
      }

      // write frames!
      rxs_ivf_write_frame(&ivf, pts, pkt->data.frame.buf, pkt->data.frame.sz);

      // cleanup, close file
      rxs_ivf_destroy(&ivf);

  ````

 */
#ifndef ROXLU_RXS_IVF_H
#define ROXLU_RXS_IVF_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct rxs_ivf rxs_ivf;

struct rxs_ivf {
  /* generic */
  FILE* fp;

  /* ivf header info */
  uint16_t version;
  uint16_t width;
  uint16_t height; 
  uint32_t timebase_den;
  uint32_t timebase_num;
  uint64_t num_frames;
};

int rxs_ivf_init(rxs_ivf* k); /* initializes all members to initial values */
int rxs_ivf_create(rxs_ivf* k, const char* filename);
int rxs_ivf_write_header(rxs_ivf* k);
int rxs_ivf_write_frame(rxs_ivf* k, uint64_t timestamp, uint8_t* data, uint32_t len);
int rxs_ivf_write_u8(rxs_ivf* k, uint8_t data);
int rxs_ivf_write_u16(rxs_ivf* k, uint16_t data);
int rxs_ivf_write_u32(rxs_ivf* k, uint32_t data);
int rxs_ivf_write_u64(rxs_ivf* k, uint64_t data);
int rxs_ivf_write(rxs_ivf* k, uint8_t* data, uint32_t len);
int rxs_ivf_destroy(rxs_ivf* k);

int rxs_ivf_open(rxs_ivf* k);
int rxs_ivf_read_header(rxs_ivf* k);
int rxs_ivf_read_frame(rxs_ivf* k);
uint8_t rxs_ivf_read_u8(rxs_ivf* k);
uint16_t rxs_ivf_read_u16(rxs_ivf* k);
uint32_t rxs_ivf_read_u32(rxs_ivf* k);
uint64_t rxs_ivf_read_u64(rxs_ivf* k);

#endif
