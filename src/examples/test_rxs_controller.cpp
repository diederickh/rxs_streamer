#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <streamer/Controller.h>

static void sig(int s);

bool must_run = true;

int main() {

  printf("\n\ntest_controller\n\n");

  signal(SIGINT, sig);

  rxs::Controller controller("127.0.0.1", 6688);
  if (0 != controller.init()) {
    exit(1);
  }

  controller.addRemote("127.0.0.1", 7000);
  controller.addRemote("127.0.0.1", 7001);
  controller.addRemote("127.0.0.1", 7002);
  controller.addRemote("127.0.0.1", 7003);

  while(must_run) {
    controller.update();
  }

  return 0;
}

static void sig(int s) {
  printf("Sig handler.\n");
  must_run = false;
}
