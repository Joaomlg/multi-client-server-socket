#include <stdlib.h>
#include <string.h>

#include "./unity/src/unity.h"
#include "../src/message.h"
#include "../src/common.h"

#pragma region TestSetup

char buf[BUFSZ];

void setUp(void)
{
  memset(buf, 0, BUFSZ);
}

void tearDown(void)
{
}

#pragma endregion

#pragma region encode_msg_tests

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

#pragma endregion

#pragma region decode_msg_tests

void test_decode_msg_without_payload_should_success_decode_buffer(void) {
  struct message *msg = malloc(sizeof(*msg));

  char *buf = "010203\n";

  decode_msg(buf, msg);

  int expected_msg_id = 1;
  int expected_src = 2;
  int expected_dst = 3;
  int expected_payload_size = 0;

  TEST_ASSERT_EQUAL_INT(expected_msg_id, msg->id);
  TEST_ASSERT_EQUAL_INT(expected_src, msg->src);
  TEST_ASSERT_EQUAL_INT(expected_dst, msg->dst);
  TEST_ASSERT_EQUAL_INT(expected_payload_size, msg->payload_size);
}

void test_decode_msg_with_single_id_in_payload_should_success_decode_buffer(void) {
  struct message *msg = malloc(sizeof(*msg));

  char *buf = "01020304\n";

  decode_msg(buf, msg);

  int expected_msg_id = 1;
  int expected_src = 2;
  int expected_dst = 3;
  int expected_payload_size = 1;
  int expected_payload[] = {4};

  TEST_ASSERT_EQUAL_INT(expected_msg_id, msg->id);
  TEST_ASSERT_EQUAL_INT(expected_src, msg->src);
  TEST_ASSERT_EQUAL_INT(expected_dst, msg->dst);
  TEST_ASSERT_EQUAL_INT(expected_payload_size, msg->payload_size);
  TEST_ASSERT_EQUAL_INT_ARRAY(expected_payload, msg->payload, expected_payload_size);
}

void test_decode_msg_with_many_ids_in_payload_should_success_decode_buffer(void) {
  struct message *msg = malloc(sizeof(*msg));

  char *buf = "010203040506\n";

  decode_msg(buf, msg);

  int expected_msg_id = 1;
  int expected_src = 2;
  int expected_dst = 3;
  int expected_payload_size = 3;
  int expected_payload[] = {4, 5 ,6};

  TEST_ASSERT_EQUAL_INT(expected_msg_id, msg->id);
  TEST_ASSERT_EQUAL_INT(expected_src, msg->src);
  TEST_ASSERT_EQUAL_INT(expected_dst, msg->dst);
  TEST_ASSERT_EQUAL_INT(expected_payload_size, msg->payload_size);
  TEST_ASSERT_EQUAL_INT_ARRAY(expected_payload, msg->payload, expected_payload_size);
}

#pragma endregion

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_formatMsg_should_withSinglePayloadId_successformatBuffer);
  RUN_TEST(test_formatMsg_with_manyPayloadIds_should_successFormatBuffer);
  RUN_TEST(test_formatMsg_with_emptyPayload_should_successFormatBuffer);
  
  RUN_TEST(test_decode_msg_without_payload_should_success_decode_buffer);
  RUN_TEST(test_decode_msg_with_single_id_in_payload_should_success_decode_buffer);
  RUN_TEST(test_decode_msg_with_many_ids_in_payload_should_success_decode_buffer);
  return UNITY_END();
}