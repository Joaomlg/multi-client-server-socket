#pragma once

#include <stdio.h>

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

void format_msg(char* buf, int id, int src, int dst, int payload[], int payload_size);