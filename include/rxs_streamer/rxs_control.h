/*
  
  rxs_control
  -----------

  rxs_control is used by the receiver of the video stream to 
  request lost packets or to request new keyframes. This is a 
  very simple implementation because this whole library is meant
  for simple point-to-point video streaming and doesn't need
  a full blown "RTCP / RTP Audio/Video Profile with Feedback"
  implementation.

  This file implements both the sender, receiver and depacketizer
  of control packets.:

  - rxs_control_sender:   used to send control packets
  - rxs_control_receiver: used to receive and parse packets. 

  +----------------+---+---+--------+
  | 0           16 |  17   | 16 * C |  bytes
  +----------------+---+---+--------+ 
  |     S          | T | C | P      |  meaning
  +----------------+---+---+--------+

  Bytes 0,1:
  ----------
  - S:  Sequence number of the control packet, 16 bytes, network byte order

  Byte 3:
  --------
  - T:  Task  - 4 bytes
  - C:  Count - number of sequence numbers, used in case 
                when the receiver requests some losts packets. 
  
  Bytes 4 ...
  ------------
  - Depending on the command, but normally a sequence of 
    packet sequence numbers.


*/
#ifndef RXS_CONTROL_H
#define RXS_CONTROL_H

#include <stdint.h>
#include <uv.h>
#include <rxs_streamer/rxs_control.h> /* @todo - what?? */

#define RXS_CONTROL_MAX_SEQNUM 15                                                 /* we don't allow more then 15 missed packets */
#define RXS_CONTROL_COMMAND_KEY_FRAME 1                                           /* command to request a new keyframe */
#define RXS_CONTROL_COMMAND_RESEND 2                                              /* command to request resending of missing sequence numbers */

typedef struct rxs_control_sender rxs_control_sender;
typedef struct rxs_control_receiver rxs_control_receiver;
typedef struct rxs_control rxs_control;

typedef void(*rxs_control_receiver_callback)(rxs_control_receiver* rec);           /* gets called when we received and parsed a command */

struct rxs_control_sender {

  /* networking */
  struct sockaddr_in saddr;
  uv_loop_t* loop;
  uv_udp_t sock;
  uint8_t buffer[2048];
  uint16_t dx;                                                                      /* offset in the buffer */

  /* protocol */
  uint16_t nr;                                                                      /* number of sent commands */
  uint16_t seqnums[RXS_CONTROL_MAX_SEQNUM];                                         /* can be filled with sequence number you want to request */ 
};

struct rxs_control_receiver {

  /* networking */
  struct sockaddr_in saddr;
  uv_loop_t* loop;
  uv_udp_t sock;

  /* protocol */
  uint8_t command;
  uint8_t count;
  uint16_t nr;
  uint16_t seqnums[RXS_CONTROL_MAX_SEQNUM];

  /* callback */
  void* user;
  rxs_control_receiver_callback on_command;
};

/* sender interface */
int rxs_control_sender_init(rxs_control_sender* s, const char* ip, int port);
void rxs_control_sender_update(rxs_control_sender* s);
int rxs_control_sender_request_keyframe(rxs_control_sender* s);
int rxs_control_sender_request_packets(rxs_control_sender* s, uint16_t* seqnums, int num); /* request to resend some packets, this will use the seqnum member */

/* receiver interface */
int rxs_control_receiver_init(rxs_control_receiver* r, int port);
void rxs_control_receiver_update(rxs_control_receiver* r);

#endif
