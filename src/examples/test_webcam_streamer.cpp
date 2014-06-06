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
#include <string.h>
#include <videocapture/Capture.h>

extern "C" {
#  include <rxs_streamer/rxs_encoder.h>
#  include <rxs_streamer/rxs_packetizer.h>
#  include <rxs_streamer/rxs_sender.h>
#  include <rxs_streamer/rxs_ivf.h>
#  include <rxs_streamer/rxs_sender.h>
#  include <rxs_streamer/rxs_control.h>
#  include <libyuv.h>
#  include <uv.h>
}

#define USE_IVF 0
#define WIDTH 640
#define HEIGHT 480
#define FPS 30

using namespace ca;

static void on_signal(int s);
static void on_webcam_frame(void* pixels, int nbytes, void* user);
static void on_vp8_packet(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);
static void on_rtp_packet(rxs_packetizer* vpx, uint8_t* buffer, uint32_t nbytes);
static void on_control_command(rxs_control_receiver* rec);

char* yuv420p;
char* yuv_y; /* points into yuv420p */
char* yuv_u; /* points into yuv420p */
char* yuv_v; /* points into yuv420p */


uint64_t total_bytes = 0;
uint64_t force_keyframe = 0; /* force a keyframe every `force_keyframe`, set to 0 when you don't want this */
uint64_t time_started = 0;
int64_t pts = 0;
int cap_capability;
int cap_fmt;
int enc_fmt;
Capture* cap;
rxs_encoder encoder;
rxs_packetizer packer;
rxs_sender sender;
rxs_ivf ivf;
rxs_control_receiver control_receiver; /* used to request handle keyframe requests */

int main() {
  printf("\n\ntest_video_streamer\n\n");

  signal(SIGINT, on_signal);

  /* buffer to convert input video to I420 (if necessary) */
  yuv420p = (char*)malloc(WIDTH * HEIGHT * 2);
  if (!yuv420p) {
    printf("Error: cannot allocate yuv420p buffer. Out of mem?\n");
    exit(1);
  }
  yuv_y = yuv420p;
  yuv_u = yuv_y + (WIDTH * HEIGHT);
  yuv_v = yuv_u + (WIDTH / 2) * (HEIGHT / 2);

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
    cap_fmt = CA_UYVY422;
    cap_capability = cap->findCapability(0, WIDTH, HEIGHT, cap_fmt);
    if (cap_capability < 0) {
      cap->listCapabilities(0);
      printf("Error: no capture device found that is supports what we need.\n");
      exit(1);
    }
  }

  cap->listCapabilities(0);
  printf("Using: %d\n", cap_capability);

  /* open the device using the capability we found.*/
  Settings settings;
  settings.device = 0;
  settings.capability = cap_capability;
  settings.format = -1; // CA_YUYV422;
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
  cfg.fps_den = FPS; /* @todo get from cap */
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
  //if (rxs_sender_init(&sender, "0.0.0.0", 6970) < 0) {
  //if (rxs_sender_init(&sender, "192.168.0.194", 6970) < 0) { 
  if (rxs_sender_init(&sender, "127.0.0.1", 6970) < 0) { 
  //if (rxs_sender_init(&sender, "192.168.0.190", 6970) < 0) {  // laptop
    printf("Error: cannot init the sender.\n");
    exit(1);
  }

#if USE_IVF
  /* create our ivf muxer */
  if (rxs_ivf_init(&ivf) < 0) {
    printf("Error: cannot create ivf.\n");
    exit(1);
  }

  ivf.width = WIDTH;
  ivf.height = HEIGHT;
  ivf.timebase_num = 1;
  ivf.timebase_den = FPS;

  if (rxs_ivf_create(&ivf, "webcam.ivf") < 0) {
    printf("Error: cannot open the ivf.\n");
    exit(1);
  }
#endif

  /* init control back channel */
  if (rxs_control_receiver_init(&control_receiver, RXS_CONTROL_PORT) < 0) {
    printf("Error: cannot create the control receiver.\n");
    exit(1);
  }

  control_receiver.on_command = on_control_command;

  time_started = uv_hrtime();

  while(1) { 
    cap->update();
    rxs_sender_update(&sender);
    rxs_control_receiver_update(&control_receiver);
  }

  if (cap) {
    delete cap;
    cap = NULL;
  }
  printf("\n");
}


static void on_webcam_frame(void* pixels, int nbytes, void* user) {

  static uint64_t nframes = 0;
  
  nframes++;
  if (force_keyframe && force_keyframe % nframes == 0) {
    rxs_encoder_request_keyframe(&encoder);
  }

  int r = 0;

  if (!time_started) {
    time_started = uv_hrtime();
  }
  
  pts = (uv_hrtime() - time_started) / (1000llu * 1000llu);

  if (cap_fmt == CA_YUYV422) {
    
    r = libyuv::YUY2ToI420((const uint8*)pixels,  WIDTH * 2, 
                           (uint8*)yuv_y, WIDTH, 
                           (uint8*)yuv_u, WIDTH / 2, 
                           (uint8*)yuv_v, WIDTH / 2, 
                           WIDTH, HEIGHT);

    if (r != 0) {
      printf("Error: cannot convert to I420.\n");
      exit(1);
    }

    rxs_encoder_encode(&encoder, (unsigned char*)yuv_y, pts);
  }
  else if (cap_fmt == CA_UYVY422) {
    r = libyuv::UYVYToI420((const uint8*)pixels,  WIDTH * 2, 
                           (uint8*)yuv_y, WIDTH, 
                           (uint8*)yuv_u, WIDTH / 2, 
                           (uint8*)yuv_v, WIDTH / 2, 
                           WIDTH, HEIGHT);
    if (r != 0) {
      printf("Error: cannot convert to I420.\n");
      exit(1);
    }

    rxs_encoder_encode(&encoder, (unsigned char*)yuv_y, pts);
  }
  else {
    rxs_encoder_encode(&encoder, (unsigned char*)pixels, pts);
  }


}

static void on_vp8_packet(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts) {
  if (!enc) { return ; } 
  if (!pkt) { return ; } 

#if USE_IVF
  /* we need to collect data because the encoder can give us fragmented output */
  static uint64_t nframes = 0;
  static uint8_t frame_buffer[1024 * 1024 * 3]; // just some arbitrary  big buffer
  static int pos = 0;

  memcpy(frame_buffer + pos, pkt->data.frame.buf, pkt->data.frame.sz);
  pos += pkt->data.frame.sz;

  if ((pkt->data.frame.flags & VPX_FRAME_IS_FRAGMENT) == 0) {
    if (rxs_ivf_write_frame(&ivf, nframes, (uint8_t*)frame_buffer, pos) < 0) {
      printf("Error: cannot write ivf frame.\n");
      exit(1);
    }
    pos = 0;
  }

  nframes++;
#endif

  if (rxs_packetizer_wrap(&packer, pkt) < 0) {
    printf("Error: packer failed.\n");
  }
}

static void on_rtp_packet(rxs_packetizer* vpx, uint8_t* buffer, uint32_t nbytes) {
  uint64_t dt = (uv_hrtime() - time_started) / (1000llu * 1000llu);
  uint64_t bitrate = 0;
  total_bytes += (nbytes * 4); /* @todo, note were sending everyting 4 times to make handle dropped packets */
  bitrate = total_bytes / dt;
  printf("Bitrate: %llu\n", bitrate);
  
  if (rxs_sender_send(&sender, buffer, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  

  /* @todo -> we're sending everything 3 times ... simple congestion control */
#if 0
  if (rxs_sender_send(&sender, buffer, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  

  if (rxs_sender_send(&sender, buffer, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  
  if (rxs_sender_send(&sender, buffer, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  
#endif
}

static void on_control_command(rxs_control_receiver* rec) {
  if (rec->command == RXS_CONTROL_COMMAND_KEY_FRAME) {
    printf("------------- key frame request -------------------- \n");
    rxs_encoder_request_keyframe(&encoder);
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

#if USE_IVF
  if (rxs_ivf_destroy(&ivf) < 0) {
    printf("Error: couldn't close the ivf correctly.\n");
  }
#endif

  exit(0);
}

