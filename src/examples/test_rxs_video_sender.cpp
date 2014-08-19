/*
  
  test_rxs_video_sender
  ---------------------

  Uses the libvideogenerator to create some test video and sends it using RTP/VP8.

 */
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <uv.h>
#include <streamer/Sender.h>
#include <video_generator.h>
#include <video/EncoderSettings.h>
#include <video/EncoderVP8.h>
#include <rtp/WriterVP8.h>
#include "config.h"

bool must_run = true;
video_generator vidgen;
video::EncoderVP8 vp8_encoder;
video::EncoderSettings vp8_encoder_settings;
rtp::WriterVP8 rtp_encoder;
rxs::Sender sender(128*1024, SENDER_IP, SENDER_PORT, RECEIVER_IP, RECEIVER_PORT);

static void sig(int s);
static void on_vp8_packet(video::EncoderVP8* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);
static void on_rtp_packet(rtp::PacketVP8* pkt, void* user);

int main() {

  signal(SIGINT, sig);

  /* Initialize the sender */
  if (0 != sender.init()) {
    printf("Error: cannot init the video sender.\n");
    exit(1);
  }

  /* Initialize the fake video generator. */
  if (video_generator_init(&vidgen, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FPS_DEN) < 0) {
    printf("Error: cannot init video generator.\n");
    exit(1);
  }

  /* Initialize the encoder. */
  vp8_encoder_settings.width = VIDEO_WIDTH;
  vp8_encoder_settings.height = VIDEO_HEIGHT;
  vp8_encoder_settings.fps_num = VIDEO_FPS_NUM;
  vp8_encoder_settings.fps_den = VIDEO_FPS_DEN;
  if (vp8_encoder.init(vp8_encoder_settings) < 0) {
    printf("Error: cannot init the video encoder.\n");
    exit(1);
  }

  /* Set callbacks */
  vp8_encoder.on_packet = on_vp8_packet;
  rtp_encoder.on_packet = on_rtp_packet;

  /* Generate and encode the video. */
  uint64_t time_started = uv_hrtime();
  while (must_run) {

    if (vidgen.frame % VIDEO_FPS_DEN == 0) {
      vp8_encoder.flags = VPX_EFLAG_FORCE_KF;
    }

    video_generator_update(&vidgen);

    vp8_encoder.encode(vidgen.y, vidgen.strides[0], 
                       vidgen.u, vidgen.strides[1],
                       vidgen.v, vidgen.strides[2], 
                       vidgen.frame);

    usleep(vidgen.fps);
  }
  
  return 0;
}

static void on_vp8_packet(video::EncoderVP8* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts) {
  rtp_encoder.packetize(pkt);
}

static void on_rtp_packet(rtp::PacketVP8* pkt, void* user) {

  if (NULL == pkt) { 
    return;
  }

  rxs::Chunk* c = sender.getFreeChunk();
  if (NULL == c) {
    printf("Error: cannot get a free chunk from the sender.\n");
    return;
  }

  /* Check if the chunk is big enough for our video data. */
  if (c->capacity() < pkt->nbytes) {
    printf("Error: the chunk size is not big enough to hold a video frame; make it bigger: %lu < %u.\n", c->capacity(), pkt->nbytes);
    sender.freeChunk(c);
    return;
  }

  printf("Sending packet: %d, nbytes: %d\n", pkt->sequence_number, pkt->nbytes);
  c->replace(pkt->payload, pkt->nbytes);

  sender.sendChunk(c);
}

static void sig(int s) {
  must_run = false;
  printf("Signalled!\n");
}

