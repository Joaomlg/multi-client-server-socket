#include "message.h"

void encode_msg(char *buf, struct message *msg) {
  int index = sprintf(buf, "%02d%02d%02d", msg->id, msg->src, msg->dst);
  
  for (int i = 0; i < msg->payload_size; i++) {
    index += sprintf(&buf[index], "%02d", msg->payload[i]);
  }

  sprintf(&buf[index], MSG_END);
}

void decode_msg(char *buf, struct message *msg) {
  char tmp[ID_BYTE_SIZE * MAX_CLIENTS];

  int id_offset = 0;
  strncpy(tmp, buf + id_offset, ID_BYTE_SIZE);
  msg->id = atoi(tmp);

  int src_offset = id_offset + ID_BYTE_SIZE;
  strncpy(tmp, buf + src_offset, ID_BYTE_SIZE);
  msg->src = atoi(tmp);

  int dst_offset = src_offset + ID_BYTE_SIZE;
  strncpy(tmp, buf + dst_offset, ID_BYTE_SIZE);
  msg->dst = atoi(tmp);

  int payload_offset = dst_offset + ID_BYTE_SIZE;

  msg->payload_size = (strlen(buf) - payload_offset) / ID_BYTE_SIZE;

  for (int i = 0; i < msg->payload_size; i++) {
    int offset = payload_offset + i * ID_BYTE_SIZE;
    strncpy(tmp, buf + offset, ID_BYTE_SIZE);
    msg->payload[i] = atoi(tmp);
  }
}

void get_error_msg_str(char *buf, struct message *msg) {
  int error_code = msg->payload[0];

  switch (error_code) {
    case EQP_NOT_FOUND:
      sprintf(buf, "Equipment not found");
      break;
    case SRC_EQP_NOT_FOUND:
      sprintf(buf, "Source equipment not found");
      break;
    case TGT_EQP_NOT_FOUND:
      sprintf(buf, "Target equipment not found");
      break;
    case EQP_LIMIT_EXCEEDED:
      sprintf(buf, "Equipment limit exceeded");
      break;
    default:
      sprintf(buf, "Unknown error code: %d", error_code);
  }
}

void get_success_msg_str(char *buf, struct message *msg) {
  int success_code = msg->payload[0];

  switch (success_code) {
    case SUCCESSFUL_REMOVAL:
      sprintf(buf, "Successful removal");
      break;
    default:
      sprintf(buf, "Unknown success code: %d", success_code);
  }
}

void build_error_msg(struct message *msg, int code, int dst) {
  msg->id = ERROR;
  msg->src = 0;
  msg->dst = dst;
  msg->payload_size = 1;
  msg->payload[0] = code;
}

void build_req_add_msg(struct message *msg) {
  msg->id = REQ_ADD;
  msg->src = 0;
  msg->dst = 0;
  msg->payload_size = 0;
}

void build_res_add_msg(struct message *msg, int eqp_id) {
  msg->id = RES_ADD;
  msg->src = 0;
  msg->dst = 0;
  msg->payload_size = 1;
  msg->payload[0] = eqp_id;
}

void build_res_list_msg(struct message *msg, int eqp_id[], int size) {
  msg->id = RES_LIST;
  msg->src = 0;
  msg->dst = 0;
  msg->payload_size = size;
  
  for (int i=0; i<size; i++) {
    msg->payload[i] = eqp_id[i];
  }
}