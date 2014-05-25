/*

  test_vpx
  --------
  Encoding raw yuv420 frame that are generated with the `rxs_generator` and 
  then written into a .ivf file.


  References:
  -----------
  - old version with raw yuv output: https://gist.github.com/roxlu/6113efe0ce63dffe584a

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <uv.h>
#include <rxs_streamer/rxs_generator.h>
#include <rxs_streamer/rxs_encoder.h>
#include <rxs_streamer/rxs_decoder.h>
#include <rxs_streamer/rxs_packetizer.h>
#include <rxs_streamer/rxs_depacketizer.h>
#include <rxs_streamer/rxs_packets.h>
#include <rxs_streamer/rxs_ivf.h>
#include <rxs_streamer/rxs_sender.h>
#include <rxs_streamer/rxs_receiver.h>
#include <rxs_streamer/rxs_control.h>

#define USE_RECEIVER 0
#define USE_DROPPING 1                    /* when set to 1 we will fake dropping of frames every N-th frame. */
#define WRITE_IVF 0                       /* when set to 1, we will write the depacketized rtp stream into a .ivf file */
#define WIDTH 640
#define HEIGHT 480
#define FPS 20

rxs_ivf ivf;
rxs_encoder encoder;
rxs_decoder decoder;
rxs_packetizer pack;
rxs_depacketizer depack;
rxs_sender sender;
rxs_packets packets;                     /* used to keep track of the last n-packets, can be used to resend */
rxs_control_receiver control_receiver;   /* handles incoming control messages, works together with the packets */

#if USE_RECEIVER
  rxs_receiver receiver;
#endif

#if USE_DROPPING
uint64_t frame_count= 0;
uint64_t frame_drop = 10; /* drop every `frame_drop` frame */
#endif

uint64_t frame_timeout = 0;
uint64_t frame_delay = 0;

static void sigh(int sn);
static void on_vp8_packet(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);
static void on_rtp_packet(rxs_packetizer* vpx, uint8_t* buffer, uint32_t nbytes);
static void on_vp8_depacket(rxs_depacketizer* dep, uint8_t* buffer, uint32_t nbytes);   /* gets called when we've collected a complete frame from the packetized rtp vp8 stream */
static void on_data(rxs_receiver* rec, uint8_t* buffer, uint32_t nbytes);
static void on_command(rxs_control_receiver* rec); /* gets called when we received a command on the control channel. */

int main() {

  printf("\n\ntest_vpx\n\n");

  signal(SIGINT, sigh);

  frame_delay = ((double)(1.0/FPS)) * 1000llu * 1000llu * 1000llu;
  frame_timeout = uv_hrtime() + frame_delay;

  /* create the ivf writer */
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

  /* create encoder */
  rxs_encoder_config cfg;
  cfg.width = WIDTH;
  cfg.height = HEIGHT;
  cfg.fps_num = 1;
  cfg.fps_den = FPS;

  encoder.on_packet = on_vp8_packet;

  if (rxs_encoder_init(&encoder, &cfg) < 0) {
    printf("Error: canot init encoder.\n");
    exit(1);
  }

  /* create decoder. */
  if (rxs_decoder_init(&decoder) < 0) {
    printf("Error: cannot init decoder.\n");
    exit(1);
  }

  /* packetizer */
  if (rxs_packetizer_init(&pack) < 0) {
    printf("Error: cannot init packetizer.\n");
    exit(1);
  }
  pack.on_packet = on_rtp_packet;

  /* depacketizer, validate stream */
  if (rxs_depacketizer_init(&depack) < 0) {
    printf("Error: cannot init the depacketizer.\n");
    exit(1);
  }

  depack.on_packet = on_vp8_depacket;

  /* generator */
  rxs_generator gen;
  if (rxs_generator_init(&gen, WIDTH, HEIGHT, FPS) < 0) {
    printf("Error: cannot initialize the generator.\n");
    exit(1);
  }

  /* networking */
  if (rxs_sender_init(&sender, "0.0.0.0", 6970) < 0) {
  //if (rxs_sender_init(&sender, "192.168.0.230", 6970) < 0) {
    // if (rxs_sender_init(&sender, "192.168.0.194", 6970) < 0) {
    printf("Error: cannot initialize networking.\n");
    exit(1);
  }

#if USE_RECEIVER
  if (rxs_receiver_init(&receiver, 6970)) {
    printf("Error: cannot initialize the receiver.\n");
    exit(1);
  }
  receiver.on_data = on_data;
#endif

  /* history of packets so we can resend them */
  if (rxs_packets_init(&packets, 50, RXS_RTP_PAYLOAD_SIZE + 256) < 0) { /* we add 256 for the RTP and RTP-VP8 payload header; this is more then enough ;-) */
    printf("Error: cannto initialize the packets history buffer.\n");
    exit(1);
  }

  /* control command handler. */
  printf("+++\n");
  if (rxs_control_receiver_init(&control_receiver, RXS_CONTROL_PORT) < 0) {
    printf("Error: cannot init control receiver.\n");
    exit(1);
  }
  control_receiver.on_command = on_command;
  printf("+++\n");

  uint64_t ds = uv_hrtime();

  while(1) {
    uint64_t now = uv_hrtime();
    if (now >= frame_timeout) {
      rxs_generator_update(&gen);

      uint64_t currtime = uv_hrtime() / (1000llu * 1000llu * 10llu);
      uint64_t dt = (uv_hrtime() - ds)  / (1000llu * 1000llu);

#if WRITE_IVF
      if (rxs_encoder_encode(&encoder, (unsigned char*)gen.y, gen.frame) < 0) {
#else
      if (rxs_encoder_encode(&encoder, (unsigned char*)gen.y, dt) < 0) {
#endif
        printf("Error: encoding failed.\n");
      }

#if WRITE_IVF      
      if (gen.frame > 300) { 
        break;
      }
      frame_timeout = now;
#else
      frame_timeout = now + frame_delay;
#endif
    }

    rxs_control_receiver_update(&control_receiver);
    rxs_sender_update(&sender);
  }

  rxs_generator_clear(&gen);
  rxs_ivf_destroy(&ivf);
  return 0;
}

static void sigh(int sn) {
  exit(1);
}

static void on_command(rxs_control_receiver* rec) {
  int i;
  rxs_packet* pkt = NULL;
  if (rec->command == RXS_CONTROL_COMMAND_RESEND) {
    for (i = 0; i < rec->count; ++i) {
      printf("- must resend: %d\n", rec->seqnums[i]);
      pkt = rxs_packets_find_seqnum(&packets, rec->seqnums[i]);
      if (pkt) {
        if (rxs_sender_send(&sender, pkt->data, pkt->nbytes) < 0) {
          printf("Error: cannot resend packet: %d\n", rec->seqnums[i]);
        }
      }
      else {
        printf("Error: cannot find sequence number to resend: %d\n", rec->seqnums[i]);
      }
    }
  }
  else if(rec->command == RXS_CONTROL_COMMAND_KEY_FRAME) {
    printf(" - requesting keyframe.\n");
    rxs_encoder_request_keyframe(&encoder);
  }
  else {
    printf("Got unhandled command: %d\n", rec->command);
  }
}

static void on_vp8_packet(rxs_encoder* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts) {
  if (!enc) { return ; } 
  if (!pkt) { return ; } 
  
  rxs_packetizer_wrap(&pack, pkt);
}

static void on_rtp_packet(rxs_packetizer* vpx, uint8_t* buffer, uint32_t nbytes) {

#if WRITE_IVF
  rxs_depacketizer_unwrap(&depack, buffer, (int64_t)nbytes);
  return;
#endif

  /* store this packet in the history */
  rxs_packet* pkt = rxs_packets_next(&packets);
  if (!pkt) {
    printf("Error: cannot get next packet for history.\n");
    exit(1);
  }

  pkt->seqnum = vpx->seqnum;

  if (rxs_packet_write(pkt, buffer, nbytes) < 0) {
    printf("Error: cannot write the history data.\n");
    exit(1);
  }

#if USE_DROPPING
  frame_count++;
  if (frame_count % frame_drop == 0) {
    printf("Simulate a drop.\n");
    return;
  }
#endif

  if (rxs_sender_send(&sender, buffer, nbytes) < 0) {
    printf("Error: cannot send the rtp packet.\n");
    exit(1);
  }
}

static void on_vp8_depacket(rxs_depacketizer* dep, uint8_t* buffer, uint32_t nbytes) {

#if WRITE_IVF
# if 1
  /* Here we wait until we get a keyframe and then we start 
     collecting until we get the "last packet" (where the marker
     bit is set to 1). 
  */
  static uint8_t data[1024 * 1024 * 4];
  static uint32_t pos = 0;
  static int had_key = 0;
  static uint64_t nframe = 0;

  if (!had_key && dep->N == 0) {
    had_key = 1;
  }

  if (nbytes > 0) {
    memcpy(data + pos, buffer, nbytes);
    pos += nbytes;
  }

  if (had_key != 0) {

    if (dep->marker == 1) {
      rxs_ivf_write_frame(&ivf, nframe, data, pos);
      nframe++;
    }
  }
  if (dep->marker == 1) {
    pos = 0;
  }
# else 
  /* simply writing everything to a file: IMPORTANT: can only be done 
     when the the video frames fit in on RTP packet */
  static uint64_t nframe = 0;
  rxs_ivf_write_frame(&ivf, nframe, buffer, nbytes);
  nframe++;
# endif 
#endif

}

static void on_data(rxs_receiver* rec, uint8_t* buffer, uint32_t nbytes) {
  printf(">>> received some data over network %u\n", nbytes);
}
