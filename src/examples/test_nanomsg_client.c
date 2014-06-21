#include <stdio.h>
#include <stdlib.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

int sock;

int main() {
  int r;
  char* buf;
  int bytes;

  printf("\n\ntest_nanomsg_client\n\n");
  sock = nn_socket(AF_SP, NN_SUB);
  if (sock < 0) {
    printf("Error: cannot create nano sock.\n");
    exit(1);
  }

  r = nn_setsockopt(sock, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
  if (r < 0) {
    printf("Error: cannot set sock opt.\n");
    exit(1);
  }

  r = nn_connect(sock, "tcp://127.0.0.1:8998");
  if (r < 0) {
    printf("Error: cannot connect.\n");
  }

  while(1) {

    buf = NULL;
    bytes = nn_recv(sock, &buf, NN_MSG, 0);

    if (bytes < 0) {
      printf("Verbose: disconnected.\n");
      break;
    }
    
    printf("CLIENT RECEIVED: %s\n", buf);

    nn_freemsg(buf);
  }

  nn_shutdown(sock, 0);

  return 0;
}

