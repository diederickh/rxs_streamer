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

      /* Make sure the buffer is reset before we write new data into it. */
      c->clear();

      /* Copy data */
      std::copy(test_data.begin(), test_data.end(), std::back_inserter(c->data));

      /* Give our chunk to the sender which will make sure it's delivere to the remote ip:port */
      sender.sendChunk(c);

      printf("Size of chunk: %lu\n", c->size());
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
