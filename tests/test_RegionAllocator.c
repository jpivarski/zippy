#include <test_util.h>

int test_1() {
  ZP_Context_t context;
  ZP_Error_t error;

  error = new_context(&context, 128, 64);
  handle_error(error);

  view_Context(&context, true, true);

  return 0;
}

int main(int argc, char** argv) {
  int result = 0;

  result = test_1();
  if (result != 0) {
    return result;
  }

  return result;
}
