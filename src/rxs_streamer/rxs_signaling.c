#include <rxs_streamer/rxs_signaling.h>

/* -------------------------------------------------------------------------- */

static void info_callback(redisAsyncContext* c, void* r, void *user);
static void notify_callback(redisAsyncContext* c, void* r, void *user);
static void connect_callback(const struct redisAsyncContext* c, int status);
static void disconnect_callback(const struct redisAsyncContext* c, int status);

/* -------------------------------------------------------------------------- */

int rxs_signal_init(rxs_signal* s, const char* ip, uint16_t port) {
  if (!s)    { return -1; } 
  if (!ip)   { return -2; } 
  if (!port) { return -3; } 

  s->connected = 0;
  s->loop = (uv_loop_t*)uv_default_loop();
  if (!s->loop) {
    printf("Error: cannot get default uv loop for signaling.\n");
    return -4;
  }

  s->redis = redisAsyncConnect(ip, port);
  if (!s->redis) {
    printf("Error: cannot create hiredis context for signaling.\n");
    return -5;
  }

  if (s->redis->err) {
    printf("Error: something went wrong when trying to connect to redis: %s\n", s->redis->errstr);
    return -6;
  }



  /* @todo - add error checks */
  redisLibuvAttach(s->redis, s->loop);
  redisAsyncSetConnectCallback(s->redis, connect_callback);
  redisAsyncSetDisconnectCallback(s->redis, disconnect_callback);

  return 0;
}

int rxs_signal_retrieve_address(rxs_signal* s, int slot) {
  if (!s) { return -1; } 
  redisAsyncCommand(s->redis, info_callback, s, "HMGET signal:%d ip port", slot);
  return 0;
}

int rxs_signal_subscribe(rxs_signal* s, int slot) {
  if (!s) { return -1; } 
  s->slot = slot;
  redisAsyncCommand(s->redis, notify_callback, s, "SUBSCRIBE slot%d", slot);
  return 0;
}

int rxs_signal_update(rxs_signal* s) {
  if (!s) { return -1; } 
  uv_run(s->loop, UV_RUN_NOWAIT);
  return 0;
}

int rxs_signal_store_address(rxs_signal* s, int slot, const char* ip, uint16_t port) {
  if (!s)    { return -1; } 
  if (!ip)   { return -2; } 
  if (!port) { return -3; } 

  redisAsyncCommand(s->redis, NULL, NULL, "PUBLISH slot%d address:%s:%d", slot, ip, port);
  redisAsyncCommand(s->redis, NULL, NULL, "HMSET signal:%d ip %s port %d", slot, ip, port);
  return 0;
}

/* -------------------------------------------------------------------------- */

static void connect_callback(const struct redisAsyncContext* c, int status) {
  /* @todo check status */
  if (status < 0) {
    printf("Error: cannot connect: %s\n", c->errstr);
  }
  else {
    printf("Connected to signaling server..: %d\n", status);
  }
}

static void disconnect_callback(const struct redisAsyncContext* c, int status) {
  printf("Disconnect from signaling server: %d, msg: %s\n", status, c->errstr);
}

static void notify_callback(redisAsyncContext* c, void* r, void *user) {
  int i = 0;
  rxs_signal* s = (rxs_signal*)user;
  redisReply* reply = (redisReply*)r;
  char* command;
  char* ip;
  char* port;
  uint16_t portnr;

  if (!reply) {
    return;
  }

  if (reply->elements != 3) {
    return;
  }

  if (!reply->element[2]->str) {
    return;
  }

  /* extract command */
  command = strtok(reply->element[2]->str, ":");
  if (strcmp(command, "address") != 0) {
    return;
  }

  /* extract ip */
  ip = strtok(NULL, ":");
  if (!ip) {
    return;
  }

  /* extract port */
  port = strtok(NULL, ":");
  if (!port) {
    return;
  }
  portnr = atoi(port);

  /* notify callback */
  if (s->on_address) {
    s->on_address(s, ip, portnr);
  }
}

static void info_callback(redisAsyncContext* c, void* r, void *user) {
  rxs_signal* s = (rxs_signal*)user;
  redisReply* reply = (redisReply*)r;
  uint16_t port;

  if (!reply) {
    return;
  }

  if (reply->elements != 2) {
    return;
  }

  port = atoi(reply->element[1]->str);

  if (s->on_address) {
    s->on_address(s, reply->element[0]->str, port);
  }
}
