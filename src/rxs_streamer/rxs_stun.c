#include <string.h>
#include <rxs_streamer/rxs_stun.h>

/* --------------------------------------------------------------------------- */

static void print_id(uint32_t* id);            /* print the id (array of 3 uint32_t), used for debugging */
static void print_cookie(uint32_t* cookie);    /* print the cookie */

static int parse_attribute(uint8_t** buf, rxs_stun_attr* attr);
static int parse_attr_mapped_address(uint8_t** buf, rxs_stun_attr* attr);
static int parse_attr_xor_mapped_address(uint8_t** buf, rxs_stun_attr* attr);
static const char* attr_type_to_string(uint16_t type);

static void write_u8(uint8_t** buffer, uint8_t v);      /* note: we're moving the pointer */
static void write_be_u16(uint8_t** buffer, uint16_t v); /* note: we're moving the pointer */
static void write_be_u32(uint8_t** buffer, uint32_t v); /* note: we're moving the pointer */

static uint8_t read_u8(uint8_t** buffer);       /* note: we're moving the pointer */
static uint16_t read_be_u16(uint8_t** buffer);  /* note: we're moving the pointer */
static uint32_t read_be_u32(uint8_t** buffer);  /* note: we're moving the pointer */

/* --------------------------------------------------------------------------- */

int rxs_stun_init(rxs_stun* st) {

  if (!st) { return -1; } 

  st->pos = 0;
  st->capacity = 4096;
  st->transid[0] = random();
  st->transid[1] = random();
  st->transid[2] = random();

  return 0;
}

int rxs_stun_clear(rxs_stun* st) {
  if (!st) { return -1; } 
  return 0;
}

/* --------------------------------------------------------------------------- */

int rxs_stun_start(rxs_stun* st) {
  if (!st) { return -1; } 

  /* bind request */
  uint8_t buf[20];
  uint8_t* ptr = buf;
    
  write_be_u16(&ptr, RXS_STUN_BIND_REQUEST);
  write_be_u16(&ptr, 0x0000);
  write_be_u32(&ptr, RXS_STUN_MAGIC_COOKIE);
  write_be_u32(&ptr, st->transid[0]);
  write_be_u32(&ptr, st->transid[1]);
  write_be_u32(&ptr, st->transid[2]);

  st->on_send(st, buf, 20);

  return 0;
}

int rxs_stun_creating_binding_indication(rxs_stun* st) {
  return 0;
}

int rxs_stun_process(rxs_stun* st, uint8_t* data, uint32_t nbytes) {

  int i;
  uint8_t* ptr = NULL;
  uint8_t* tmp = NULL; /* used to log some debuf info */
  uint16_t msg_type = 0;
  uint16_t msg_length = 0;
  uint32_t msg_cookie = 0;
  uint32_t msg_id[3] = { 0 } ;
  int64_t bytes_left = 0;
  rxs_stun_attr attr;
  uint8_t send_buf[32];         /* used to reply on a STUN_MAPPED_ADDRESS */
  uint8_t* send_ptr = send_buf; /* used when writing our response */

  if (!st) { return -1; } 
  if (!data) { return -2; } 
  if (!nbytes) { return -3; } 

  bytes_left = st->capacity - st->pos;
  if (bytes_left < nbytes) {
    /* not supposed to happen */
    printf("Error: our internal buffer is not large enough in rxs_stun.\n");
    return -1; 
  }

  /* copy data into our parse buffer */
  memcpy(st->buffer + st->pos, data, nbytes);
  ptr = st->buffer + st->pos;
  st->pos += nbytes;

  /* not a complete header yet */
  if (st->pos < 20) {
    return nbytes;;
  }

  msg_type   = read_be_u16(&ptr);
  msg_length = read_be_u16(&ptr);
  msg_cookie = read_be_u32(&ptr);
  msg_id[0]  = read_be_u32(&ptr);
  msg_id[1]  = read_be_u32(&ptr);
  msg_id[2]  = read_be_u32(&ptr);

   tmp = (uint8_t*)&msg_cookie;

  switch(msg_type) {
    case RXS_STUN_BIND_RESPONSE: {
      
      printf("> stun bind response\n");
      printf("> stun msg length: %d\n", msg_length);
      print_cookie(&msg_cookie);
      print_id(msg_id);

      if(parse_attribute(&ptr, &attr) < 0) {
        printf("XXX Error: cannot parse STUN_BIND_RESPONSE.\n");
      }
      else {
        if(attr.type == RXS_STUN_MAPPED_ADDRESS) {

          /* stun header (20 bytes) */
          write_be_u16(&send_ptr, RXS_STUN_BIND_RESPONSE);
          write_be_u16(&send_ptr, 12);
          write_be_u32(&send_ptr, RXS_STUN_MAGIC_COOKIE);
          write_be_u32(&send_ptr, msg_id[0]);
          write_be_u32(&send_ptr, msg_id[1]);
          write_be_u32(&send_ptr, msg_id[2]);

          /* mapped address */
          write_be_u16(&send_ptr, RXS_STUN_MAPPED_ADDRESS);
          write_be_u16(&send_ptr, 1);
          write_u8(    &send_ptr, 0x00);
          write_u8(    &send_ptr, (attr.address.sin_family == AF_INET) ? 0x01 : 0x02);
          write_be_u16(&send_ptr, attr.address.sin_port);
          write_be_u32(&send_ptr, attr.address.sin_addr.s_addr);

          st->on_send(st, send_buf, 32);
        }
      }
      break;
    };
    default: {
      printf("Unhandled STUN message.\n");
      return -1;
    }
  }

  return 0;
}

/* --------------------------------------------------------------------------- */

int parse_attribute(uint8_t** buf, rxs_stun_attr* attr) {

  uint16_t attr_type = 0;
  uint16_t attr_length = 0;
  
  attr_type = read_be_u16(buf);
  attr_length = read_be_u16(buf);

  printf("> stun.attribute.type: %d, %s\n", attr_type, attr_type_to_string(attr_type));
  printf("> stun.attribute.length: %d\n", attr_length);

  switch(attr_type) {
    case RXS_STUN_MAPPED_ADDRESS: { 
      parse_attr_mapped_address(buf, attr);
      break;
    }
    case RXS_STUN_XOR_MAPPED_ADDRESS: {
      parse_attr_xor_mapped_address(buf, attr);
      break;
    }
    default: {
      printf("> stun.attribute.type: UNKNOWN\n");
      return -1;
    }
  }
  return 0;
}

int parse_attr_mapped_address(uint8_t** buf, rxs_stun_attr* attr) {

  int i;
  uint8_t family;
  uint16_t port;
  uint32_t ip;
  unsigned char addr[16];
  
  read_u8(buf); /* padded on 32bit */
  family = read_u8(buf);
  port = read_be_u16(buf);

  if(family != 0x01) {
    /* @todo(roxlu): add IP6 support in parse_attr_mapped_address(). */;
    printf("Error: rxs_stun only handles IP4 for now.\n");
    exit(0);
  }

  if(family == 0x01) {
    ip = read_be_u32(buf);
  }
  else {
    /* @todo - ip6 */
  }

  for(i = 0; i < 4; ++i) {
    addr[i] = (ip >> (i * 8)) & 0xFF;
  }

  printf("> stun.attribute.type: MAPPED-ADDRESS\n");
  printf("> stun.attribute.family: %s (%02X)\n", (family == 0x01) ? "IP4" : "IP6", family);
  printf("> stun.attribute.port: %d\n", port);
  printf("> stun.attribute.ip: %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);

  attr->type = RXS_STUN_MAPPED_ADDRESS;
  attr->address.sin_family = AF_INET;
  attr->address.sin_port = port;
  attr->address.sin_addr.s_addr = ip;

  return 0;
}

/* see: http://tools.ietf.org/html/rfc5389#section-15.2 */
int parse_attr_xor_mapped_address(uint8_t** buf, rxs_stun_attr* attr) {

  printf("> stun.attribute.type: XOR-MAPPED-ADDRESS\n");

  int i;
  uint8_t family;
  uint16_t port;
  uint32_t ip;
  unsigned char addr[16];
  uint32_t cookie = 0x2112A442;
  
  read_u8(buf); /* padded on 32bit */
  family = read_u8(buf);
  port = read_be_u16(buf);

  if(family != 0x01) {
    /* @todo(roxlu): add IP6 support in parse_attr_mapped_address(). */;
    printf("Error: krx_stunc only handles IP4 for now.\n");
    exit(0);
  }

  if(family == 0x01) {
    ip = read_be_u32(buf);
  }
  else {
    /* @todo - ip6 */
  }

  uint8_t* ip_ptr = (uint8_t*)&ip;
  uint8_t* port_ptr = (uint8_t*)&port;
  uint8_t* key_ptr = (uint8_t*)&cookie;

  /* xor */
  for(i = 0; i < 2; ++i) {
    port_ptr[i] = port_ptr[i] ^ key_ptr[2 + i];
  }

  for(i = 0; i < 4; ++i) {
    ip_ptr[i] = ip_ptr[i] ^ key_ptr[i];
  }

  /* make writable */
  for(i = 0; i < 4; ++i) {
    addr[i] = (ip >> (i * 8)) & 0xFF;
  }

  printf("> stun.attribute.port: %d\n", port);
  printf("> stun.attribute.ip: %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);

  attr->type = RXS_STUN_XOR_MAPPED_ADDRESS;

  /* @todo - do something with the stun result ^.^ */
}

/* --------------------------------------------------------------------------- */

static void write_u8(uint8_t** buffer, uint8_t v) {
  uint8_t* val = (uint8_t*)&v;
  uint8_t* ptr = *buffer;
  ptr[0] = val[0];
  *buffer = *buffer + 1;
}

static void write_be_u16(uint8_t** buffer, uint16_t v) {
  uint8_t* val = (uint8_t*)&v;
  uint8_t* ptr = *buffer;
  ptr[0] = val[1];
  ptr[1] = val[0];
  *buffer = *buffer + 2;
}

static void write_be_u32(uint8_t** buffer, uint32_t v) {
  uint8_t* val = (uint8_t*)&v;
  uint8_t* ptr = *buffer;
  ptr[0] = val[3];
  ptr[1] = val[2];
  ptr[2] = val[1];
  ptr[3] = val[0];
  *buffer = *buffer + 4;
}

static uint8_t read_u8(uint8_t** buffer) {
  uint8_t* ptr = *buffer;
  uint8_t result = 0;
  uint8_t* v = (uint8_t*)&result;
  v[0] = ptr[0];
  *buffer = ptr + 1;
  return result;
}

static uint16_t read_be_u16(uint8_t** buffer) {
  uint8_t* ptr = *buffer;
  uint16_t result = 0;
  uint8_t* v = (uint8_t*)&result;
  v[0] = ptr[1];
  v[1] = ptr[0];
  *buffer = ptr + 2;
  return result;
}

static uint32_t read_be_u32(uint8_t** buffer) {
  uint8_t* ptr = *buffer;
  uint32_t result = 0;
  uint8_t* v = (uint8_t*)&result;
  v[0] = ptr[3];
  v[1] = ptr[2];
  v[2] = ptr[1];
  v[3] = ptr[0];
  *buffer = ptr + 4;
  return result;
}

/* --------------------------------------------------------------------------- */

static const char* attr_type_to_string(uint16_t type) {
  switch (type) {
    case RXS_STUN_MAPPED_ADDRESS: { return "RXS_STUN_MAPPED_ADDRESS"; } 
    case RXS_STUN_XOR_MAPPED_ADDRESS: { return "RXS_STUN_XOR_MAPPED_ADDRESS"; } 
    default: { return "UNKNOWN_ATTRIBUTE_TYPE"; } 
  };
}

static void print_id(uint32_t* id) {
  int i, j;
  uint8_t* ptr;

  if (!id) { return; } 

  printf("> stun.id: ");
  for (i = 0; i < 3; ++i) {
    ptr = (uint8_t*)&id[i];
    for (j = 0; j < 4; ++j) {
      printf("%x", ptr[3-j]);
    }
    printf(".");
  }

  printf("\n");
}

static void print_cookie(uint32_t* cookie) {
  int i;
  uint8_t* ptr = (uint8_t*)cookie;

  if (!cookie) { return ; }   

  printf("> stun.cookie: ");

  for (i = 0; i < 4; ++i) {
    printf("%x", ptr[3-i]);
  }

  printf("\n");
}
