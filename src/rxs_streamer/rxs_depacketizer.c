#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rxs_streamer/rxs_depacketizer.h>

/* ----------------------------------------------------------------------------- */

static int depacketizer_unwrap_rtp(rxs_depacketizer* dep);
static int depacketizer_unwrap_vp8(rxs_depacketizer* dep);
static uint16_t depacketizer_read_u16(uint8_t* buf);
static uint32_t depacketizer_read_u32(uint8_t* buf);
static uint16_t depacketizer_read_picture_id(uint8_t* buf);

/* ----------------------------------------------------------------------------- */

int rxs_depacketizer_init(rxs_depacketizer* dep) {

  if (!dep) { return -1; } 

  dep->pos = 0;
  dep->received_keyframe = 0;

  return rxs_depacketizer_reset(dep);
}

int rxs_depacketizer_reset(rxs_depacketizer* dep) {

  if (!dep) { return -1; } 

  dep->version = 0;
  dep->padding = 0;
  dep->extension = 0;
  dep->csrc_count = 0;
  dep->marker = 0;
  dep->pt = 0;
  dep->seqnum = 0;
  dep->timestamp = 0;
  dep->ssrc = 0;

  dep->X = 0;
  dep->N = 0;
  dep->S = 0;
  dep->PID = 0;

  dep->I = 0;
  dep->L = 0;
  dep->T = 0;
  dep->K = 0;
  dep->PictureID = 0;
  dep->TL0PICIDX = 0;

  dep->pos = 0;
  dep->len = 0;
  dep->buf = NULL;

  return 0;
}

int rxs_depacketizer_unwrap(rxs_depacketizer* dep, uint8_t* buffer, int64_t nbytes) {

  if (!dep) { return -1; } 
  if (!buffer) { return -2; } 
  if (!nbytes) { return -3; } 

  if (!rxs_depacketizer_reset(dep) < 0) {
    printf("Error: cannot reset packetizer.\n");
    return -4;
  }

  dep->len = nbytes;
  dep->buf = buffer;

  if (depacketizer_unwrap_rtp(dep) < 0) {
    printf("Error: cannot unwrap rtp header.\n");
    return -5;
  }

  if (depacketizer_unwrap_vp8(dep) < 0) {
    printf("Error: cannot unwrap vp8 header.\n");
    return -6;
  }

  return 0;
}

/* ----------------------------------------------------------------------------- */
/*
  depacketizer unwrap rtp needs to check buffer size. 

*/

int depacketizer_unwrap_rtp(rxs_depacketizer* dep) {

  /* Parse RTP header */
  if (dep->len < 12) {
    printf("Error: invalid number of bytes in rtp packet.\n");
    return -5;
  }

  /* first bytes, version padding, extension, .. */
  dep->version    = (dep->buf[0] & 0xC0) >> 6;
  dep->padding    = (dep->buf[0] & 0x20) >> 4;
  dep->extension  = (dep->buf[0] & 0x10) >> 3;
  dep->csrc_count = (dep->buf[0] & 0x0F);
  dep->buf++;
  dep->len--;
  
  /* marker and payload type */
  dep->marker = (dep->buf[0] & 0x80) >> 7;
  dep->pt     = (dep->buf[0] & 0x7F);
  dep->len--;
  dep->buf++;

  /* sequence type */
  dep->seqnum = depacketizer_read_u16(dep->buf);
  dep->buf += 2;
  dep->len -= 2;

  dep->timestamp = depacketizer_read_u32(dep->buf);
  dep->buf += 4;
  dep->len -= 4;

  dep->ssrc = depacketizer_read_u32(dep->buf);
  dep->buf += 4;
  dep->len -= 4;

  /* @todo read csrc counts. */
  if (dep->csrc_count != 0) {
    printf("Error: need to implement csrc elements in depacketizer.\n");
    exit(1);
  }

#if 0  
  printf("<< version: %d, padding: %d, extension; %d, csrc: %d, marker: %d, payload: %d, "
         "seqnum: %d, timestamp: %d, ssrc: %04X | ",
         dep->version, 
         dep->padding, 
         dep->extension, 
         dep->csrc_count, 
         dep->marker,
         dep->pt,
         dep->seqnum,
         dep->timestamp,
         dep->ssrc
  );
#endif

  return 0;
}

/* @todo vp8 unwrapping needs to check lengths */
int depacketizer_unwrap_vp8(rxs_depacketizer* dep) {

  if (!dep) { return -1; } 

  /* VP8-Payload-Descriptor obligatory header*/
  dep->X = (dep->buf[0] & 0x80) >> 7;                     /* Extended control bits present */
  dep->N = (dep->buf[0] & 0x20) >> 5;                     /* None reference frame. (if 1, we can discard this frame). */
  dep->S = (dep->buf[0] & 0x10) >> 4;                     /* Start of VP8 partition */
  dep->PID = (dep->buf[0] & 0x07);                        /* Partition index */
  dep->buf++;
  dep->len--;

  /*  X: |I|L|T|K| RSV  | (OPTIONAL)  */
  if (dep->X == 1) {
    dep->I = (dep->buf[0] & 0x80) >> 7;   /* PictureID present */
    dep->L = (dep->buf[0] & 0x40) >> 6;   /* TL0PICIDX present */
    dep->T = (dep->buf[0] & 0x20) >> 5;   /* TID present */
    dep->K = (dep->buf[0] & 0x10) >> 4;   /* KEYIDX present */
    dep->buf++;
    dep->len--;
  }

  if (dep->I) {
    if (dep->buf[0] & 0x80) {  /* M, if M == 1, the picture ID takes 16 bits <-- is that right? */
      /* @todo - see https://github.com/alfredh/baresip/blob/98bf08bdcf2edd9d397f32650a8bfe62186fbecf/modules/vpx/decode.c how to decode the pic id correctly! */
      dep->PictureID = depacketizer_read_picture_id(dep->buf);
      dep->buf += 2;
      dep->len -=2;
    }
    else {
      dep->buf++;
      dep->len--;
    }
  }

  if(dep->L) {
    dep->buf++;
    dep->len--;
  }

  if(dep->T || dep->K) {
    dep->buf++;
    dep->len--;
  }

  if(dep->S == 1 && dep->PID == 0) {
    if((dep->buf[0] & 0x01) == 0) {
      //printf("+ We received a keyframe.\n");
      //  exit(0);
    }
  }

  if (dep->received_keyframe == 0 && dep->N == 0) {
    dep->received_keyframe = 1;
  }

  // rxs_depacketizer_print(dep);

  if (dep->len > 0) {
    /* @todo check out of range */
    memcpy(dep->buffer + dep->pos, dep->buf, dep->len);
    dep->pos += dep->len;
  }

  /* @todo:  we added a received_keyframe flag because libvpx crashes
             on linux when you give it any data before a first keyframe.
             though this kind of logic should be handled by the user, 
             not in here. 
  */

  if (dep->on_packet) {
    dep->on_packet(dep, dep->buffer, dep->len);
  }
            
  /*
  if (dep->marker == 1) {
    if (dep->received_keyframe) {
      dep->on_frame(dep, dep->buffer, dep->pos);
    }
    else {
      printf("Skipping data.\n");
    }
    dep->pos = 0;
  }
  */

  if (dep->marker == 1) {
    dep->pos = 0;
  }

  return 0;
}

void rxs_depacketizer_print(rxs_depacketizer* dep) {
  if (!dep) { return ; } 

  printf(" X: %d, N: %d, S: %d, PID: %d "
         "I: %d, L: %d, T: %d, K: %d, PictureID: %d, seqnum: %d\n", 
         dep->X,
         dep->N,
         dep->S,
         dep->PID,
         dep->I,
         dep->L,
         dep->T,
         dep->K,
         dep->PictureID,
         dep->seqnum
  );
}

/* @todo - verify if the picture id reading is correct 
   Check: https://github.com/alfredh/baresip/blob/98bf08bdcf2edd9d397f32650a8bfe62186fbecf/modules/vpx/decode.c
   this seems to be correct for 2 byte encoded pic 
*/

static uint16_t depacketizer_read_picture_id(uint8_t* buf) {
  uint16_t r = 0;
  r = (buf[0] & 0x7f) << 8;
  r += buf[1] ;
  return r;
}

static uint16_t depacketizer_read_u16(uint8_t* buf) {
  uint16_t result; 
  uint8_t* dest = (uint8_t*)&result;
  dest[0] = buf[1];
  dest[1] = buf[0];
  return result;
}

static uint32_t depacketizer_read_u32(uint8_t* buf) {
  uint32_t result; 
  uint8_t* dest = (uint8_t*)&result;
  dest[0] = buf[3];
  dest[1] = buf[2];
  dest[2] = buf[1];
  dest[3] = buf[0];
  return result;
}
