/*

  text_rxs_receiver
  ----------------

  This tests shows how the receiver binds a local socket to 7001 and 
  receives data from the controller. This example is pretty basic and
  just prints out when it receives some data. 

  IMPORTANT: 
  ----------
  
  - Make sure to set the in_dx to 0 in the data handler. 
  - The on_data callback is called from another thread!

 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <streamer/Receiver.h>
#include "config.h"

static void sig(int s);
static void on_data(rxs::Receiver* recv, uint8_t* data, uint32_t nbytes);

bool must_run = true;

int main() {
  printf("\n\ntest_rxs_receiver\n\n");

  signal(SIGINT, sig);

  /* Initialize the receiver. */
  rxs::Receiver recv(RECEIVER_IP, RECEIVER_PORT, CONTROLLER_IP, CONTROLLER_PORT);
  if (0 != recv.init()) {
    exit(1);
  }

  /* Set our data handler: is called whenever we read some data from the controller */
  recv.on_data = on_data;
  
  /* we simply keep running until the user presses CTRL-C */
  while(must_run) {
    printf("Stil running the Receiver.\n");
    usleep(1000000);
  }

  printf("Shutting down.\n");
  recv.shutdown();
}

static void sig(int s) {
  printf("Sig handler.\n");
  must_run = false;
}

static void on_data(rxs::Receiver* recv, uint8_t* data, uint32_t nbytes) {

  /* IMPORTANT: We must reset the in_dx of the socket whenever we're ready with parsing the data */
  recv->sock.in_dx = 0;

  printf("Received %u bytes of data.\n", nbytes);
  
}

