#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <rxs_streamer/rxs_control.h>

#define RXS_PORT 4455

rxs_control_sender sender;
rxs_control_receiver receiver;

static void on_command(rxs_control_receiver* rec);

static void sigh(int s);

int main() {

  int i = 0;
  int num = 4; /* ask to resend 4 packets */

  printf("\n\nrxs_control\n\n");

  signal(SIGINT, sigh);

  if (rxs_control_sender_init(&sender, "0.0.0.0", RXS_PORT) < 0) {
    exit(1);
  }

  if (rxs_control_receiver_init(&receiver, RXS_PORT) < 0) {
    exit(1);
  }

  receiver.on_command = on_command;

  /* test some keyframe requests */
  rxs_control_sender_request_keyframe(&sender);
  rxs_control_sender_request_keyframe(&sender);
  rxs_control_sender_request_keyframe(&sender);

  /* test some packets request */
  for (i = 0; i < num; ++i) {
    sender.seqnums[i] = rand();
    printf("Requesting: %d\n", sender.seqnums[i]);
  }
  rxs_control_sender_request_packets(&sender, sender.seqnums, num);

  while(1) {
    rxs_control_receiver_update(&receiver);
    rxs_control_sender_update(&sender);
  }

  printf("\n");
  return 0;
}

static void sigh(int s) {
  printf("\n\nGot signal!\n\n");
  exit(0);
}

static void on_command(rxs_control_receiver* rec) {

  int i = 0;

  printf("Command: %d, nr: %d,  count: %d\n", 
         rec->command,
         rec->nr, 
         rec->count);

  switch (rec->command) {
    case RXS_CONTROL_COMMAND_KEY_FRAME: {
      printf("- Receiver wants a new keyframe.\n");
      break;
    }
    case RXS_CONTROL_COMMAND_RESEND: {
      printf(" - Receiver wants the sender to resend the following packets: \n");
      for (i = 0; i < rec->count; ++i) {
        printf(" -- %d\n", rec->seqnums[i]);
      }
      break;
    }
    default: {
      printf(" - Unhandled commands.\n");
      break;
    }
  }
}
