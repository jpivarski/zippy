#include <test_util.h>

int test_symbol_set() {
  ZP_Context_t context;
  ZP_Error_t error;
  error = new_context(&context, 64, 48);
  handle_error(error);

  view_Context(&context, true, true);
  printf("\n===========================\n\n");

  ZP_Tagged_t symbol1;
  const char* symbol1_data = "one";
  error = ZP_Context_get_symbol(&context, &symbol1, symbol1_data, strlen(symbol1_data));
  handle_error(error);

  view_Context(&context, true, true);
  printf("\n===========================\n\n");

  ZP_Tagged_t symbol2;
  const char* symbol2_data = "two";
  error = ZP_Context_get_symbol(&context, &symbol2, symbol2_data, strlen(symbol2_data));
  handle_error(error);

  view_Context(&context, true, true);
  printf("\n===========================\n\n");

  ZP_Tagged_t symbol3;
  const char* symbol3_data = "three";
  error = ZP_Context_get_symbol(&context, &symbol3, symbol3_data, strlen(symbol3_data));
  handle_error(error);

  view_Context(&context, true, true);
  printf("\n===========================\n\n");

  ZP_Tagged_t symbol2_again;
  error = ZP_Context_get_symbol(&context, &symbol2_again, symbol2_data, strlen(symbol2_data));
  handle_error(error);

  view_Context(&context, true, true);
  printf("\n===========================\n\n");

  assert(symbol2.data, symbol2_again.data);

  ZP_Context_slab_decref(&context, &symbol1);




  /* state_Context_refcnt(&context); */
  /* state_Context_slab(&context); */
  /* state_Context_region(&context); */


  return 0;
}

int main(int argc, char** argv) {
  int result = 0;

#if ZIPPY_POINTER_SIZE == 32
#if ZIPPY_REFCNT_SIZE == 8

  result = test_symbol_set();
  if (result != 0) {
    return result;
  }

#endif /* ZIPPY_REFCNT_SIZE */
#endif /* ZIPPY_POINTER_SIZE == 32 */

  return result;
}
