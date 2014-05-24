/* 

   test_jitter
   -----------
   
   Testing rxs_jitter. 
   
   References: 
   -----------
   
   Experimenting with packet loss can done using the `tc` util on linux. See 
   this SO for some info http://stackoverflow.com/questions/614795/simulate-delayed-and-dropped-packets-on-linux
   
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <rxs_streamer/rxs_types.h>
#include <rxs_streamer/rxs_jitter.h>
#include <rxs_streamer/rxs_receiver.h>
#include <rxs_streamer/rxs_depacketizer.h>
#include <rxs_streamer/rxs_decoder.h>
#include <rxs_streamer/rxs_control.h>

static void sigh(int s);
static void on_data(rxs_receiver* r, uint8_t* buf, uint32_t nbytes);
static void on_packet(rxs_depacketizer* d, uint8_t* buffer, uint32_t nbytes);
static void on_missing_seqnum(rxs_jitter* jit, uint16_t* seqnums, int num);
static void on_frame(rxs_jitter* jit, uint8_t* data, uint32_t nbytes);
static void on_jit_packet(rxs_jitter* jit, rxs_packet* pkt);
static void on_vp8_image(rxs_decoder* dec, vpx_image_t* img);

rxs_jitter jit;
rxs_depacketizer depack;
rxs_receiver receiver;
rxs_decoder decoder;
rxs_control_sender control_sender;

int main() {
  
  signal(SIGINT, sigh);

  if (rxs_jitter_init(&jit) < 0) {
    printf("Error: cannot init jitter.\n");
    exit(1);
  }

  if (rxs_receiver_init(&receiver, 6970) < 0) {
    printf("Error: cannot init the receiver.\n");
    exit(1);
  }

  if (rxs_depacketizer_init(&depack) < 0) {
    printf("Error: cannot init depacketizer.\n");
    exit(1);
  }

  if (rxs_decoder_init(&decoder) < 0) {
    printf("Error: cannot init decoder.\n");
    exit(1);
  }

  //if (rxs_control_sender_init(&control_sender, "192.168.0.190", RXS_CONTROL_PORT) < 0) {
  if (rxs_control_sender_init(&control_sender, "0.0.0.0", RXS_CONTROL_PORT) < 0) {
    printf("Error: cannot control sender.\n");
    exit(1);
  }
  
  receiver.on_data = on_data;
  depack.on_packet = on_packet;
  decoder.on_image = on_vp8_image;
  jit.on_missing_seqnum = on_missing_seqnum;
  jit.on_frame = on_frame;

  while(1) {
    rxs_receiver_update(&receiver);
    rxs_jitter_update(&jit);
    rxs_control_sender_update(&control_sender);
  }
  
  return 0;
}

static void sigh(int s) {
  printf("\nCatched signal.\n");
  exit(1);
}

static void on_data(rxs_receiver* r, uint8_t* buf, uint32_t nbytes) {
  if (rxs_depacketizer_unwrap(&depack, buf, nbytes) < 0) {
    printf("Error: cannot unwrap the incoming packet.\n");
    exit(1);
  }
}

static void on_packet(rxs_depacketizer* d, uint8_t* buffer, uint32_t nbytes) {

  /* copy all necessary data for the jitter buffer */
  rxs_packet p;
  p.marker = d->marker;
  p.timestamp = d->timestamp;
  p.seqnum = d->seqnum; // + rand();
  p.data = buffer;
  p.nbytes = nbytes;
  p.nonref = d->N;

  if (rxs_jitter_add_packet(&jit, &p) < 0) {
    printf("Error: could not add a new packet to the jitter buffer.\n");
    exit(1);
  }
}

static void on_missing_seqnum(rxs_jitter* jit, uint16_t* seqnums, int num) {
  printf("Missing some sequence numbers: %d\n", num);
  rxs_control_sender_request_packets(&control_sender, seqnums, num);
}

static void on_frame(rxs_jitter* jit, uint8_t* data, uint32_t nbytes) {
  if (rxs_decoder_decode(&decoder, data, nbytes) < 0) {
    printf("Error: cannot decode.\n");
    exit(1);
  }
}

static void on_jit_packet(rxs_jitter* jit, rxs_packet* pkt) {

  static int had_key = 0;
  if (pkt->nonref == 0) {
    had_key = 1;
  }

  if (!had_key) {
    printf("- ignoring pkt, because we're waiting for a keyframe. %d\n", pkt->seqnum);
    return;
  }
  
  if (rxs_decoder_decode(&decoder, pkt->data, pkt->nbytes) < 0) {
    printf("Error: cannot decode.\n");
    exit(1);
  }
}

static void on_vp8_image(rxs_decoder* dec, vpx_image_t* img) {
  //  printf("Got a decoded vp8 image.\n");
}
