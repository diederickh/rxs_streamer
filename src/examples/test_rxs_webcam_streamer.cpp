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
#  include <rxs_streamer/rxs_signaling.h>
#  include <libyuv.h>
#  include <uv.h>
}

#define USE_SIGNALING 0         /* when set to 1, we try to connect to a signalign server and retrieve an IP:PORT for a specific slot */
#define USE_IVF 0
#define DEVICE 1
#define WIDTH 640
#define HEIGHT 480
#define FPS 5

using namespace ca;

static void on_signal(int s);
static void on_webcam_frame(void* pixels, int nbytes, void* user);
static void on_vp8_packet(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);
static void on_rtp_packet(rxs_packetizer* vpx, uint8_t* buffer, uint32_t nbytes);
static void on_control_command(rxs_control_receiver* rec);
//static void on_slot_info(rxs_sigclient* client, uint32_t slot, char* ip, uint16_t port);
static void on_address(rxs_signal* s, char* ip, uint16_t port);

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
rxs_signal sig_sub; /* signal subscriber, used to get IP:PORT information when using signaling  */
int got_remote_ip = 0; /* when using signaling we wait to update the sender until we received the remote IP:PORT. */
int ndevices = 0;

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
  ndevices = cap->listDevices();
  if (ndevices <= 0) {
    printf("Error: no capture device found.\n");
    exit(1);
  }

  if (DEVICE > (ndevices - 1)) {
    printf("Error: invalid device ID, check #define DEVICE value.\n");
    exit(1);
  }
  

#if 0
  if (cap->listCapabilities(DEVICE) < 0) {
    printf("Error: couldn't list capabilities for device 0.\n");
    exit(1);
  }
#endif

  /* check if the capture device support the format we want */
  cap_fmt = CA_YUV420P;
  enc_fmt = VPX_IMG_FMT_I420;
  cap_capability = cap->findCapability(DEVICE, WIDTH, HEIGHT, cap_fmt);
  if (cap_capability < 0) {
    printf("Warning:: I420 not supported falling back to YUY2.\n");
    cap_fmt = CA_YUYV422;
    cap_fmt = CA_UYVY422; /* need this on Mac with C920 */
    cap_capability = cap->findCapability(DEVICE, WIDTH, HEIGHT, cap_fmt);

    //cap_capability = 161; /* there is something silly going on with findCapability and CA_YUYV422 */

    if (cap_capability < 0) {
      //cap->listCapabilities(0);
      printf("Error: no capture device found that is supports what we need.\n");
      exit(1);
    }
  }

  //cap->listCapabilities(DEVICE);
  printf("Using: %d\n", cap_capability);

  /* open the device using the capability we found.*/
  Settings settings;
  settings.device = DEVICE;
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

#if USE_SIGNALING

  //sigclient.on_slot_info = on_slot_info;

  //if (rxs_sigclient_init(&sigclient, "tcp://home.roxlu.com:5995") < 0) {
  if (rxs_signal_init(&sig_sub, "home.roxlu.com", 6379) < 0) {
    printf("Error: cannot initialize the sigclient.\n");
    exit(1);
  }

  if (rxs_signal_subscribe(&sig_sub, 5) < 0) {
    printf("Error: cannot subscribe to signal server.\n");
    exit(1);
  }

  sig_sub.on_address = on_address;

  /*
  if (rxs_sigclient_retrieve_address(&sigclient, 5) < 0) {
    printf("Error: failed to request client info from sigserv.\n");
    exit(1);
  }
  */
#else
  /* initialize our sender (network output) */
  /* @todo - we should be able to pass an domain too! */
  if (rxs_sender_init(&sender, "127.0.0.1", 6970) < 0) { 
  //if (rxs_sender_init(&sender, "76.123.235.77", 6970) < 0) { 
    printf("Error: cannot init the sender.\n");
    exit(1);
  }
#endif  

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

    
    rxs_control_receiver_update(&control_receiver);

#if USE_SIGNALING
    //rxs_sigclient_update(&sigclient);
    rxs_signal_update(&sig_sub);
    if (got_remote_ip == 1) {
      rxs_sender_update(&sender);
    }
#else 
   rxs_sender_update(&sender); 
#endif

  }

  if (cap) {
    delete cap;
    cap = NULL;
  }
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

#define USE_KEYFRAMES 1

#if USE_KEYFRAMES

   if (nframes % 25 == 0) {
     if (rxs_encoder_request_keyframe(&encoder) < 0) {
       printf("Error: cannot request a keyframe.\n");
     }
     else {
         printf("Requested a new keyframe.\n");
     }
   }

#endif   

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

#if USE_SIGNALING
  /* start sending once we know the IP:PORT of the receiver */
  if (got_remote_ip == 0) {
    return;
  }
#endif

#if 1
  if (rxs_sender_send(&sender, buffer, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  
#endif

  /* @todo -> we're sending everything 3 times ... simple congestion control */
#define USE_TEST_A 0
#define USE_TEST_B 0
#define USE_TEST_C 0

#if USE_TEST_A
  uint8_t* m0 = (uint8_t*)malloc(nbytes);
  memcpy(m0, buffer, nbytes);
  if (rxs_sender_send(&sender, m0, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  

  uint8_t* m1 = (uint8_t*)malloc(nbytes);
  memcpy(m1, buffer, nbytes);
  if (rxs_sender_send(&sender, m1, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  

  uint8_t* m2 = (uint8_t*)malloc(nbytes);
  memcpy(m2, buffer, nbytes);
  if (rxs_sender_send(&sender, m2, nbytes) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  
#endif

#if USE_TEST_B

  uint8_t* m0 = (uint8_t*)malloc(5);
  memcpy(m0, buffer, 5);
  if (rxs_sender_send(&sender, m0, 5) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  

  uint8_t* m1 = (uint8_t*)malloc(5);
  memcpy(m1, buffer, 5);
  if (rxs_sender_send(&sender, m1, 5) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  

  uint8_t* m2 = (uint8_t*)malloc(5);
  memcpy(m2, buffer, 5);
  if (rxs_sender_send(&sender, m2, 5) < 0) {
    printf("Error: cannot send rtp packet.\n");
  }  
#endif

#if USE_TEST_C

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

static void on_address(rxs_signal* s, char* ip, uint16_t port) {
  //static void on_slot_info(rxs_sigclient* client, uint32_t slot, char* ip, uint16_t port) {
  printf("Got remote IP: %s\n", ip);

  /* initialize the sender with the IP:PORT of the we received from the signaling server. */
  if (got_remote_ip == 0) {
    if (rxs_sender_init(&sender, ip, port) < 0) { 
      printf("Error: cannot init the sender.\n");
      exit(1);
    }
    got_remote_ip = 1;
  }
}
