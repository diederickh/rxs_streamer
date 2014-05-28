#include <rxs_streamer/rxs_stun.h>

/* --------------------------------------------------------------------------- */

//static int stun_start(rxs_stun* st);
static void write_be_u16(uint8_t* buffer, uint16_t v);
static void write_be_u32(uint8_t* buffer, uint32_t v);

/* --------------------------------------------------------------------------- */

int rxs_stun_init(rxs_stun* st) {

  if (!st) { return -1; } 

  st->dx = 0;
  st->state = RXS_STUN_STATE_NONE;

  return 0;
}

void rxs_stun_update(rxs_stun* st) {

  /*
  if (st->state == RXS_STUN_STATE_NONE) {
    if (stun_start(st) < 0) {
      printf("Error: couldn't start stun.\n");
      exit(0);
    }
  }
  */

  /* @todo - not sure if we actually need this .. or a state */
}

int rxs_stun_clear(rxs_stun* st) {
  if (!st) { return -1; } 
  return 0;
}

/* --------------------------------------------------------------------------- */

/*
  

 */
int rxs_stun_start(rxs_stun* st) {
  if (!st) { return -1; } 
  st->state = RXS_STUN_STATE_BIND_REQUEST;

  /* bind request */
  uint8_t buf[20];
  uint8_t* ptr = buf;
    
  write_be_u16(ptr, RXS_STUN_BIND_REQUEST);
  write_be_u16(ptr+2, 0x0000);
  write_be_u32(ptr+4, RXS_STUN_MAGIC_COOKIE);
  write_be_u32(ptr+8, random());
  write_be_u32(ptr+12, random());
  write_be_u32(ptr+16, random());

  st->on_send(st, buf, 20);

  return 0;
}

/* --------------------------------------------------------------------------- */

static void write_be_u16(uint8_t* buffer, uint16_t v) {
  uint8_t* val = (uint8_t*)&v;
  buffer[0] = val[1];
  buffer[1] = val[0];
}

static void write_be_u32(uint8_t* buffer, uint32_t v) {
  uint8_t* val = (uint8_t*)&v;
  buffer[0] = val[3];
  buffer[1] = val[2];
  buffer[2] = val[1];
  buffer[3] = val[0];
}
