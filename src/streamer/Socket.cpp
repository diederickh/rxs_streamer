#include <stdio.h>
#include <stdlib.h>
#include <streamer/Socket.h>
#include <streamer/Loop.h>

Socket::Socket(Loop* loop) 
  :in_dx(0)
   //  ,out_dx(0)
  ,on_read(NULL)
  ,user(NULL)
  ,loop(loop)
{

  in_buffer = new uint8_t[SOCKET_BUFFER_SIZE];
  if (NULL == in_buffer) {
    printf("Error: socket cannot allocate the input buffer. Out of mem?\n");
    exit(1);
  }
  
  /* 
  out_buffer = new uint8_t[SOCKET_BUFFER_SIZE];
  if (NULL == out_buffer) {
    printf("Error: socket cannot allocate the output buffer. Out of mem?\n");
    exit(1);
  }
  */

  if (NULL == loop) {
    printf("Error: created socket with a invalid loop!\n");
    exit(1);
  }
}

Socket::~Socket() {

  if (NULL != in_buffer) {
    delete[] in_buffer;
    in_buffer = NULL;
  }

  /*
  if (NULL != out_buffer) {
    delete[] out_buffer;
    out_buffer = NULL;
  }
  */

  in_dx = 0;
  //  out_dx = 0;
  user = NULL;
  on_read = NULL;
}
