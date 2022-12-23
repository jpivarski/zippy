#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <zippy/data.h>

#define handle_error(error)        \
  if (error != ZP_NO_ERROR) {      \
    printf("error: %s\n", error);  \
    return -1;                     \
  }                                \

#define handle_not_error(error)    \
  if (error == ZP_NO_ERROR) {      \
    printf("didn't encounter expected error\n"); \
    return -1;                     \
  }                                \

#define assert(obtained, expected) \
  if ((obtained) != (expected)) {  \
    printf("line %d: expected %x but obtained %x\n", __LINE__, expected, obtained); \
    return -1;                     \
  }                                \

void init(void* arena, size_t arena_size) {
  ZP_Index_t i;
  for (i = 0;  i < arena_size;  i++) {
    ((unsigned char*)arena)[i] = 0;
  }
}

void view_region(ZP_Context_t* context) {
  ZP_Index_t i;
  for (i = 0;  i < context->region_allocator.arena_size;  i++) {
    unsigned char x = ((unsigned char*)context->region_allocator.arena)[i];
    if (x == 0) {
      printf("__ ");
    }
    else if (('A' <= x  &&  x <= 'Z')  ||  ('a' <= x  &&  x <= 'z')  ||  ('0' <= x  &&  x <= '9')) {
      printf("'%c ", x);
    }
    else {
      printf("%02x ", x);
    }
  }
  printf("\n");
}

void view_fixed(ZP_Context_t* context) {
  ZP_Index_t i;
  for (i = 0;  i < context->fixed_allocator.num_pairs * 2 * sizeof(ZP_Index_t);  i++) {
    unsigned char x = ((unsigned char*)context->fixed_allocator.arena)[i];
    if (x == 0) {
      printf("__");
    }
    else {
      printf("%02x", x);
    }
    if ((i + 1) % (2 * sizeof(ZP_Index_t)) == 0) {
      printf("|");
    }
    else if ((i + 1) % sizeof(ZP_Index_t) == 0) {
      printf(" ");
    }
    else {
      printf(".");
    }
  }
  printf("\n");
}

void view_refcount(ZP_Context_t* context) {
  ZP_Index_t i;
  printf("refcnt ");
  for (i = 0;  i < context->fixed_allocator.num_pairs;  i++) {
    printf("%3d ", ((unsigned char*)context->refcount)[i]);
  }
  printf("\n");
}

ZP_Error_t new_context(ZP_Context_t* context) {
  size_t fixed_size = 128;
  void* fixed_arena = malloc(fixed_size);
  init(fixed_arena, fixed_size);

  size_t region_size = 64;
  void* region_arena = malloc(region_size);
  init(region_arena, region_size);

  size_t refcount_size = 16;
  void* refcount_arena = malloc(refcount_size);

  return ZP_Context_new(
    context,
    fixed_arena,
    region_arena,
    refcount_arena,
    fixed_size,
    region_size,
    refcount_size
  );
}

int test_RegionAllocator() {
  printf("BEGIN test_RegionAllocator\n\n");

  ZP_Context_t context;
  ZP_Error_t error = new_context(&context);
  handle_error(error);

  view_region(&context);
  printf("\n");

  ZP_Index_t pointer_one;
  ZP_Index_t request_size_one = 11;
  error = ZP_RegionAllocator_alloc(&context.region_allocator, request_size_one, &pointer_one);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&context.region_allocator, pointer_one, request_size_one, "hello there");
  view_region(&context);
  printf("\n");

  ZP_Index_t pointer_two;
  ZP_Index_t request_size_two = 8;
  error = ZP_RegionAllocator_alloc(&context.region_allocator, request_size_two, &pointer_two);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&context.region_allocator, pointer_two, request_size_two, "you guys");
  view_region(&context);
  printf("\n");

  ZP_Index_t pointer_three;
  ZP_Index_t request_size_three = 23;
  error = ZP_RegionAllocator_alloc(&context.region_allocator, request_size_three, &pointer_three);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&context.region_allocator, pointer_three, request_size_three, "WHAT SHOULD HAPPEN NOW?");
  view_region(&context);
  printf("\n");

  ZP_RegionAllocator_free(&context.region_allocator, pointer_two, request_size_two);
  view_region(&context);
  printf("\n");

  ZP_Index_t pointer_four;
  ZP_Index_t request_size_four = 15;
  error = ZP_RegionAllocator_alloc(&context.region_allocator, request_size_four, &pointer_four);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&context.region_allocator, pointer_four, request_size_four, "Something else.");
  view_region(&context);
  printf("\n");

  ZP_RegionAllocator_free(&context.region_allocator, pointer_three, request_size_three);
  view_region(&context);
  printf("\n");

  error = ZP_RegionAllocator_alloc(&context.region_allocator, request_size_three, &pointer_three);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&context.region_allocator, pointer_three, request_size_three, "WHAT SHOULD HAPPEN NOW?");
  view_region(&context);
  printf("\n");

  error = ZP_RegionAllocator_alloc(&context.region_allocator, request_size_two, &pointer_two);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&context.region_allocator, pointer_two, request_size_two, "you guys");
  view_region(&context);
  printf("\n");

  printf("END test_RegionAllocator\n");

  return 0;
}

int test_FixedAllocator() {
  printf("BEGIN test_FixedAllocator\n\n");

  ZP_Context_t context;
  ZP_Error_t error = new_context(&context);
  handle_error(error);

  view_fixed(&context);
  printf("\n");

  ZP_FixedLocation_t fixed1;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed1);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed1, 5, 6);
  view_fixed(&context);
  printf("\n");

  ZP_Index_t one;
  ZP_Index_t two;
  ZP_FixedAllocator_get(&context.fixed_allocator, &fixed1, &one, &two);
  assert(one, 5);
  assert(two, 6);

  ZP_FixedLocation_t fixed2;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed2);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed2, 0xeeeeee01, 0xeeeeee02);
  view_fixed(&context);
  printf("\n");

  ZP_FixedLocation_t fixed3;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed3);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed3, 0xdddddd01, 0xdddddd02);
  view_fixed(&context);
  printf("\n");

  ZP_FixedLocation_t fixed4;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed4);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed4, 0xaaaaaa01, 0xaaaaaa02);
  view_fixed(&context);
  printf("\n");

  ZP_FixedAllocator_get(&context.fixed_allocator, &fixed3, &one, &two);
  assert(one, 0xdddddd01);
  assert(two, 0xdddddd02);

  ZP_FixedAllocator_free(&context.fixed_allocator, &fixed3);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed5;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed5);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed5, 0xbbbbbb01, 0xbbbbbb02);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedAllocator_free(&context.fixed_allocator, &fixed5);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedAllocator_free(&context.fixed_allocator, &fixed2);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedAllocator_free(&context.fixed_allocator, &fixed4);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed6;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed6);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed6, 0xcccccc08, 0xcccccc09);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed7;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed7);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed7, 0xcccccc06, 0xcccccc07);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed8;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed8);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed8, 0xcccccc04, 0xcccccc05);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed9;
  error = ZP_FixedAllocator_alloc(&context.fixed_allocator, &fixed9);
  handle_error(error);
  ZP_FixedAllocator_set(&context.fixed_allocator, &fixed9, 0xcccccc02, 0xcccccc03);
  view_fixed(&context);
  printf("first_pointer %d\n", context.fixed_allocator.first_pointer);
  printf("\n");

  printf("END test_FixedAllocator\n");

  return 0;
}

int test_int_tagging() {
  printf("BEGIN test_int_tagging\n");

  const ZP_Index_t SHIFT = (8 * sizeof(ZP_Index_t)) - 3;
  const ZP_Index_t MAXINT = (1 << SHIFT) - 1;
  const ZP_Index_t MININT = -MAXINT - 1;

  ZP_Int_t myint_m2 = -2;
  ZP_Int_t myint_m1 = -1;
  ZP_Int_t myint_ze = 0;
  ZP_Int_t myint_p1 = 1;
  ZP_Int_t myint_p2 = 2;

  ZP_Int_t myint_high_1 = MAXINT - 2;
  ZP_Int_t myint_high_2 = MAXINT - 1;
  ZP_Int_t myint_high_3 = MAXINT;
  ZP_Int_t myint_high_err = MAXINT + 1;

  ZP_Int_t myint_low_err = MININT - 1;
  ZP_Int_t myint_low_1 = MININT;
  ZP_Int_t myint_low_2 = MININT + 1;
  ZP_Int_t myint_low_3 = MININT + 2;

  ZP_Tagged_t tagged;
  ZP_Error_t error;
  ZP_Int_t integer_copy;
  ZP_Index_t tagged_data_copy;

  integer_copy = myint_m2;
  error = ZP_Tagged_from_int(&tagged, &myint_m2);
  handle_error(error);
  assert(integer_copy, myint_m2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_m2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_m1;
  error = ZP_Tagged_from_int(&tagged, &myint_m1);
  handle_error(error);
  assert(integer_copy, myint_m1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_m1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_ze;
  error = ZP_Tagged_from_int(&tagged, &myint_ze);
  handle_error(error);
  assert(integer_copy, myint_ze);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_ze);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_p1;
  error = ZP_Tagged_from_int(&tagged, &myint_p1);
  handle_error(error);
  assert(integer_copy, myint_p1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_p1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_p2;
  error = ZP_Tagged_from_int(&tagged, &myint_p2);
  handle_error(error);
  assert(integer_copy, myint_p2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_p2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_1;
  error = ZP_Tagged_from_int(&tagged, &myint_high_1);
  handle_error(error);
  assert(integer_copy, myint_high_1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_high_1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_2;
  error = ZP_Tagged_from_int(&tagged, &myint_high_2);
  handle_error(error);
  assert(integer_copy, myint_high_2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_high_2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_3;
  error = ZP_Tagged_from_int(&tagged, &myint_high_3);
  handle_error(error);
  assert(integer_copy, myint_high_3);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_high_3);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_err;
  error = ZP_Tagged_from_int(&tagged, &myint_high_err);
  handle_not_error(error);
  assert(integer_copy, myint_high_err);  /* didn't change the original */

  integer_copy = myint_low_1;
  error = ZP_Tagged_from_int(&tagged, &myint_low_1);
  handle_error(error);
  assert(integer_copy, myint_low_1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_low_1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_low_2;
  error = ZP_Tagged_from_int(&tagged, &myint_low_2);
  handle_error(error);
  assert(integer_copy, myint_low_2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_low_2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_low_3;
  error = ZP_Tagged_from_int(&tagged, &myint_low_3);
  handle_error(error);
  assert(integer_copy, myint_low_3);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_low_3);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_low_err;
  error = ZP_Tagged_from_int(&tagged, &myint_low_err);
  handle_not_error(error);
  assert(integer_copy, myint_low_err);  /* didn't change the original */

  printf("END test_int_tagging\n");

  return 0;
}

int test_float_tagging() {
  printf("BEGIN test_float_tagging\n");

  ZP_Float_t myfloat1 = 3.14f;
  ZP_Float_t myfloat2 = 3.14f;
  ZP_Float_t myfloat3 = -3.14f;
  ZP_Float_t myfloat4 = -3.14f;

  ZP_Index_t* tmp;

  tmp = (ZP_Index_t*)(&myfloat2);
  *tmp &= ~(ZP_Index_t)1;
  tmp = (ZP_Index_t*)(&myfloat4);
  *tmp &= ~(ZP_Index_t)1;

  printf("%.30f\n", myfloat1);
  printf("%.30f\n", myfloat2);
  printf("%.30f\n", myfloat3);
  printf("%.30f\n", myfloat4);

  assert(myfloat1 != myfloat2, true);
  assert(myfloat1 != myfloat3, true);
  assert(myfloat1 != myfloat4, true);

  ZP_Tagged_t tagged;
  ZP_Float_t float_copy;
  ZP_Index_t tagged_data_copy;

  float_copy = myfloat1;
  assert(myfloat1 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat1);
  assert(myfloat1 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat1 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  float_copy = myfloat2;
  assert(myfloat2 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat2);
  assert(myfloat2 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat2 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  float_copy = myfloat3;
  assert(myfloat3 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat3);
  assert(myfloat3 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat3 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  float_copy = myfloat4;
  assert(myfloat4 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat4);
  assert(myfloat4 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat4 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  printf("END test_float_tagging\n");

  return 0;
}

int test_buffer() {
  printf("BEGIN test_buffer\n");

  ZP_Context_t context;
  ZP_Error_t error = new_context(&context);
  handle_error(error);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Tagged_t buffer1;
  const char* payload1 = "hello there";
  ZP_Index_t payload1_size = 11;
  error = ZP_BufferOwned_new(&context, &buffer1, payload1_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&buffer1), ZP_TAG_BUFFER_OWNED);
  printf("hello there: 0x%08x\n", buffer1.data);

  ZP_BufferOwned_copy_data(&context, &buffer1, payload1);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Tagged_t buffer2;
  const char* payload2 = "you guys";
  ZP_Index_t payload2_size = 8;
  error = ZP_BufferOwned_new(&context, &buffer2, payload2_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&buffer2), ZP_TAG_BUFFER_OWNED);
  printf("you guys: 0x%08x\n", buffer2.data);

  ZP_BufferOwned_copy_data(&context, &buffer2, payload2);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Tagged_t buffer3;
  error = ZP_BufferView_new(&context, &buffer3, &buffer1, 3, 7);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&buffer3), ZP_TAG_BUFFER_VIEW_WRAP);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Tagged_t buffer4;
  error = ZP_BufferView_new(&context, &buffer4, &buffer2, 3, 5);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&buffer4), ZP_TAG_BUFFER_VIEW_WRAP);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Tagged_t buffer5;
  error = ZP_BufferView_new(&context, &buffer5, &buffer4, 0, 100);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&buffer5), ZP_TAG_BUFFER_VIEW_WRAP);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Context_decref(&context, &buffer1);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Context_decref(&context, &buffer3);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Context_decref(&context, &buffer5);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Context_decref(&context, &buffer4);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  ZP_Context_decref(&context, &buffer2);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("END test_buffer\n");

  return 0;
}

int test_list() {
  printf("BEGIN test_list\n");

  ZP_Context_t context;
  ZP_Error_t error = new_context(&context);
  handle_error(error);

  view_fixed(&context);
  view_refcount(&context);
  printf("\n");

  ZP_Tagged_t list1;
  ZP_List_new_empty(&list1);
  assert(ZP_Tagged_tag_of(&list1), ZP_TAG_LIST);

  ZP_Tagged_t tagged_int1;
  ZP_Int_t int1 = 5;
  error = ZP_Tagged_from_int(&tagged_int1, &int1);
  handle_error(error);

  ZP_Tagged_t list2;
  error = ZP_List_push(&context, &list2, &list1, &tagged_int1);
  handle_error(error);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 1);

  ZP_Tagged_t tagged_int2;
  ZP_Int_t int2 = 6;
  error = ZP_Tagged_from_int(&tagged_int2, &int2);
  handle_error(error);

  ZP_Tagged_t list3;
  error = ZP_List_push(&context, &list3, &list2, &tagged_int2);
  handle_error(error);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 2);
  assert(ZP_Context_refcnt(&context, &list3), 1);

  ZP_Tagged_t tagged_int3;
  ZP_Int_t int3 = 7;
  error = ZP_Tagged_from_int(&tagged_int3, &int3);
  handle_error(error);

  ZP_Tagged_t list4;
  error = ZP_List_push(&context, &list4, &list3, &tagged_int3);
  handle_error(error);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 2);
  assert(ZP_Context_refcnt(&context, &list3), 2);
  assert(ZP_Context_refcnt(&context, &list4), 1);

  ZP_Tagged_t obj1;
  ZP_Tagged_t obj2;
  error = ZP_List_pop(&context, &obj1, &obj2, &list4);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&obj1), ZP_TAG_LIST);
  assert(ZP_Tagged_tag_of(&obj2), ZP_TAG_INT);
  assert(ZP_Tagged_to_int(&obj2), 7);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 2);
  assert(ZP_Context_refcnt(&context, &list3), 3);
  assert(ZP_Context_refcnt(&context, &list4), 1);

  ZP_Tagged_t obj3;
  ZP_Tagged_t obj4;
  error = ZP_List_pop(&context, &obj3, &obj4, &obj1);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&obj3), ZP_TAG_LIST);
  assert(ZP_Tagged_tag_of(&obj4), ZP_TAG_INT);
  assert(ZP_Tagged_to_int(&obj4), 6);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 3);
  assert(ZP_Context_refcnt(&context, &list3), 3);
  assert(ZP_Context_refcnt(&context, &list4), 1);

  ZP_Tagged_t obj5;
  ZP_Tagged_t obj6;
  error = ZP_List_pop(&context, &obj5, &obj6, &obj3);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&obj5), ZP_TAG_LIST);
  assert(ZP_Tagged_tag_of(&obj6), ZP_TAG_INT);
  assert(ZP_Tagged_to_raw(&obj5), 0x00ffffff);  /* list terminator */
  assert(ZP_Tagged_to_int(&obj6), 5);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 3);
  assert(ZP_Context_refcnt(&context, &list3), 3);
  assert(ZP_Context_refcnt(&context, &list4), 1);

  printf("incref list4 many times\n");
  ZP_Index_t i;
  for (i = 0;  i < 254;  i++) {
    error = ZP_Context_incref(&context, &list4);
    handle_error(error);
  }
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list4), 255);

  ZP_Tagged_t new_list4;
  ZP_Tagged_copy(&new_list4, &list4);
  error = ZP_Context_incref(&context, &new_list4);
  handle_error(error);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 3);
  assert(ZP_Context_refcnt(&context, &list3), 4);
  assert(ZP_Context_refcnt(&context, &list4), 255);
  assert(ZP_Context_refcnt(&context, &new_list4), 1);

  printf("decref list4 many times\n");
  for (i = 0;  i < 254;  i++) {
    ZP_Context_decref(&context, &list4);
  }
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list4), 1);

  ZP_Context_decref(&context, &list4);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 3);
  assert(ZP_Context_refcnt(&context, &list3), 3);
  assert(ZP_Context_refcnt(&context, &new_list4), 1);

  ZP_Context_decref(&context, &new_list4);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 3);
  assert(ZP_Context_refcnt(&context, &list3), 2);

  ZP_Context_decref(&context, &list3);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 3);
  assert(ZP_Context_refcnt(&context, &list3), 1);

  ZP_Context_decref(&context, &list3);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 2);

  ZP_Context_decref(&context, &list2);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");
  assert(ZP_Context_refcnt(&context, &list2), 1);

  ZP_Context_decref(&context, &list2);
  view_fixed(&context);
  view_refcount(&context);
  printf("\n");

  printf("END test_list\n");

  return 0;
}

int test_symbols() {
  printf("BEGIN test_symbols\n");

  ZP_Context_t context;
  ZP_Error_t error = new_context(&context);
  handle_error(error);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  const char* symbol1_data = "someSymbol";
  ZP_Index_t symbol1_size = 10;
  ZP_Tagged_t symbol1;
  error = ZP_Symbol_new_or_get(&context, &symbol1, symbol1_data, symbol1_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&symbol1), ZP_TAG_SYMBOL);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  const char* symbol2_data = "anotherSymbol";
  ZP_Index_t symbol2_size = 13;
  ZP_Tagged_t symbol2;
  error = ZP_Symbol_new_or_get(&context, &symbol2, symbol2_data, symbol2_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&symbol2), ZP_TAG_SYMBOL);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("incref symbol2 many times\n");
  ZP_Tagged_t tmp_symbol;
  ZP_Index_t i;
  for (i = 0;  i < 253;  i++) {
    ZP_Tagged_copy(&tmp_symbol, &symbol2);
    ZP_Context_incref(&context, &tmp_symbol);
  }

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("incref symbol2 once more, should create a new symbol, not in the list\n");
  ZP_Tagged_copy(&tmp_symbol, &symbol2);
  error = ZP_Context_incref(&context, &tmp_symbol);
  handle_error(error);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  const char* symbol3_data = "anotherSymbol";
  ZP_Index_t symbol3_size = 13;
  ZP_Tagged_t symbol3;
  error = ZP_Symbol_new_or_get(&context, &symbol3, symbol3_data, symbol3_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&symbol3), ZP_TAG_SYMBOL);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("decref symbol1\n");
  ZP_Context_decref(&context, &symbol1);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  const char* symbol4_data = "newOne";
  ZP_Index_t symbol4_size = 6;
  ZP_Tagged_t symbol4;
  error = ZP_Symbol_new_or_get(&context, &symbol4, symbol4_data, symbol4_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&symbol4), ZP_TAG_SYMBOL);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("decref symbol2 to zero\n");
  for (i = 0;  i < 254;  i++) {
    ZP_Context_decref(&context, &symbol2);
  }

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("decref symbol3 to zero\n");
  ZP_Context_decref(&context, &symbol3);  /* one of these is the duplicated symbol2 */
  ZP_Context_decref(&context, &symbol3);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("decref symbol4 to zero\n");
  ZP_Context_decref(&context, &symbol4);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("all refcounts should now be zero\n");

  printf("END test_symbols\n");

  return 0;
}

int test_class() {
  printf("BEGIN test_class\n");

  ZP_Context_t context;
  ZP_Error_t error = new_context(&context);
  handle_error(error);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("define symbol1\n");
  const char* symbol1_data = "one";
  ZP_Index_t symbol1_size = 3;
  ZP_Tagged_t symbol1;
  error = ZP_Symbol_new_or_get(&context, &symbol1, symbol1_data, symbol1_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&symbol1), ZP_TAG_SYMBOL);

  printf("define symbol2\n");
  const char* symbol2_data = "two";
  ZP_Index_t symbol2_size = 3;
  ZP_Tagged_t symbol2;
  error = ZP_Symbol_new_or_get(&context, &symbol2, symbol2_data, symbol2_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&symbol2), ZP_TAG_SYMBOL);

  printf("define symbol3\n");
  const char* symbol3_data = "three";
  ZP_Index_t symbol3_size = 5;
  ZP_Tagged_t symbol3;
  error = ZP_Symbol_new_or_get(&context, &symbol3, symbol3_data, symbol3_size);
  handle_error(error);
  assert(ZP_Tagged_tag_of(&symbol3), ZP_TAG_SYMBOL);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("put them in a list\n");
  ZP_Tagged_t list0;
  ZP_List_new_empty(&list0);

  ZP_Tagged_t list1;
  error = ZP_List_push(&context, &list1, &list0, &symbol2);
  handle_error(error);

  ZP_Tagged_t list2;
  error = ZP_List_push(&context, &list2, &list1, &symbol3);
  handle_error(error);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("make a class\n");
  ZP_Tagged_t list_of_supers;
  ZP_List_new_empty(&list_of_supers);
  ZP_Tagged_t class1;
  error = ZP_Class_new(&context, &class1, &list_of_supers, &list2);
  handle_error(error);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("now decref it\n");
  ZP_Class_decref(&context, &class1);

  view_fixed(&context);
  view_refcount(&context);
  view_region(&context);
  printf("\n");

  printf("END test_class\n");

  return 0;
}

int main(int argc, char** argv) {
  int result;

  /* result = test_RegionAllocator(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_FixedAllocator(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_int_tagging(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_float_tagging(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_buffer(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_list(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_symbols(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  result = test_class();
  if (result != 0) {
    return result;
  }

  return 0;
}
