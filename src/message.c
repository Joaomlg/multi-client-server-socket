#include "message.h"

void encode_msg(char *buf, struct message *msg) {
  int index = sprintf(buf, "%02d%02d%02d", msg->id, msg->src, msg->dst);
  
  for (int i = 0; i < msg->payload_size; i++) {
    index += sprintf(&buf[index], "%02d", msg->payload[i]);
  }

  sprintf(&buf[index], "\n");
}