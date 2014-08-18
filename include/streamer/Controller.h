/*

  Controller 
  ----------

  A controller takes care of receiving data on it's port and sending 
  it to all the added remotes; nothing more than a basic proxy. In the 
  future we can add more special features.

  You bind the controller to a specific port, then add the remotes that
  you want to be able to receive the data.


 */
#ifndef STREAMER_CONTROLLER_H
#define STREAMER_CONTROLLER_H

#include <string>
#include <streamer/SocketUDP.h>
#include <vector>

namespace rxs {

  /* -------------------------------------------------------------------------- */

  struct Remote {
    std::string ip;
    uint16_t port;
  };

  /* -------------------------------------------------------------------------- */

  class Controller {

  public:
    Controller(std::string ip, uint16_t port);
    ~Controller();
    int init();
    void update();
    void addRemote(std::string rip, uint16_t port);

  public:
    std::string ip;
    uint16_t port;
    Loop loop;
    SocketUDP sock;
    std::vector<Remote> remotes;
  };

} /* namespace rxs */

#endif
