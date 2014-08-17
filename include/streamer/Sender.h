/* 
   
   Sender
   -----

   Sender is used to push media data to the Controller. It wraps
   around the Loop/Socket/etc.. to make it easy for the user to 
   push data.

*/

#ifndef STREAMER_SENDER_H
#define STREAMER_SENDER_H

#include <streamer/Buffer.h>
#include <streamer/Socket.h>
#include <streamer/SocketUDP.h>
#include <streamer/Loop.h>
#include <pthread.h>
#include <string>
#include <stdint.h>

class Sender {

 public: 
  Sender(std::string lip, uint16_t lport, std::string rip, uint16_t rport);       /* lip = local ip, lport = local port, rip = remote IP, rport = remote port to which we send data. */
  ~Sender();
  int init();
  void shutdown();
  void update();
  void lock();
  void unlock();
  Chunk* getFreeChunk();
  void sendChunk(Chunk* c);

 public:
  Buffer buffer;                                                                  /* our output buffer, contains the chunks we can send. */
  Loop loop;
  SocketUDP sock;
  bool is_running;
  std::string rip;
  std::string lip;
  uint16_t rport;
  uint16_t lport;
  bool must_stop;
  pthread_t thread;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
};

inline void Sender::lock() {
  pthread_mutex_lock(&mutex);
}

inline void Sender::unlock() {
  pthread_mutex_unlock(&mutex);
}

#endif
