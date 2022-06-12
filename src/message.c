#include "message.h"

void format_msg(char* buf, int id, int src, int dst, int payload[], int payload_size) {
  int index = sprintf(buf, "%02d%02d%02d", id, src, dst);
  
  for (int i=0; i<payload_size; i++) {
    index += sprintf(&buf[index], "%02d", payload[i]);
  }

  sprintf(&buf[index], "\n");
}