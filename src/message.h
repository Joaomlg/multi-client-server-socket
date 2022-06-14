#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CLIENTS 15
#define ID_BYTE_SIZE 2
#define MSG_END "\n"

enum msg_id {REQ_ADD = 1, REQ_REM, RES_ADD, RES_LIST, REQ_INF, RES_INF, ERROR, OK};

enum error_code {
  EQP_NOT_FOUND = 1,
  SRC_EQP_NOT_FOUND,
  TGT_EQP_NOT_FOUND,
  EQP_LIMIT_EXCEEDED
};

enum ok_code {
  SUCCESSFUL_REMOVAL = 1
};

struct message {
  int id;
  int src;
  int dst;
  int payload[MAX_CLIENTS];
  int payload_size;
};

void encode_msg(char *buf, struct message *msg);

void decode_msg(char *buf, struct message *msg);

void get_error_msg_str(char *buf, struct message *msg);

void get_success_msg_str(char *buf, struct message *msg);

void build_error_msg(struct message *msg, int code, int dst);

void build_req_add_msg(struct message *msg);

void build_res_add_msg(struct message *msg, int eqp_id);

void build_res_list_msg(struct message *msg, int eqp_id[], int size);

void build_req_rem_msg(struct message *msg, int src);