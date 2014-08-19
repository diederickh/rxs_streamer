#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <streamer/Sender.h>

namespace rxs {

  /* -------------------------------------------------------------- */
  static void sender_on_write(WriteRequest* req); 
  static void* sender_thread(void* user);
  /* -------------------------------------------------------------- */

  Sender::Sender(uint32_t chunksize, std::string lip, uint16_t lport, std::string rip, uint16_t rport) 
    :buffer(chunksize, 50)
    ,sock(&loop)
    ,lip(lip)
    ,lport(lport)
    ,rip(rip)
    ,rport(rport)
    ,is_running(false)
    ,must_stop(true)
  {
  
  }

  Sender::~Sender() {
    /* @todo - destroy mutex, cond, thread. */
  }

  int Sender::init() {

    if (true == is_running) {
      printf("Error: Sender is already running.\n");
      return -1;
    }

    if (0 != sock.bind(lip, lport)) {
      return -2;
    }

    /* @todo - destroy mutex, cond, thread. */
    if (0 !=  pthread_mutex_init(&mutex, NULL)) {
      printf("Error: cannot initialize the mutex for the sender.\n");
      return -3;
    }

    if (0 != pthread_cond_init(&cond, NULL)) {
      printf("Error: cannot initilialize the cond var in the sender.\n");
      return -4;
    }

    must_stop = false;
    if (0 != pthread_create(&thread, NULL, sender_thread, this)) {
      printf("Error: cannot initialize the thread for the sender.\n");
      return -5;
    }
  
    is_running = true;
    return 0;
  }

  Chunk* Sender::getFreeChunk() {
    Chunk* c;

    lock();
    {
      c = buffer.getFreeChunk();
    }
    unlock();

    if (NULL == c) {
      printf("Error: the sender can't find a new free chunk!\n");
    }
    return c;
  }

  void Sender::sendChunk(Chunk* c) {
    lock();
    {
      buffer.addUsedChunk(c);
      pthread_cond_signal(&cond);
    }
    unlock();
  }

  void Sender::freeChunk(Chunk* c) {

    if (NULL == c) {
      printf("Error: invalid chunk given to Sender::freeChunk.\n");
      return;
    }

    lock();
      buffer.addFreeChunk(c);
    unlock();
  }

  void Sender::shutdown() {
    if (false == is_running) {
      printf("Error: trying to shutdown the sender but it's not running.\n");
      return;
    }

    must_stop = true;

    lock();
    {
      pthread_cond_signal(&cond);
    }
    unlock();

    /* @todo - destroy mutex/cond */
    printf("Verbose: shutting down thread..\n");
    pthread_join(thread, NULL);
  }

  /* -------------------------------------------------------------- */
  static void sender_on_write(WriteRequest* req) {
    //    printf("sender_on_write - making the WriteRequest free again + reset the chunk\n");

#if 1
    /* get the chunk so we can make it free again. */
    Chunk* chunk = static_cast<Chunk*>(req->user);
    if (NULL == chunk) {
      printf("Error: cannot get the chunk* in sender_on_write!\n");
      exit(1);
    }

    Sender* sender = static_cast<Sender*>(chunk->user);
    if (NULL == sender) {
      printf("Error: cannot get sender* from the chunk in sender_on_write!\n");
      exit(1);
    }

    /* add the chunk to the free list so it can be used again. */
    sender->lock();
    sender->buffer.addFreeChunk(chunk);
    sender->unlock();
#endif

    /* and reset the write request. */
    req->reset();
  }

  static void* sender_thread(void* user) {

    printf("Sender thread running.\n");

    /* get the sender. */
    int r = 0;
    Sender* sender = static_cast<Sender*>(user);
    if (NULL == sender) {
      printf("Error: sender thread did not get the Sender* as user data; not supposed to happen.\n");
      ::exit(1);
    }

    std::vector<Chunk*> work;

    while (false == sender->must_stop) {

      /* Wake up when the condition var triggers */
      sender->lock();
      {
        while (0 == work.size() && false == sender->must_stop) {
          pthread_cond_wait(&sender->cond, &sender->mutex);
          std::copy(sender->buffer.used_chunks.begin(), sender->buffer.used_chunks.end(), std::back_inserter(work));
          sender->buffer.used_chunks.clear();
        }
      }
      sender->unlock();

      if (sender->must_stop) {
        break;
      }

      if (0 == work.size()) {
        /* spurious wake-up. */
        continue;
      }

      /* Send the data from the chunks! */
      for (size_t i = 0; i < work.size(); ++i) {
        Chunk* chunk = work[i];
        chunk->user = sender;

        r = sender->sock.sendTo(sender->rip, sender->rport, chunk->ptr(), chunk->size(), sender_on_write, chunk);
        if (r != 0) {
          printf("Error: cannot send data: %d\n", r);
        }

        sender->loop.update();
      }

      work.clear();
    }

    printf("Sender thread stopped.\n");

    sender->is_running = false;
    return NULL;
  }

} /* namespace rxs */
