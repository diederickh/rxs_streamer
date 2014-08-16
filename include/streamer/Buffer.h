#ifndef STREAMER_BUFFER_H
#define STREAMER_BUFFER_H

#include <vector>
#include <stdint.h>

/* --------------------------------------------------------------- */

class Chunk {
 public:
  Chunk();
  ~Chunk();
  void copy(uint8_t* bytes, uint32_t nbytes);             /* Copy the given bytes into the internal buffer */
  size_t capacity();                                      /* Returns how many bytes you can write into this chunk. */
  size_t size();                                          /* Returns the number of bytes written into this chunk */

 public:
  std::vector<uint8_t> data;                              /* The buffer into you write your data. */
  bool is_free;                                           /* Indicates is this chunk is in use or if you can write into the member. */
};

/* --------------------------------------------------------------- */

inline size_t Chunk::capacity() {
  return data.capacity();
}

inline size_t  Chunk::size() {
  return data.size();
}

/* --------------------------------------------------------------- */

class Buffer {
 public:
  Buffer(uint32_t nsize, int nchunks);                    /* Create nchunks of nsize */
  ~Buffer();                                              /* Destructor, will free all allocate memory / chunks */
  Chunk* getFreeChunk();                                  /* Returns the next free chunk from the list of internal chunks an also setting the chunk to non free */

 public:    
  std::vector<Chunk*> chunks;                             /* The collection of chunks. */
};

#endif
