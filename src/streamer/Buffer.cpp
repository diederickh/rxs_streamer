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

  printf("Createing %d chunks for our buffer with a size of %u per chunk.\n", nchunks, nsize);

  for (int i = 0; i < nchunks; ++i) {

    Chunk* ar = new Chunk();
    if ( NULL == ar) {
      printf("Error: cannot allocate the chunks for the buffer.\n");
      ::exit(1);
    }

    ar->data.reserve(nsize);
    ar->is_free = true;
    free_chunks.push_back(ar);
  }
}

Buffer::~Buffer() {

  /* free our "free" chunks */
  for (size_t i = 0; i < free_chunks.size(); ++i) {
    delete free_chunks[i];
  }
  free_chunks.clear();

  /* free our "used" chunks. */
  for (size_t i = 0; i < used_chunks.size(); ++i) {
    delete used_chunks[i];
  }
  used_chunks.clear();
}

Chunk* Buffer::getFreeChunk() {
  Chunk* r = NULL;
  for (std::vector<Chunk*>::iterator it = free_chunks.begin(); it != free_chunks.end(); ++it) {
    r = *it;
    free_chunks.erase(it);
    break;
  }
  return r;
}

void Buffer::addUsedChunk(Chunk* c) {
  used_chunks.push_back(c);
}

void Buffer::addFreeChunk(Chunk* c) {
  free_chunks.push_back(c);
}
