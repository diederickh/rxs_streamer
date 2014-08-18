#include <stdio.h>
#include <stdlib.h>
#include <streamer/Socket.h>
#include <streamer/Loop.h>

namespace rxs {

  Socket::Socket(Loop* loop, size_t buffersize) 
    :in_dx(0)
    ,on_read(NULL)
    ,user(NULL)
    ,loop(loop)
    ,in_capacity(buffersize)
    ,type(SOCKET_TYPE_NONE)
  {

    in_buffer = new uint8_t[buffersize];
    if (NULL == in_buffer) {
      printf("Error: socket cannot allocate the input buffer. Out of mem?\n");
      exit(1);
    }

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

    in_dx = 0;
    in_capacity = 0;
    user = NULL;
    on_read = NULL;
  }


  bool Socket::growInputBuffer() {

    /* duplicate the size. */
    uint8_t* new_buffer = (uint8_t*)realloc(in_buffer, in_capacity * 2);
    if (NULL == new_buffer) {
      printf("Error: cannot reallocate the input buffer. Make sure you use it read bytes and reset in_dx of your socket.\n");
      return false;
    }

    in_buffer = new_buffer;
    in_capacity = in_capacity * 2;

    printf("Verbose: we've grown the input buffer for the socket to a size of: %u bytes.\n", in_capacity);
    return true;
  }

} /* namespace rxs */
