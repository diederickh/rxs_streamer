#include <stdlib.h>
#include <stdio.h>
#include <rxs_streamer/rxs_encoder.h>

/* ----------------------------------------------------------------------------- */

void print_vpx_packet(const vpx_codec_cx_pkt_t* pkt);

/* ----------------------------------------------------------------------------- */

int rxs_encoder_init(rxs_encoder* enc, rxs_encoder_config* cfg) {

  vpx_codec_err_t err;
  int flags = VPX_CODEC_CAP_OUTPUT_PARTITION; 

  /* validate */
  if (!enc) { return -1; } 
  if (!cfg) { return -2; } 
  if (!cfg->width) { return -3; } 
  if (!cfg->height) { return -4; } 

  /* initialize the codec */
  err = vpx_codec_enc_config_default(vpx_cx_interface, &enc->cfg, 0);
  if (err) {
    printf("Error: failed to setup vpx encoder. %s\n", vpx_codec_err_to_string(err));
    return -5;
  }

  /* update config */
  enc->cfg.rc_target_bitrate = 1500;
  enc->cfg.g_w = cfg->width;
  enc->cfg.g_h = cfg->height;
  enc->cfg.g_timebase.num = 1;
  enc->cfg.g_timebase.den = (int) 1000;

  /* @todo - copied these settings from: https://github.com/3XX0/rtvs/blob/master/encode.c , have to verify/test those */
  enc->cfg.g_pass = VPX_RC_ONE_PASS;
  enc->cfg.g_error_resilient = 1;
  enc->cfg.kf_mode = VPX_KF_AUTO;
  enc->cfg.g_lag_in_frames = 0;
  enc->cfg.rc_dropframe_thresh = 1;
  enc->cfg.rc_end_usage = VPX_CBR;

#if 0
  enc->cfg.kf_mode = VPX_KF_AUTO;
  enc->cfg.g_lag_in_frames = 0;
  enc->cfg.rc_dropframe_thresh = 1;
  enc->cfg.rc_end_usage = VPX_CBR;
  enc->cfg.rc_buf_sz = 6000;
  enc->cfg.rc_buf_initial_sz = 4000;
  enc->cfg.rc_buf_optimal_sz = 5000;
#endif

  /* init codec */
  if (vpx_codec_enc_init(&enc->ctx, vpx_cx_interface, &enc->cfg, flags)) {
    printf("Error: could not initialize the vpx encoder.\n");
    return -6;
  }

  /* copy some info from the config that we need */
  enc->width = cfg->width;
  enc->height = cfg->height;
  enc->fps_num = cfg->fps_num;
  enc->fps_den = cfg->fps_den;

  /* @todo - is this correct? */
  enc->frame_duration = ((double) 1.0 / cfg->fps_den) / ((double) enc->cfg.g_timebase.num / enc->cfg.g_timebase.den);

  return 0;
}


int rxs_encoder_encode(rxs_encoder* enc, unsigned char* yuv420, int64_t pts) {
  
  vpx_codec_err_t err;
  vpx_image_t* img = NULL;
  vpx_codec_iter_t iter = NULL;
  const vpx_codec_cx_pkt_t* pkt;

  if (!enc) { return -1; } 
  if (!yuv420) { return -2; } 
  
  img = vpx_img_wrap(&enc->img, VPX_IMG_FMT_I420, enc->width, enc->height, 1, yuv420);
  if (!img) {
    printf("Error: cannot wrap the image.\n");
    return -4;
  }

  err = vpx_codec_encode(&enc->ctx, img, pts, enc->frame_duration, 0, VPX_DL_REALTIME);
  if (err) {
    printf("Error: while encoding.\n");
    return -5;
  }
  
  while ( (pkt = vpx_codec_get_cx_data(&enc->ctx, &iter)) ) {
    if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
      enc->on_packet(enc, pkt, pts);
    }
  }

  return 0;
}

/* ----------------------------------------------------------------------------- */

void print_vpx_packet(const vpx_codec_cx_pkt_t* pkt) {

  if (!pkt) { return ; } 
  
  if (pkt->data.frame.partition_id == 0) {
    printf("-------\n");
  }

  printf("pkt.frame.sz: %zu\n", pkt->data.frame.sz);
  printf("pkt.frame.pts: %lld\n", pkt->data.frame.pts);
  printf("pkt.frame.duration: %lu\n", pkt->data.frame.duration);
  printf("pkt.frame.flags: %d\n", pkt->data.frame.flags);
  printf("pkt.frame.partition_id: %d\n", pkt->data.frame.partition_id);
  printf("pkt.frame.flags (VPX_FRAME_IS_FRAGMENT): %d\n", pkt->data.frame.flags & VPX_FRAME_IS_FRAGMENT);
  printf("+\n");
}

/* ----------------------------------------------------------------------------- */
