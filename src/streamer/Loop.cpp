#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <streamer/Loop.h>

/* ------------------------------------------------------------- */

WriteRequest::WriteRequest() {
  reset();
}

WriteRequest::~WriteRequest() {
  reset();
  is_free = false;
}

void WriteRequest::reset() {
  type = WRITE_REQ_NONE;
  data = NULL;
  is_free = true;
  nbytes = 0;
  sender = NULL;
  on_written = NULL;
  user = NULL;
}

/* ------------------------------------------------------------- */

Loop::Loop() 
  :user(NULL)
{
  kq = kqueue();
  printf("Created qeuue: %d\n", kq);
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

  /* event monitor list */
  struct kevent mev;
  EV_SET(&mev, s->getSocketDescriptor(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, s);
  read_list.push_back(mev);

  return 0;
} 

int Loop::notifyWrite(Socket* s) {
  if (NULL == s) { return -1; } 

  return 0;
}

void Loop::update() {

  int nev, i;
  struct timespec timeout = { 1, 0 } ;

  /* combine our read and write list. */
  monitor_list.clear();
  trigger_list.clear();
  std::copy(read_list.begin(), read_list.end(), std::back_inserter(monitor_list));
  std::copy(write_list.begin(), write_list.end(), std::back_inserter(monitor_list));
  std::copy(monitor_list.begin(), monitor_list.end(), std::back_inserter(trigger_list));

  nev = kevent(kq, &monitor_list[0], monitor_list.size(), &trigger_list[0], trigger_list.size(), &timeout);
  if (-1 == nev) {
    printf("Error: kevent returned -1.\n");
    return;
  }

  /* no event? just a timeout? */
  if (0 > nev) {
    return;
  }

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
      /* @todo make sure we don't read outside the buffer or overflow the wdx */
      if (kev.filter == EVFILT_READ) {
        printf("Read event! %p\n", kev.udata);

        Socket* socket = static_cast<Socket*>(kev.udata);
        int nread = read(kev.ident, socket->in_buffer + socket->in_dx, SOCKET_BUFFER_SIZE - socket->in_dx);
        socket->in_dx += nread;

        if (socket->on_read) {
          socket->on_read(socket);
        }
      }
      else if (kev.filter == EVFILT_WRITE) {
        WriteRequest* wr = static_cast<WriteRequest*>(kev.udata);
        if (wr->type == WRITE_REQ_UDP_SENDTO) {
          /* @todo - check result of sendto() */
          sendto(kev.ident, wr->data, wr->nbytes, 0, (struct sockaddr*)&wr->addr, sizeof(struct sockaddr));

          if (wr->on_written) {
            wr->on_written(wr);
          }
        }

        /* all write requests are EV_ONESHOT, so we need to remote them here! */
        for (std::vector<struct kevent>::iterator it = write_list.begin(); it != write_list.end(); ++it) {
          struct kevent& wk = *it;
          if (wk.ident == kev.ident) {
            write_list.erase(it);
            break;
          }
        }
      }
    }
  }
}

WriteRequest* Loop::getFreeWriteRequest() {

  /* check if there is a free write request */
  for (size_t i = 0; i < write_requests.size(); ++i) {
    WriteRequest* wr = write_requests[i];
    if (true == wr->is_free) {
      printf("Reusing write request.\n");
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
    printf("Warning: we've created %lu or write requests. Looks like their not set free. You should call 'reset()' in your write callback.\n", write_requests.size());
  }

  return wr;
}

int Loop::sendTo(Socket* sender, struct sockaddr_in addr, 
                 const uint8_t* data, uint32_t nbytes, 
                 loop_on_written onwritten, void* udata) 
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
  wr->type = WRITE_REQ_UDP_SENDTO;
  wr->sender = sender;
  wr->user = udata;
  wr->on_written = onwritten;

  /* create an monitor event */
  struct kevent mev;
  EV_SET(&mev, sender->getSocketDescriptor(), EVFILT_WRITE,  EV_ONESHOT | EV_ADD | EV_ENABLE, 0, 0, wr);
  write_list.push_back(mev);

  return 0;
}
