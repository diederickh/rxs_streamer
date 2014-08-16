#include <stdlib.h>
#include <stdio.h>
#include <streamer/Buffer.h>

/* --------------------------------------------------------------- */

Chunk::Chunk() 
  :is_free(true)
{
}

Chunk::~Chunk() {
  data.clear();
  is_free = false;
}

void Chunk::copy(uint8_t* bytes, uint32_t nbytes) {
  std::copy(bytes, bytes + nbytes, std::back_inserter(data));
}

/* --------------------------------------------------------------- */

Buffer::Buffer(uint32_t nsize, int nchunks) {

  for (int i = 0; i < nchunks; ++i) {

    Chunk* ar = new Chunk();
    if ( NULL == ar) {
      printf("Error: cannot allocate the chunks for the buffer.\n");
      ::exit(1);
    }

    ar->data.reserve(nsize);
    ar->is_free = true;
    chunks.push_back(ar);
  }
}

Buffer::~Buffer() {

  /* free our chunks */
  for (size_t i = 0; i < chunks.size(); ++i) {
    delete chunks[i];
  }
  chunks.clear();
}

Chunk* Buffer::getFreeChunk() {
  for (size_t i = 0; i < chunks.size(); ++i) {
    Chunk* c = chunks[i];
    if (true == c->is_free) { 
      c->is_free = false;
      return c;
    }
  }
  return NULL;
}
