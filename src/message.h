#pragma once

#include <stdio.h>

#define MAX_PAYLOAD_SIZE 15

enum msg_id {REQ_ADD, REQ_REM, RES_ADD, RES_LIST, REQ_INF, RES_INF, ERROR, OK};

enum error_code {
  EQP_NOT_FOUND,
  SRC_EQP_NOT_FOUND,
  TGT_EQP_NOT_FOUND,
  EQP_LIMIT_EXCEEDED
};

enum ok_code {
  SUCCESSFUL_REMOVAL
};

struct message {
  int id;
  int src;
  int dst;
  int payload[MAX_PAYLOAD_SIZE];
  int payload_size;
};

void encode_msg(char *buf, struct message *msg);