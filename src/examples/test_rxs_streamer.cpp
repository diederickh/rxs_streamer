/*
  
  test_streamer
  -------------

  This file demonstrates a basic example on how to use the 
  C++ streamer socket code. It creates a UDP server/client and
  starts sending data. 

  When the server receives data it is automatically stored into 
  the in_buffer member of the socket. New data will be appended to 
  this buffer. Whenever you have processed data from this buffer
  you need to reset the in_dx member. 

 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <streamer/Loop.h>
#include <streamer/Socket.h>
#include <streamer/SocketUDP.h>

static void on_read(rxs::ReadRequest* s);
static void on_written(rxs::WriteRequest* wr);

int main() {

  printf("\n\ntest_streamer\n\n");

  /* Create the server and client sockets. */
  rxs::Loop loop;
  rxs::SocketUDP server(&loop, 128);
  rxs::SocketUDP client(&loop);

  /* Bind the sockets. */
  server.bind("127.0.0.1", 6667);
  client.bind("127.0.0.1", 6668);

  /* Whenever the server gets some data, on_read is called.
     The data which is read from the socket is stored in the 
     in_buffer of the socket. You need to make sure to 
     reset the in_dx which points to the write index. */
  server.on_read = on_read;

  std::string test_data = "Hello world.\n";

  while(1) {
    loop.update();
    client.sendTo("127.0.0.1", 6667, (const uint8_t*)test_data.data(), test_data.size(), on_written, NULL);
    usleep(100000);
  }

  return 0;
}


static void on_read(rxs::ReadRequest* rr) {
  rxs::Socket* s = rr->socket;
  printf("Read some bytes, current dx: %u.\n", s->in_dx);
  for (int i = 0; i < s->in_dx; ++i) {
    printf("%c", s->in_buffer[i]);
  }
  s->in_dx = 0;

 /* note: we don't need to reset the ReadRequest here, because that would mean we won't get any reads anymore. */
}

static void on_written(rxs::WriteRequest* wr) {
  printf("Ready with writing the data.\n");
  wr->reset();
}
