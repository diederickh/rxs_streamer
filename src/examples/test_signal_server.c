#include <stdio.h>
#include <stdlib.h>
#include <rxs_streamer/rxs_signaling.h>

rxs_sigserv server;

int main() {
  int r;
  printf("\n\ntest_signal_server\n\n");

  //if (rxs_sigserv_init(&server, "tcp://192.168.0.194:5995") < 0) {
  r = rxs_sigserv_init(&server, "tcp://0.0.0.0:5995");
  if (r < 0) {
    printf("Error: cannot create signal server: %d\n", r);
    exit(0);
  }

  while (1) {
    rxs_sigserv_update(&server);
  }

  if (rxs_sigserv_clear(&server) < 0) {
    printf("Error: failed to clear/unset the signal server.\n");
  }
  return 0;
}
