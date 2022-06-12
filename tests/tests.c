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

void test_encode_msg_with_single_id_in_payload_should_success_format_buffer(void) {
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

void test_encode_msg_with_many_ids_in_payload_should_success_format_buffer(void) {
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

void test_encode_msg_with_empty_payload_should_success_format_buffer(void) {
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

#pragma region get_error_msg_str_tests

void test_get_error_msg_str_tests_with_eqp_not_found_error_code_should_return_right_str(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = ERROR;
  msg->payload_size = 1;
  msg->payload[0] = EQP_NOT_FOUND;

  get_error_msg_str(buf, msg);

  char *expected_str = "Equipment not found";

  TEST_ASSERT_EQUAL_STRING(expected_str, buf);
}

void test_get_error_msg_str_tests_with_src_eqp_not_found_error_code_should_return_right_str(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = ERROR;
  msg->payload_size = 1;
  msg->payload[0] = SRC_EQP_NOT_FOUND;

  get_error_msg_str(buf, msg);

  char *expected_str = "Source equipment not found";

  TEST_ASSERT_EQUAL_STRING(expected_str, buf);
}

void test_get_error_msg_str_tests_with_tgt_eqp_not_found_error_code_should_return_right_str(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = ERROR;
  msg->payload_size = 1;
  msg->payload[0] = TGT_EQP_NOT_FOUND;

  get_error_msg_str(buf, msg);

  char *expected_str = "Target equipment not found";

  TEST_ASSERT_EQUAL_STRING(expected_str, buf);
}

void test_get_error_msg_str_tests_with_eqp_limit_exceeded_error_code_should_return_right_str(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = ERROR;
  msg->payload_size = 1;
  msg->payload[0] = EQP_LIMIT_EXCEEDED;

  get_error_msg_str(buf, msg);

  char *expected_str = "Equipment limit exceeded";

  TEST_ASSERT_EQUAL_STRING(expected_str, buf);
}

#pragma endregion

#pragma region get_success_msg_str_tests

void test_get_success_msg_str_tests_with_successful_removal_success_code_should_return_right_str(void) {
  struct message *msg = malloc(sizeof(*msg));

  msg->id = OK;
  msg->payload_size = 1;
  msg->payload[0] = SUCCESSFUL_REMOVAL;

  get_success_msg_str(buf, msg);

  char *expected_str = "Successful removal";

  TEST_ASSERT_EQUAL_STRING(expected_str, buf);
}

#pragma endregion

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_encode_msg_with_single_id_in_payload_should_success_format_buffer);
  RUN_TEST(test_encode_msg_with_many_ids_in_payload_should_success_format_buffer);
  RUN_TEST(test_encode_msg_with_empty_payload_should_success_format_buffer);
  
  RUN_TEST(test_decode_msg_without_payload_should_success_decode_buffer);
  RUN_TEST(test_decode_msg_with_single_id_in_payload_should_success_decode_buffer);
  RUN_TEST(test_decode_msg_with_many_ids_in_payload_should_success_decode_buffer);

  RUN_TEST(test_get_error_msg_str_tests_with_eqp_not_found_error_code_should_return_right_str);
  RUN_TEST(test_get_error_msg_str_tests_with_src_eqp_not_found_error_code_should_return_right_str);
  RUN_TEST(test_get_error_msg_str_tests_with_tgt_eqp_not_found_error_code_should_return_right_str);
  RUN_TEST(test_get_error_msg_str_tests_with_eqp_limit_exceeded_error_code_should_return_right_str);

  RUN_TEST(test_get_success_msg_str_tests_with_successful_removal_success_code_should_return_right_str);

  return UNITY_END();
}