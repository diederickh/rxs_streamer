#ifndef STREAMER_BUFFER_H
#define STREAMER_BUFFER_H

#include <vector>
#include <stdint.h>

namespace rxs { 

  /* --------------------------------------------------------------- */

  class Chunk {
  public:
    Chunk();
    ~Chunk();
    void append(uint8_t* bytes, uint32_t nbytes);           /* Copy the given bytes into the internal buffer */
    void replace(uint8_t* bytes, uint32_t nbytes);          /* Replace current data with the given bytes. */
    size_t capacity();                                      /* Returns how many bytes you can write into this chunk. */
    size_t size();                                          /* Returns the number of bytes written into this chunk */
    void clear();                                           /* Resets the buffer; should be called when you want to reset the data */
    uint8_t* ptr();                                         /* Returns pointer to first entry */
  public:
    std::vector<uint8_t> data;                              /* The buffer into you write your data. */
    bool is_free;                                           /* Indicates is this chunk is in use or if you can write into the member. */
    void* user;                                             /* User data */
  };

  /* --------------------------------------------------------------- */

  inline size_t Chunk::capacity() {
    return data.capacity();
  }

  inline size_t  Chunk::size() {
    return data.size();
  }

  inline uint8_t* Chunk::ptr() {
    return &data[0];
  }

  inline void Chunk::clear() {
    data.clear();
  }

  /* --------------------------------------------------------------- */

  class Buffer {
  public:
    Buffer(uint32_t nsize, int nchunks);                    /* Create nchunks of nsize */
    ~Buffer();                                              /* Destructor, will free all allocate memory / chunks */
    Chunk* getFreeChunk();                                  /* Returns the next free chunk from the list of internal chunks an also setting the chunk to non free */
    void addUsedChunk(Chunk* c);         
    void addFreeChunk(Chunk* c);

  public:    
    std::vector<Chunk*> free_chunks;                        /* The collection of chunks. */
    std::vector<Chunk*> used_chunks;                        /* The collection of chunks. */
  };

} /* namespace rxs */

#endif
