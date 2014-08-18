#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <streamer/Loop.h>

namespace rxs { 

  /* ------------------------------------------------------------- */

  WriteRequest::WriteRequest() {
    reset();
  }

  WriteRequest::~WriteRequest() {
    reset();
    is_free = false;
  }

  void WriteRequest::reset() {
    data = NULL;
    is_free = true;
    nbytes = 0;
    socket = NULL;
    on_write = NULL;
    user = NULL;
  }

  /* ------------------------------------------------------------- */

  ReadRequest::ReadRequest() {
    is_free = true;
  }

  ReadRequest::~ReadRequest() {
    is_free = false;
  }

  void ReadRequest::reset() {
    printf("@todo - at this point we don't need to reset the read request! must be implemented when e.g. a socket disconnects!\n");
    is_free = true;
    socket = NULL;
    on_read = NULL;
    user = NULL;
  }

  bool ReadRequest::getSenderInfo(std::string& ip, uint16_t& port) {
    if (NULL == socket) { 
      return false; 
    } 

    char shost[NI_MAXHOST];
    char sport[NI_MAXSERV];

    int r = getnameinfo((struct sockaddr*)&addr, 
                        sizeof(struct sockaddr_storage), 
                        shost, sizeof(shost), sport, 
                        sizeof(sport), NI_NUMERICHOST | NI_NUMERICSERV);

    if (0 != r) {
      printf("Error: cannot retrieve host info.\n");
      return false;
    }

    port = atoi(sport);
    ip = shost;

    return true;
  }

  /* ------------------------------------------------------------- */

  Loop::Loop() 
    :user(NULL)
  {
    kq = kqueue();
  }

  Loop::~Loop() {
    kq = 0;
    user = NULL;
    read_list.clear();
    write_list.clear();
    trigger_list.clear();
    monitor_list.clear();
  }

  int Loop::notifyRead(Socket* s) {

    if (NULL == s) { return -1; } 

    /* Get a read request. */
    ReadRequest* rr = getFreeReadRequest();
    rr->socket = s;
    rr->on_read = s->on_read;

    /* event monitor list */
    struct kevent mev;
    EV_SET(&mev, s->getSocketDescriptor(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, rr);
    read_list.push_back(mev);

    return 0;
  } 

  int Loop::notifyWrite(Socket* s) {
    if (NULL == s) { return -1; } 

    return 0;
  }

  void Loop::update() {

    int nev, i;
    struct timespec timeout = { 0, 0 } ;

    /* combine our read and write list. */
    monitor_list.clear();
    trigger_list.clear();
    std::copy(read_list.begin(), read_list.end(), std::back_inserter(monitor_list));
    std::copy(write_list.begin(), write_list.end(), std::back_inserter(monitor_list));
    std::copy(monitor_list.begin(), monitor_list.end(), std::back_inserter(trigger_list));
  
    /* check which events we get */
    nev = kevent(kq, &monitor_list[0], monitor_list.size(), &trigger_list[0], trigger_list.size(), &timeout);
    if (-1 == nev) {
      printf("Error: kevent returned -1.\n");
      return;
    }

    /* no event? just a timeout? */
    if (0 > nev) {
      return;
    }

    /* process all events */
    for (i = 0; i < nev; ++i) {
      struct kevent& kev = trigger_list[i];
      
      if (kev.flags == EV_EOF) {
        printf("EV_EOF: %lu\n", kev.ident);
      }
      else if (kev.flags & EV_ERROR) {
        printf("EV_ERROR: %s\n", strerror(kev.data));
      }
      else {

        /* read directly into the buffer of the socket. */
        if (kev.filter == EVFILT_READ) {
          read(kev);
        }
        else if (kev.filter == EVFILT_WRITE) {
          send(kev);
        }
      }
    }
  }

  ssize_t Loop::send(struct kevent& kev) {
  
    /* get the WriteRequest */
    WriteRequest* req = static_cast<WriteRequest*>(kev.udata);
    if (NULL == req) {
      printf("Error: cannot find the write request for the send() call.\n");
      exit(1);
    }

    if (req->socket->type == SOCKET_TYPE_UDP) {

      sendto(kev.ident, req->data, req->nbytes, 0, (struct sockaddr*)&req->addr, sizeof(struct sockaddr));

      if (NULL != req->on_write) {
        req->on_write(req);
      }
    }
    else {
      printf("Error: unhandled socket type in Loop::send().\n");
      exit(1);
    }

    /* all write requests are EV_ONESHOT, so we need to remote them here! */
    for (std::vector<struct kevent>::iterator it = write_list.begin(); it != write_list.end(); ++it) {
      struct kevent& wk = *it;
      if (wk.udata == req) {
        write_list.erase(it);
        break;
      }
    }

    return 0;
  }

  ssize_t Loop::read(struct kevent& kev) {

    /* find the read request for the ident. */
    ReadRequest* req = static_cast<ReadRequest*>(kev.udata);;
    if (NULL == req) {
      printf("Error: cannot find a read request for this read event.\n");
      exit(1);
    }
  
    if (NULL == req->socket) {
      printf("Error: the socket pointer of the ReadRequest is NULL; shouldn't happen. We're probably still have a read entry in the read event list but the user reset the read request.\n");
      return -2;
    }

    if (req->socket->type == SOCKET_TYPE_UDP) {
      return recvmsg(req);
    }

    printf("Error: unhandled socket type in Loop::read().\n");
    exit(1);
  }

  ssize_t Loop::recvmsg(ReadRequest* req) {

    ssize_t nread = 0;
    struct msghdr hdr;
    struct iovec vec;

    vec.iov_base = req->socket->in_buffer + req->socket->in_dx;
    vec.iov_len = (req->socket->in_capacity - req->socket->in_dx);

    hdr.msg_name = (void*)&req->addr;
    hdr.msg_namelen = sizeof(req->addr);
    hdr.msg_iov = &vec;
    hdr.msg_iovlen = 1;

    nread = ::recvmsg(req->socket->getSocketDescriptor(), &hdr, 0);
  
    if (-1 == nread) {
      printf("Error: unhandled UDP read error.\n");
      exit(1);
    }

    req->socket->in_dx += nread;

    if (hdr.msg_flags & MSG_TRUNC) {
      if (false == req->socket->growInputBuffer()) {
        printf("Error: cannot grow the input buffer.\n");
        exit(1);
      }
      printf("Error: only received part of the data that is available on UDP socket. @todo - handle this!\n");
    }

    if (NULL == req->socket->on_read) {
      printf("Warning: read something on socket, but no on_read handler set.\n");
      return -1;
    }

    req->socket->on_read(req);

    return nread;
  }

  WriteRequest* Loop::getFreeWriteRequest() {

    /* check if there is a free write request */
    for (size_t i = 0; i < write_requests.size(); ++i) {
      WriteRequest* wr = write_requests[i];
      if (true == wr->is_free) {
        wr->is_free = false;
        return wr;
      }
    }

    /* not found, allocate a new one. */
    WriteRequest* wr = new WriteRequest();
    if (NULL == wr) {
      printf("Error: cannot allocate a new WriteRequest.\n");
      ::exit(1);
    }

    wr->is_free = false;
    write_requests.push_back(wr);

    /* probably too many write requests in use, log a warning */
    if (100 < write_requests.size()) {
      printf("Warning: we've created %lu write requests. Looks like their not set free. You should call 'reset()' in your write callback.\n", write_requests.size());
    }

    return wr;
  }

  ReadRequest* Loop::getFreeReadRequest() {

    /* check if there is a free read request */
    for (size_t i = 0; i < read_requests.size(); ++i) {
      ReadRequest* rr = read_requests[i];
      if (true == rr->is_free) {
        rr->is_free = false;
        return rr;
      }
    }

    /* not found, allocate a new one. */
    ReadRequest* rr = new ReadRequest();
    if (NULL == rr) {
      printf("Error: cannot allocate a new ReadRequest.\n");
      ::exit(1);
    }

    rr->is_free = false;
    read_requests.push_back(rr);

    /* probably too many write requests in use, log a warning */
    if (100 < read_requests.size()) {
      printf("Warning: we've created %lu read requests. Looks like their not set free. You should call 'reset()' in your read callback.\n", read_requests.size());
    }

    return rr;
  }

  int Loop::sendTo(Socket* sender, struct sockaddr_in addr, 
                   const uint8_t* data, uint32_t nbytes, 
                   socket_on_write onwrite, void* udata) 
  {

    if (NULL == data) { return -2; } 
    if (0 == nbytes) { return -3; } 

    WriteRequest* wr = getFreeWriteRequest();
    if (NULL == wr) {
      printf("Error: cannot get a free write request.\n");
      return -5;
    }

    /* fill the write request */
    wr->data = data; 
    wr->nbytes = nbytes;
    wr->addr = addr;
    wr->socket = sender;
    wr->user = udata;
    wr->on_write = onwrite;

    /* create an monitor event */
    struct kevent mev;
    EV_SET(&mev, sender->getSocketDescriptor(), EVFILT_WRITE,  EV_ONESHOT | EV_ADD | EV_ENABLE, 0, 0, wr);
    write_list.push_back(mev);

    return 0;
  }

} /* namespace rxs */
