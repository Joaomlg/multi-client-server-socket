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

void test_formatMsg_should_withSinglePayloadId_concatIdsInBuffer(void) {
  int id = 1;
  int src = 2;
  int dst = 3;
  int payload_size = 1;
  int payload[1] = {4};

  format_msg(buf, id, src, dst, payload, payload_size);

  const char* expected = "01020304\n";

  TEST_ASSERT_EQUAL_STRING(expected, buf);
}

void test_formatMsg_with_manyPayloadIds_should_concatAllInBuffer(void) {
  int id = 1;
  int src = 2;
  int dst = 3;
  int payload_size = 3;
  int payload[3] = {4, 5, 6};

  format_msg(buf, id, src, dst, payload, payload_size);
  
  const char* expected = "010203040506\n";

  TEST_ASSERT_EQUAL_STRING(expected, buf);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_formatMsg_should_withSinglePayloadId_concatIdsInBuffer);
  RUN_TEST(test_formatMsg_with_manyPayloadIds_should_concatAllInBuffer);
  return UNITY_END();
}