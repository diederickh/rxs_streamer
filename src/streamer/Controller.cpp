#include <streamer/Controller.h>

namespace rxs  {
  /* -------------------------------------------------------------------------- */

  static void on_read(ReadRequest* req);
  static void on_write(WriteRequest* req);

  /* -------------------------------------------------------------------------- */

  Controller::Controller(std::string ip, uint16_t port) 
    :sock(&loop)
    ,ip(ip)
    ,port(port)
  {
  }

  Controller::~Controller() {
    remotes.clear();
  }

  int Controller::init() {
    if (0 == port) { return -1; } 
    if (0 == ip.size()) { return -2; } 

    /* Bind the (UDP) socket */
    if (0 != sock.bind(ip, port)) {
      return -3;
    }

    sock.on_read = on_read;
    sock.user = this;
  
    return 0;
  }

  void Controller::addRemote(std::string rip, uint16_t rport) {
    Remote r;
    r.ip = rip;
    r.port = rport;
    remotes.push_back(r);
  }

  void Controller::update() {
    loop.update();
  }

  /* -------------------------------------------------------------------------- */

  static void on_read(ReadRequest* req) {

    /* get controller */
    Controller* con = static_cast<Controller*>(req->socket->user);
    if (NULL == con) {
      printf("Error: the socket user data is not set correctly in Controller. Not supposed to happen.\n");
      return;
    }
  
    Socket* s = req->socket;
    if (NULL == s) {
      printf("Error: socket member of the read request is NULL. Not supposed to happen.\n");
      return;
    }
  
    for (size_t i = 0; i < con->remotes.size(); ++i) {
      Remote rem = con->remotes[i];
      con->sock.sendTo(rem.ip, rem.port, s->in_buffer, s->in_dx, on_write, NULL);
      printf("Sending %u bytes to %s:%hu\n", s->in_dx, rem.ip.c_str(), rem.port);
    }

    s->in_dx = 0;
  }

  static void on_write(WriteRequest* req) {
    req->reset();
  }

} /* namespace rxs */
