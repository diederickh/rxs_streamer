/*

  test_signal_redis
  -----------------

  Using redis as signaling intermediate. We're making use of the redis
  pub/sub feature. Note that when you're subscribed to a channel you 
  cannot change/set values using rxs_signal_store_address()

 */

#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <rxs_streamer/rxs_signaling.h>
rxs_signal sig_pub;
rxs_signal sig_sub;
rxs_signal sig_retr;
uint64_t timeout;
uint64_t delay;
uint64_t now;

static void on_address(rxs_signal* signal, char* ip, uint16_t port);

int main() {

  printf("\n\ntest_signal_redis\n\n");

  // subscriber.
  if (rxs_signal_init(&sig_sub, "home.roxlu.com", 6379) < 0) {
    printf("Error: rxs_signal_init() failed. (2)\n");
    exit(1);
  }

  if (rxs_signal_subscribe(&sig_sub, 5) < 0) {
    printf("Error: rxs_signal_subscribe() failed.\n");
    exit(1);
  }

  sig_sub.on_address = on_address;

  // publisher
  if (rxs_signal_init(&sig_pub, "home.roxlu.com", 6379) < 0) {
    printf("Error: rxs_signal_init() failed. (1)\n");
    exit(1);
  }

  delay = 1 * 1000llu * 1000llu * 1000llu;
  timeout = uv_hrtime() + delay;

  // retriever
  if (rxs_signal_init(&sig_retr, "home.roxlu.com", 6379) < 0) {
    printf("Error: rxs_signal_init() failed. (3)\n");
    exit(1);
  }
  if (rxs_signal_retrieve_address(&sig_retr, 5) < 0) {
    printf("Error: rxs_signal_retrieve_address() failed.\n");
    exit(1);
  }

  sig_retr.on_address = on_address;

  while(1) {

    /* every n-seconds publish the ip on slot 5. */
    now = uv_hrtime();
    if (now > timeout) {
      timeout = now + delay;
      if (rxs_signal_store_address(&sig_pub, 5, "127.0.0.2", 1234) < 0) {
        printf("rxs_signal_store_address() failed.\n");
        exit(1);
      }
    }

    rxs_signal_update(&sig_pub);
    rxs_signal_update(&sig_sub);
    rxs_signal_update(&sig_retr);
  }

  return 0;
}

static void on_address(rxs_signal* signal, char* ip, uint16_t port) {
  printf("-- received address: %s:%d\n", ip, port);
}
