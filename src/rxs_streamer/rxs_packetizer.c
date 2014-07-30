#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rxs_streamer/rxs_packetizer.h>

/* ----------------------------------------------------------------------------- */

static int packetizer_wrap_rtp(rxs_packetizer* vpx, const vpx_codec_cx_pkt_t* pkt);
static int packetizer_wrap_vp8(rxs_packetizer* vpx, const vpx_codec_cx_pkt_t* pkt);
static void packetizer_write_u16(rxs_packetizer* vpx, uint16_t val);
static void packetizer_write_u32(rxs_packetizer* vpx, uint32_t val);
static void packetizer_write(rxs_packetizer* vpx, uint8_t* data, uint32_t nbytes);
static int packetizer_calc_payload_size(rxs_packetizer* vpx);

/* ----------------------------------------------------------------------------- */

int rxs_packetizer_init(rxs_packetizer* vpx) {
  if (!vpx) { return -1; }

  if (rxs_packetizer_reset(vpx) < 0) {
    printf("Error: cannot reset the packetizer.\n");
    return -2;
  }

  srand(time(NULL));

  vpx->ssrc = rand();
  vpx->seqnum = rand();
  vpx->picture_id = rand();

  return 0;
}

int rxs_packetizer_reset(rxs_packetizer* vpx) {

  if (!vpx) { return -1; }

  /* fragmenting packets */
  vpx->frame_dx = 0;
  vpx->frame_len = 0;
  vpx->frame_size = 0;
  vpx->dx = 0;

  /* rtp header */
  vpx->version = 2;
  vpx->padding = 0;
  vpx->extension = 0;
  vpx->cc = 0;
  vpx->marker = 0;
  vpx->payload_type = 98;
  vpx->timestamp = 0;
  vpx->nonref = 0;

  /* vp8 rtp */
  vpx->pstart = 0;
  vpx->pid = 0;

  return 0;
}

int rxs_packetizer_wrap(rxs_packetizer* vpx, const vpx_codec_cx_pkt_t* pkt) {

  if (!vpx) { return -1; }
  if (!pkt) { return -2; }

  if (rxs_packetizer_reset(vpx) < 0) {
    return -3;
  }

  /* set correct values */
  vpx->frame_len = pkt->data.frame.sz;
  vpx->timestamp = pkt->data.frame.pts * 90; /* 90kHz per rfc */
  vpx->pstart = 1;
  vpx->extended = 1; /* add extension header for vp8 */
  vpx->nonref = (pkt->data.frame.flags & VPX_FRAME_IS_KEY) ? 0 : 1;

  printf("Timestamp: %d\n", vpx->timestamp);

  while(vpx->frame_len) {

    vpx->dx = 0;
    vpx->frame_size = packetizer_calc_payload_size(vpx);
    vpx->marker = (vpx->frame_len < RXS_RTP_PAYLOAD_SIZE)
                  && ((pkt->data.frame.flags & VPX_FRAME_IS_FRAGMENT) == 0);

    if (packetizer_wrap_rtp(vpx, pkt) < 0) {
      printf("Error: cannot wrap the vpx into a rtp packet.\n");
      return -3;
    }

    if (packetizer_wrap_vp8(vpx, pkt) < 0) {
      printf("Error: cannot wrap the vpx into a the vp8 payload.\n");
      return -4;
    }

    rxs_packetizer_print(vpx);

    /* tell the user that we have some data to be written */
    vpx->on_packet(vpx, vpx->buffer, vpx->dx);

    vpx->frame_dx += vpx->frame_size;
    vpx->frame_len -= vpx->frame_size;
    vpx->pstart = 0;                                    /* S bit, must be set to 1 when the first payload octect is the beginning of a new vp8 partition */
    vpx->seqnum++;                                      /* the sequence number needs to increment for each packet */

    if (vpx->marker == 1) {
      /* @todo see https://gist.github.com/roxlu/ceb1e8c95aff5ba60f45 of
               the webrtc api, which uses this to calculate the next
               picture id: picture_id_ = (picture_id_ + 1) & 0x7FFF;  // prepare next
      */
      vpx->picture_id++;
    }
  }

  return 0;
}

void rxs_packetizer_print(rxs_packetizer* vpx) {
  if (!vpx) { return ; }

  printf("#%d %u - V: %d, P: %d, X: %d, C: %d, M: %d, PT: %d, X: %d, N: %d, S: %d, PID: %d, bytes: %lld\n",
         vpx->seqnum,
         vpx->timestamp,
         vpx->version,
         vpx->padding,
         0,
         0,
         vpx->marker,
         vpx->payload_type,
         0,
         vpx->nonref,
         vpx->pstart,
         vpx->pid,
         vpx->frame_size
  );

}

/* ----------------------------------------------------------------------------- */

static int packetizer_calc_payload_size(rxs_packetizer* vpx) {
  if (vpx->frame_len >= RXS_RTP_PAYLOAD_SIZE) {
    return RXS_RTP_PAYLOAD_SIZE;
  }
  else {
    return vpx->frame_len;
  }
}

static int packetizer_wrap_rtp(rxs_packetizer* vpx, const vpx_codec_cx_pkt_t* pkt) {

  vpx->buffer[vpx->dx]  = (vpx->version   & 0x02) << 6;            /* RTP: version */
  vpx->buffer[vpx->dx] |= (vpx->padding   & 0x01) << 5;            /* RTP: has padding? */
  vpx->buffer[vpx->dx] |= (vpx->extension & 0x01) << 4;            /* RTP: has extension header */
  vpx->buffer[vpx->dx] |= (vpx->cc        & 0x0f) << 0;            /* RTP: CSRC count */
  vpx->dx++;

  vpx->buffer[vpx->dx]  = (vpx->marker       & 0x01) << 7;         /* RTP: marker bit, set for the last packet of an encoded frame */
  vpx->buffer[vpx->dx] |= (vpx->payload_type & 0x7f) << 0;         /* RTP: payload type */
  vpx->dx++;

  packetizer_write_u16(vpx, vpx->seqnum);                          /* RTP: sequence number */
  packetizer_write_u32(vpx, vpx->timestamp);                       /* RTP: timestamp */
  packetizer_write_u32(vpx, vpx->ssrc);                            /* RTP: identifier */

  return 0;
}

/*

  See: http://www.ffmpeg.org/doxygen/2.0/rtpenc__vp8_8c_source.html
       where they say the picture id is present in the first partition.

       Non-reference frames, @todo add
       // N bit: none reference frame, can be dropped
       if (pkt->data.frame.flags & VPX_FRAME_IS_DROPPABLE) {
         //vpx->buffer[vpx->dx] |= 0x20;
       }

 */

static int packetizer_wrap_vp8(rxs_packetizer* vpx, const vpx_codec_cx_pkt_t* pkt) {

  uint8_t* pid = NULL;

  /* @todo we don't really need to check here as we're checking in the calling function. */
  if (!vpx) { return -1; }
  if (!pkt) { return -2; }

  /* unset all (makes sure all R bits are set to 0) */
  vpx->buffer[vpx->dx]  = 0;
  vpx->buffer[vpx->dx]  = (vpx->extended                & 0x01) << 7;      /* VP8 RTP: extension bit set? */
  vpx->buffer[vpx->dx] |= (vpx->nonref                  & 0x01) << 5;      /* VP8 RTP: non reference frame,  0 = can be discarded, 1 = key/golden frame. */
  vpx->buffer[vpx->dx] |= (vpx->pstart                  & 0x01) << 4;      /* VP8 RTP: S bit, start of parition. */
  vpx->buffer[vpx->dx] |= (pkt->data.frame.partition_id & 0x07);           /* VP8 RTP: PID: parition index. */
  vpx->dx++;

  if (vpx->extended == 1) {
    /* @todo: have to test this a bit more */
    vpx->buffer[vpx->dx] |= 0x80; /* Extension header, picture ID present */
    vpx->dx++;

    /* picture id */
    pid = (uint8_t*)&vpx->picture_id;
    vpx->buffer[vpx->dx] = 0x80 | (pid[1] & 0x7f);
    vpx->dx++;

    vpx->buffer[vpx->dx] = pid[0];
    vpx->dx++;

    /* @todo: we need a better way to wrap */
    if (vpx->picture_id == 32767) {
      vpx->picture_id = 0;
    }
  }

  /* write the vp8 frame data */
  packetizer_write(vpx, pkt->data.frame.buf + vpx->frame_dx, vpx->frame_size);

  return 0;
}

static void packetizer_write_u16(rxs_packetizer* vpx, uint16_t val) {
  uint8_t* v = (uint8_t*)&val;
  vpx->buffer[vpx->dx++] = v[1];
  vpx->buffer[vpx->dx++] = v[0];
}

static void packetizer_write_u32(rxs_packetizer* vpx, uint32_t val) {
  uint8_t* v = (uint8_t*)&val;
  vpx->buffer[vpx->dx++] = v[3];
  vpx->buffer[vpx->dx++] = v[2];
  vpx->buffer[vpx->dx++] = v[1];
  vpx->buffer[vpx->dx++] = v[0];
}

static void packetizer_write(rxs_packetizer* vpx, uint8_t* data, uint32_t nbytes) {
  /* @todo - we should check buffer overflow here */
  memcpy((void*)vpx->buffer + vpx->dx, (const void*)data, nbytes);
  vpx->dx += nbytes;
}
