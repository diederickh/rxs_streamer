#ifndef RXS_TYPES_H
#define RXS_TYPES_H

//#define RXS_RTP_PAYLOAD_SIZE 16371
#define RXS_RTP_PAYLOAD_SIZE 700              /* max bytes of a udp packet */
//#define RXS_RTP_PAYLOAD_SIZE 1010
#define RXS_CONTROL_PORT 5544                  /* default control port */
#define RXS_MAX_MISSING_PACKETS 15
#define RXS_MAX_SPLIT_PACKETS 8                /* over how many rtp packets one VP8 frame can be split */

#endif
