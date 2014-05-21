#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rxs_streamer/rxs_packets.h>

#define FAKE_SIZE (1024 * 1024)

int main() {
  printf("\n\ntest_packets.\n\n");

  int i;
  rxs_packet* pkt;
  rxs_packets ps;
  uint8_t* fake_data;

  if (!rxs_packets_init(&ps, 10, FAKE_SIZE) < 0) {
    printf("Error: cannot create the packets buffer.\n");
    exit(1);
  }

  fake_data = (uint8_t*)malloc(FAKE_SIZE);
  if (!fake_data) {
    printf("Error: out of mem, cannot alloc tmp buffer.\n");
    exit(1);
  }

  for (i = 0; i < 15; ++i) {
    pkt = rxs_packets_find_free(&ps);
    if (!pkt) {
      printf("Warning: cannot find a free packet for: %d\n", i);
    }
    else {
      printf("Writing into a packet: %p, %zu bytes\n", pkt->data, FAKE_SIZE);
      if (rxs_packet_write(pkt, fake_data, sizeof(fake_data)) < 0) {
        printf("Error: cannot write to packet.\n");
      }
      else {
        printf("Wrote some fake data.\n");
      }
    }
    printf("-\n");
  }

  free(fake_data);
  fake_data = NULL;

  if (rxs_packets_clear(&ps) < 0) {
    printf("Error: cannot clear the packet buffer. Leaking here.\n");
    exit(1);
  }

  printf("\n\n");

  return 0;
}
