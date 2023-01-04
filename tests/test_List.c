#include <test_util.h>

int test_push() {
  ZP_Context_t context;
  ZP_Error_t error;
  error = new_context(&context, 32, 32);
  handle_error(error);

  ZP_Tagged_t tagged_int1;
  ZP_Int_t int1 = 0x11;
  ZP_Tagged_from_int(&tagged_int1, &int1);

  ZP_Tagged_t tagged_int2;
  ZP_Int_t int2 = 0x22;
  ZP_Tagged_from_int(&tagged_int2, &int2);

  ZP_Tagged_t tagged_int3;
  ZP_Int_t int3 = 0x33;
  ZP_Tagged_from_int(&tagged_int3, &int3);

  ZP_Tagged_t list1;
  ZP_List_new_empty(&list1);

  ZP_Tagged_t list2;
  error = ZP_List_push(&context, &list2, &list1, &tagged_int1);
  handle_error(error);

  ZP_Tagged_t list3;
  error = ZP_List_push(&context, &list3, &list2, &tagged_int2);
  handle_error(error);

  ZP_Tagged_t list4;
  error = ZP_List_push(&context, &list4, &list3, &tagged_int3);
  handle_error(error);

  assert_Context_refcnt(&context, 4, "\x02\x02\x01\x00");
  assert_Context_slab(&context, 32, "\xff\xff\xff\x1f\x11\x00\x00\x40\x00\x00\x00\x10\x22\x00\x00\x40\x01\x00\x00\x10\x33\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00");

  ZP_Context_slab_decref(&context, &list1);
  ZP_Context_slab_decref(&context, &list2);
  ZP_Context_slab_decref(&context, &list3);

  assert_Context_refcnt(&context, 4, "\x01\x01\x01\x00");
  assert_Context_slab(&context, 32, "\xff\xff\xff\x1f\x11\x00\x00\x40\x00\x00\x00\x10\x22\x00\x00\x40\x01\x00\x00\x10\x33\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00");

  /* list4 is the only reference; deleting it cascades down the whole list */
  ZP_Context_slab_decref(&context, &list4);

  /* build it up again */
  ZP_List_new_empty(&list1);

  error = ZP_List_push(&context, &list2, &list1, &tagged_int1);
  handle_error(error);

  error = ZP_List_push(&context, &list3, &list2, &tagged_int2);
  handle_error(error);

  error = ZP_List_push(&context, &list4, &list3, &tagged_int3);
  handle_error(error);

  assert_Context_refcnt(&context, 4, "\x02\x02\x01\x00");
  assert_Context_slab(&context, 32, "\xff\xff\xff\x1f\x11\x00\x00\x40\x00\x00\x00\x10\x22\x00\x00\x40\x01\x00\x00\x10\x33\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00");

  /* but this time, only let the cascade go part-way */
  ZP_Context_slab_decref(&context, &list3);
  ZP_Context_slab_decref(&context, &list4);
  assert_Context_refcnt(&context, 4, "\x01\x00\x00\x00");
  assert_Context_slab(&context, 32, "\xff\xff\xff\x1f\x11\x00\x00\x40\x02\x00\x00\x00\x22\x00\x00\x40\xff\xff\xff\xff\x33\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00");

  return 0;
}

int test_copy() {
  ZP_Context_t context;
  ZP_Error_t error;
  error = new_context(&context, 32, 32);
  handle_error(error);

  ZP_Tagged_t tagged_int1;
  ZP_Int_t int1 = 0x11;
  ZP_Tagged_from_int(&tagged_int1, &int1);

  ZP_Tagged_t tagged_int2;
  ZP_Int_t int2 = 0x22;
  ZP_Tagged_from_int(&tagged_int2, &int2);

  ZP_Tagged_t tagged_int3;
  ZP_Int_t int3 = 0x33;
  ZP_Tagged_from_int(&tagged_int3, &int3);

  ZP_Tagged_t list1;
  ZP_List_new_empty(&list1);

  ZP_Tagged_t list2;
  error = ZP_List_push(&context, &list2, &list1, &tagged_int1);
  handle_error(error);

  ZP_Tagged_t list3;
  error = ZP_List_push(&context, &list3, &list2, &tagged_int2);
  handle_error(error);

  ZP_Tagged_t list4;
  error = ZP_List_push(&context, &list4, &list3, &tagged_int3);
  handle_error(error);

  assert_Context_refcnt(&context, 4, "\x02\x02\x01\x00");
  assert_Context_slab(&context, 32, "\xff\xff\xff\x1f\x11\x00\x00\x40\x00\x00\x00\x10\x22\x00\x00\x40\x01\x00\x00\x10\x33\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00");

  ZP_Tagged_t list4_copy;

  ZP_Index_t i;
  for (i = 0;  i < 254;  i++) {
    error = ZP_Context_slab_incref(&context, &list4_copy, &list4);
    handle_error(error);
  }

  assert_Context_refcnt(&context, 4, "\x02\x02\xff\x00");
  assert_Context_slab(&context, 32, "\xff\xff\xff\x1f\x11\x00\x00\x40\x00\x00\x00\x10\x22\x00\x00\x40\x01\x00\x00\x10\x33\x00\x00\x40\x00\x00\x00\x00\x00\x00\x00\x00");

  error = ZP_Context_slab_incref(&context, &list4_copy, &list4);
  handle_error(error);

  assert_Context_refcnt(&context, 4, "\x02\x03\xff\x01");
  assert_Context_slab(&context, 32, "\xff\xff\xff\x1f\x11\x00\x00\x40\x00\x00\x00\x10\x22\x00\x00\x40\x01\x00\x00\x10\x33\x00\x00\x40\x01\x00\x00\x10\x33\x00\x00\x40");

  return 0;
}

int main(int argc, char** argv) {
  int result = 0;

#if ZIPPY_POINTER_SIZE == 32
#if ZIPPY_REFCNT_SIZE == 8

  result = test_push();
  if (result != 0) {
    return result;
  }

  result = test_copy();
  if (result != 0) {
    return result;
  }

#endif /* ZIPPY_REFCNT_SIZE */
#endif /* ZIPPY_POINTER_SIZE == 32 */

  return result;
}
