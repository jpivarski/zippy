#include <utilities_for_tests.h>

int test_alloc_free() {
  /* complete coverage over ZP_SlabAllocator_alloc and ZP_SlabAllocator_free */
  /* correctness verified manually with view_Context */

  ZP_Context_t context;
  ZP_Error_t error;
  error = new_context(&context, 32, 32);
  handle_error(error);

  ZP_SlabPointer_t pointer1;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer1
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer1,
    0x11,
    0x22
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

  ZP_SlabPointer_t pointer2;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer2
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer2,
    0x33,
    0x44
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x33\x00\x00\x00\x44\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

  ZP_SlabPointer_t pointer3;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer3
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer3,
    0x55,
    0x66
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x33\x00\x00\x00\x44\x00\x00\x00\x55\x00\x00\x00\x66\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

  ZP_SlabAllocator_free(
    &context.slab_allocator,
    &pointer2
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\xff\xff\xff\xff\x44\x00\x00\x00\x55\x00\x00\x00\x66\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

  ZP_SlabPointer_t pointer4;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer4
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer4,
    0x77,
    0x88
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x77\x00\x00\x00\x88\x00\x00\x00\x55\x00\x00\x00\x66\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

  ZP_SlabPointer_t pointer5;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer5
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer5,
    0x99,
    0xaa
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x77\x00\x00\x00\x88\x00\x00\x00\x55\x00\x00\x00\x66\x00\x00\x00\x99\x00\x00\x00\xaa\x00\x00\x00");

  ZP_SlabAllocator_free(
    &context.slab_allocator,
    &pointer3
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x77\x00\x00\x00\x88\x00\x00\x00\xff\xff\xff\xff\x66\x00\x00\x00\x99\x00\x00\x00\xaa\x00\x00\x00");

  ZP_SlabAllocator_free(
    &context.slab_allocator,
    &pointer5
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x77\x00\x00\x00\x88\x00\x00\x00\xff\xff\xff\xff\x66\x00\x00\x00\x02\x00\x00\x00\xaa\x00\x00\x00");

  ZP_SlabAllocator_free(
    &context.slab_allocator,
    &pointer4
  );
  assert_Context_slab(&context, 32, "\x11\x00\x00\x00\x22\x00\x00\x00\x03\x00\x00\x00\x88\x00\x00\x00\xff\xff\xff\xff\x66\x00\x00\x00\x02\x00\x00\x00\xaa\x00\x00\x00");

  ZP_SlabAllocator_free(
    &context.slab_allocator,
    &pointer1
  );
  assert_Context_slab(&context, 32, "\x01\x00\x00\x00\x22\x00\x00\x00\x03\x00\x00\x00\x88\x00\x00\x00\xff\xff\xff\xff\x66\x00\x00\x00\x02\x00\x00\x00\xaa\x00\x00\x00");

  ZP_SlabPointer_t pointer6;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer6
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer6,
    0xbb,
    0xcc
  );
  assert_Context_slab(&context, 32, "\xbb\x00\x00\x00\xcc\x00\x00\x00\x03\x00\x00\x00\x88\x00\x00\x00\xff\xff\xff\xff\x66\x00\x00\x00\x02\x00\x00\x00\xaa\x00\x00\x00");

  ZP_SlabPointer_t pointer7;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer7
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer7,
    0xdd,
    0xee
  );
  assert_Context_slab(&context, 32, "\xbb\x00\x00\x00\xcc\x00\x00\x00\xdd\x00\x00\x00\xee\x00\x00\x00\xff\xff\xff\xff\x66\x00\x00\x00\x02\x00\x00\x00\xaa\x00\x00\x00");

  ZP_SlabPointer_t pointer8;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer8
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer8,
    0xff,
    0x11
  );
  assert_Context_slab(&context, 32, "\xbb\x00\x00\x00\xcc\x00\x00\x00\xdd\x00\x00\x00\xee\x00\x00\x00\xff\xff\xff\xff\x66\x00\x00\x00\xff\x00\x00\x00\x11\x00\x00\x00");

  ZP_SlabPointer_t pointer9;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer9
  );
  handle_error(error);
  ZP_SlabAllocator_set(
    &context.slab_allocator,
    &pointer9,
    0x22,
    0x33
  );
  assert_Context_slab(&context, 32, "\xbb\x00\x00\x00\xcc\x00\x00\x00\xdd\x00\x00\x00\xee\x00\x00\x00\x22\x00\x00\x00\x33\x00\x00\x00\xff\x00\x00\x00\x11\x00\x00\x00");

  ZP_SlabPointer_t pointer10;
  error = ZP_SlabAllocator_alloc(
    &context.slab_allocator,
    &pointer10
  );
  handle_not_error(error);

  return 0;
}

int main(int argc, char** argv) {
  int result = 0;

#if ZIPPY_POINTER_SIZE == 32
#if ZIPPY_REFCNT_SIZE == 8

  result = test_alloc_free();
  if (result != 0) {
    return result;
  }

#endif /* ZIPPY_REFCNT_SIZE */
#endif /* ZIPPY_POINTER_SIZE == 32 */

  return result;
}
