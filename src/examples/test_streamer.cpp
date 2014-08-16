#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <streamer/Loop.h>
#include <streamer/Socket.h>
#include <streamer/SocketUDP.h>

static void on_read(Socket* s);
static void on_written(WriteRequest* wr);

int main() {

  printf("\n\ntest_streamer\n\n");
  Loop loop;

  SocketUDP server(&loop);
  server.on_read = on_read;
  server.bind("127.0.0.1", 6667);

  SocketUDP client(&loop);
  client.bind("127.0.0.1", 6668);

  std::string test_data = "Hello world.";
  client.sendTo("127.0.0.1", 6667, (const uint8_t*)test_data.data(), test_data.size(), NULL, NULL);

  while(1) {
    loop.update();
    client.sendTo("127.0.0.1", 6667, (const uint8_t*)test_data.data(), test_data.size(), on_written, NULL);
    usleep(100000);
  }

  return 0;
}


static void on_read(Socket* s) {
  printf("Read some bytes.\n");
  for (int i = 0; i < s->in_dx; ++i) {
    printf("%c", s->in_buffer[i]);
  }
  printf("\n");
  s->in_dx = 0;
}

static void on_written(WriteRequest* wr) {
  printf("Ready with writing the data.\n");
  wr->reset();
}
