/*
  
  test_webcam_streamer
  --------------------
  
  Uses libvideocapture to create a cross platform low latency 
  video streamer. This test is used while testing latency and
  is tested on linux and mac using a Logitech C920 webcam. 

 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <videocapture/Capture.h>

extern "C" {
#  include <rxs_streamer/rxs_encoder.h>
#  include <rxs_streamer/rxs_packetizer.h>
#  include <rxs_streamer/rxs_sender.h>
#  include <uv.h>
}

#define WIDTH 640
#define HEIGHT 480

using namespace ca;

static void on_signal(int s);
static void on_webcam_frame(void* pixels, int nbytes, void* user);
static void on_vp8_packet(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);
static void on_rtp_packet(rxs_packetizer* vpx, uint8_t* buffer, uint32_t nbytes);

uint64_t time_started = 0;
int64_t pts = 0;
int cap_capability;
int cap_fmt;
int enc_fmt;
Capture* cap;
rxs_encoder encoder;
rxs_packetizer packer;
rxs_sender sender;

int main() {
  printf("\n\ntest_video_streamer\n\n");

  signal(SIGINT, on_signal);

  /* create our capture instance. */
  cap = new Capture(on_webcam_frame, NULL);
  if (!cap) {
    printf("Cannot allocate the capture.\n");
    exit(1);
  }

  /* displays a list of available devices */
  if (cap->listDevices() <= 0) {
    printf("Error: no capture device found.\n");
    exit(1);
  }

#if 0
  if (cap->listCapabilities(0) < 0) {
    printf("Error: couldn't list capabilities for device 0.\n");
    exit(1);
  }
#endif

  /* check if the capture device support the format we want */
  cap_fmt = CA_YUV420P;
  enc_fmt = VPX_IMG_FMT_I420;
  cap_capability = cap->findCapability(0, WIDTH, HEIGHT, cap_fmt);
  if (cap_capability < 0) {
    printf("Warning:: I420 not supported falling back to YUY2.\n");
    cap_fmt = CA_YUYV422;
    enc_fmt = VPX_IMG_FMT_YV12;
    cap_capability = cap->findCapability(0, WIDTH, HEIGHT, cap_fmt);
    if (cap_capability < 0) {
      cap->listCapabilities(0);
      printf("Error: no capture device found that is supports what we need.\n");
      exit(1);
    }
  }

  /* open the device using the capability we found.*/
  Settings settings;
  settings.device = 0;
  settings.capability = cap_capability;
  settings.format = -1;
  if (cap->open(settings) < 0) {
    printf("Error: cannot open the capture device.\n");
    exit(1);
  }

  if (cap->start() < 0) {
    printf("Error: cannot start capture device.\n");
    exit(1);
  }

  /* Initialize the encoder */
  rxs_encoder_config cfg;
  cfg.width = WIDTH;
  cfg.height = HEIGHT;
  cfg.fps_num = 1;
  cfg.fps_den = 24; /* @todo get from cap */
  cfg.fmt = enc_fmt;

  if (rxs_encoder_init(&encoder, &cfg) < 0) {
    printf("Error: cannot initialize the encoder.\n");
    exit(1);
  }

  encoder.on_packet = on_vp8_packet;  

  /* initialize the packetizer */
  if (rxs_packetizer_init(&packer) < 0) {
    printf("Error: cannot init the packetizer.\n");
    exit(1);
  }
  packer.on_packet = on_rtp_packet;

  /* initialize our sender (network output) */
  if (rxs_sender_init(&sender, "0.0.0.0", 6970) < 0) {
    printf("Error: cannot init the sender.\n");
    exit(1);
  }

  while(1) { 
    cap->update();
    rxs_sender_update(&sender);
  }

  if (cap) {
    delete cap;
    cap = NULL;
  }
  printf("\n");
}


static void on_webcam_frame(void* pixels, int nbytes, void* user) {

  if (!time_started) {
    time_started = uv_hrtime();
  }
  
  pts = (uv_hrtime() - time_started) / (1000llu * 1000llu);

  rxs_encoder_encode(&encoder, (unsigned char*)pixels, pts);
}

static void on_vp8_packet(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts) {
  if (!enc) { return ; } 
  if (!pkt) { return ; } 

  if (rxs_packetizer_wrap(&packer, pkt) < 0) {
    printf("Error: packer failed.\n");
  }
}

static void on_rtp_packet(rxs_packetizer* vpx, uint8_t* buffer, uint32_t nbytes) {

  if (rxs_sender_send(&sender, buffer, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  
}

static void on_signal(int s) {
  
  printf("\n\nSIGNAL\n\n");

  if (cap) {
    printf("Closing device.\n");
    cap->stop();
    cap->close();
    delete cap;
    cap = NULL;
  }

  exit(0);
}
