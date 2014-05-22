/* 

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
#include <rxs_streamer/rxs_jitter.h>
#include <rxs_streamer/rxs_receiver.h>
#include <rxs_streamer/rxs_depacketizer.h>

static void sigh(int s);
static void on_data(rxs_receiver* r, uint8_t* buf, uint32_t nbytes);
static void on_packet(rxs_depacketizer* d, uint8_t* buffer, uint32_t nbytes);
static void on_missing_packet(rxs_jitter* jit);

rxs_jitter jit;
rxs_depacketizer depack;
rxs_receiver receiver;

int main() {
  

  signal(SIGINT, sigh);

  if (rxs_jitter_init(&jit) < 0) {
    printf("Error: cannot init jitter.\n");
    exit(1);
  }

  jit.on_missing_packet = on_missing_packet;

  if (rxs_receiver_init(&receiver, 6970) < 0) {
    printf("Error: cannot init the receiver.\n");
    exit(1);
  }

  if (rxs_depacketizer_init(&depack) < 0) {
    printf("Error: cannot init depacketizer.\n");
    exit(1);
  }
  
  receiver.on_data = on_data;
  depack.on_packet = on_packet;

  while(1) {
    rxs_receiver_update(&receiver);
    rxs_jitter_update(&jit);
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

  rxs_packet p;
  p.marker = d->marker;
  p.timestamp = d->timestamp;
  p.seqnum = d->seqnum; // + rand();
  p.data = buffer;
  p.nbytes = nbytes;
  printf("Got: %u, seqnum: %d\n", nbytes, p.seqnum);
  if (rxs_jitter_add_packet(&jit, &p) < 0) {
    printf("Error: could not add a new packet to the jitter buffer.\n");
    exit(1);
  }

}

static void on_missing_packet(rxs_jitter* jit) {
  printf("Missing packet: %u\n", jit->missing_seqnum);
}
