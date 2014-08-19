#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#define CONTROLLER_IP "127.0.0.1"
#define CONTROLLER_PORT 6688

#define RECEIVER_IP "127.0.0.1"
#define RECEIVER_PORT 7000

#define SENDER_IP "127.0.0.1"
#define SENDER_PORT 6677

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 240
#define VIDEO_FPS_NUM 1
#define VIDEO_FPS_DEN 15

#define RECORD_TO_IVF 1      /* When set to 1, the video receiver will store the recorded frames into a .ivf file */

#endif
