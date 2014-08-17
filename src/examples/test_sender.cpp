#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <streamer/Sender.h>


static void sig(int s);

bool must_run = true;

int main() {

  signal(SIGINT, sig);

  Sender sender("127.0.0.1", 6677, "127.0.0.1", 6688);
  if (0 != sender.init()) {
    exit(1);
  }

  std::string test_data = "test data";

  while(must_run) {

    /* Get a free chunk, add some test data to it and give it back to the sender so it's send. */
    Chunk* c = sender.getFreeChunk();
    if (NULL != c) {
      std::copy(test_data.begin(), test_data.end(), std::back_inserter(c->data));
      sender.sendChunk(c);
    }

    usleep(100000);
  }

  sender.shutdown();

  return 0;
}

static void sig(int s) {
  printf("Sig hanlder.\n");
  must_run = false;
}
