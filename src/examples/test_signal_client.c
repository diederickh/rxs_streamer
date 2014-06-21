#include <libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rxs_streamer/rxs_signaling.h>

rxs_sigclient client;

int main() {

  printf("\n\ntest_signal_client\n\n");

  if (rxs_sigclient_init(&client, "tcp://127.0.0.1:5656") < 0) {
    printf("Error: cannot initialize the signaling client.\n");
    exit(1);
  }
  
  uint32_t ip = 0x01010202;
  uint16_t port = 0x0303;

  while(1) {
    rxs_sigclient_store_address(&client, 0, ip, port); 
    rxs_sigclient_update(&client);
    sleep(1);
  }

  if (rxs_sigclient_clear(&client) < 0) {
    printf("Error: cannot clear the sigclient.\n");
    exit(1);
  }

  return 0;
}
