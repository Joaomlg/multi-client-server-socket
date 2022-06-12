#include <stdlib.h>
#include <string.h>

#include "./unity/src/unity.h"
#include "../src/message.h"
#include "../src/common.h"

char buf[BUFSZ];

void setUp(void)
{
  memset(buf, 0, BUFSZ);
}

void tearDown(void)
{
}

void test_formatMsg_should_withSinglePayloadId_successformatBuffer(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = 1;
  msg->src = 2;
  msg->dst = 3;
  
  msg->payload_size = 1;
  
  msg->payload[0] = 4;

  encode_msg(buf, msg);

  const char* expected = "01020304\n";

  TEST_ASSERT_EQUAL_STRING(expected, buf);
}

void test_formatMsg_with_manyPayloadIds_should_successFormatBuffer(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = 1;
  msg->src = 2;
  msg->dst = 3;

  msg->payload_size = 3;

  msg->payload[0] = 4;
  msg->payload[1] = 5;
  msg->payload[2] = 6;

  encode_msg(buf, msg);
  
  const char* expected = "010203040506\n";

  TEST_ASSERT_EQUAL_STRING(expected, buf);
}

void test_formatMsg_with_emptyPayload_should_successFormatBuffer(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = 1;
  msg->src = 2;
  msg->dst = 3;
  msg->payload_size = 0;

  encode_msg(buf, msg);
  
  const char* expected = "010203\n";

  TEST_ASSERT_EQUAL_STRING(expected, buf);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_formatMsg_should_withSinglePayloadId_successformatBuffer);
  RUN_TEST(test_formatMsg_with_manyPayloadIds_should_successFormatBuffer);
  RUN_TEST(test_formatMsg_with_emptyPayload_should_successFormatBuffer);
  return UNITY_END();
}