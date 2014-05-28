#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <rxs_streamer/rxs_stun_io.h>

static void sigh(int s);

rxs_stun_io stun_io;

int main() {
  printf("\n\ntest_stun\n\n");

  signal(SIGINT, sigh);

  if (rxs_stun_io_init(&stun_io, "stun.l.google.com", "19302") < 0) {
    printf("Error: cannot init stun io.\n");
    exit(1);
  }

  while(1) {
    rxs_stun_io_update(&stun_io);
  }

  printf("\n");
  return 0;
}

static void sigh(int s) {
  printf("\n\nSIGNALLED!\n\n");
  exit(0);
}
