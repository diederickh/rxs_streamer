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

#define WIDTH 320
#define HEIGHT 240
#define FPS 25

bool must_run = true;
video_generator vidgen;
video::EncoderVP8 vp8_encoder;
video::EncoderSettings vp8_encoder_settings;
rtp::WriterVP8 rtp_encoder;
rxs::Sender sender(128*1024, "127.0.0.1", 6677, "127.0.0.1", 6688);

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
  if (video_generator_init(&vidgen, WIDTH, HEIGHT, FPS) < 0) {
    printf("Error: cannot init video generator.\n");
    exit(1);
  }

  /* Initialize the encoder. */
  vp8_encoder_settings.width = WIDTH;
  vp8_encoder_settings.height = HEIGHT;
  vp8_encoder_settings.fps_num = 1;
  vp8_encoder_settings.fps_den = FPS;
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

    if (vidgen.frame % FPS == 0) {
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
    
  c->replace(pkt->payload, pkt->nbytes);

  sender.sendChunk(c);
}

static void sig(int s) {
  must_run = false;
  printf("Signalled!\n");
}

