#include <libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <pack.h>

int sock;

int main() {
  int r;
  const char* buf = "THIS IS THE SERVER";
  int bytes;

  printf("\n\ntest_nanomsg_server\n\n");
  sock = nn_socket(AF_SP, NN_PUB);
  if (sock < 0) {
    printf("Error: cannot create nano sock.\n");
    exit(1);
  }

  r = nn_bind(sock, "tcp://127.0.0.1:8998");
  if (r < 0) {
    printf("Error: cannot bind nanomsg sock.\n");
    exit(1);
  }


  while(1) {

    printf("Sending: %s\n", buf);

    bytes = nn_send(sock, buf, strlen(buf) + 1, 0);

    if (bytes < 0) {
      printf("Verbose: disconnected.\n");
      break;
    }

    sleep(1);
  }

  nn_shutdown(sock, 0);

  return 0;
}

/* --------------------------------------------------------------------------------- */
