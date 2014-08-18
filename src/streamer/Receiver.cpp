#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <streamer/Receiver.h>

namespace rxs {

  /* -------------------------------------------------------------- */
  static void* receiver_thread(void* user);
  static void receiver_on_read(ReadRequest* req);
  /* -------------------------------------------------------------- */

  Receiver::Receiver(std::string lip, uint16_t lport, std::string rip, uint16_t rport)
    :rip(rip)
    ,lport(lport)
    ,lip(lip)
    ,rport(rport)
    ,must_stop(true)
    ,is_running(false)
    ,sock(&loop)
    ,on_data(NULL)
    ,user(NULL)
  {

  }
  
  Receiver::~Receiver() {
  }

  int Receiver::init() {

    if (true == is_running) {
      printf("Error: cannot start the receiver; already running.\n");
      return -1;
    }

    if (0 != sock.bind(lip, lport)) {
      return -2;
    }

    sock.on_read = receiver_on_read;
    sock.user = this;

    is_running = true;
    must_stop = false;
    
    if (0 != pthread_create(&thread, NULL, receiver_thread, this)) {
      printf("Error: cannot create the thread.\n");
      must_stop = true;
      is_running = false;
      return -3;
    }

    return 0;
  }

  int Receiver::shutdown() {

    if (false == is_running) {
      printf("Error: cannot shutdown the receiver; not running.\n");
      return -1;
    }
    
    must_stop = true;

    pthread_join(thread, NULL);

    return 0;
   
  }

  /* -------------------------------------------------------------- */
  static void* receiver_thread(void* user) {

    /* get receiver. */
    Receiver* recv = static_cast<Receiver*>(user);
    if (NULL == recv) {
      printf("Error: invalid thread user data in Receiver.\n");
      ::exit(1);
    }

    /* our thread loop */
    while (false == recv->must_stop) {
      recv->loop.update();
      usleep(10000);
    }

    recv->must_stop = false;
    recv->is_running = false;

    return NULL;
  }

  static void receiver_on_read(ReadRequest* req) {

    /* Make sure we received a valid read request. */
    if (NULL == req || NULL == req->socket->user) {
      printf("Error: invalid ReadRequest received in the receiver on read callback.\n");
      return;
    }

    /* Notify our listener. */
    Receiver* recv = static_cast<Receiver*>(req->socket->user);
    if (NULL == recv) {
      printf("Error: the receiver is invalid in the on_read callback of the receiver..\n");
      return;
    }

    if (NULL == recv->on_data) {
      printf("Warning: you haven't set a on_data handler of the Receiver.\n");
      return;
    }

    /* Call the set handler. */
    recv->on_data(recv, recv->sock.in_buffer, recv->sock.in_dx);
  }

} /* namespace rxs */
